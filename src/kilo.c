/* Based on https://github.com/antirez/kilo
 *
 * -----------------------------------------------------------------------
 *
 * Copyright (C) 2016 Salvatore Sanfilippo <antirez at gmail dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define KILO_VERSION "0.0.1"

#ifdef __linux__
#define _POSIX_C_SOURCE 200809L
#endif

#include <assert.h>
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#include "lua.h"
#include "teliva.h"

/* Syntax highlight types */
#define HL_NORMAL 0
#define HL_NONPRINT 1
#define HL_COMMENT 2   /* Single line comment. */
#define HL_MLCOMMENT 3 /* Multi-line comment. */
#define HL_KEYWORD1 4
#define HL_KEYWORD2 5
#define HL_STRING 6
#define HL_NUMBER 7
#define HL_MATCH 8      /* Search match. */

struct editorSyntax {
    char **keywords;
    char singleline_comment_start[2];
    char *multiline_comment_start;
    char *multiline_comment_end;
};

/* This structure represents a single line of the file we are editing. */
typedef struct erow {
    int idx;            /* Row index in the file, zero-based. */
    int size;           /* Size of the row, excluding the null term. */
    int rsize;          /* Size of the rendered row. */
    char *chars;        /* Row content. */
    char *render;       /* Row content "rendered" for screen (for TABs). */
    unsigned char *hl;  /* Syntax highlight type for each character in render.*/
    int hl_oc;          /* Row had open comment at end in last syntax highlight
                           check. */
} erow;

typedef struct hlcolor {
    int r,g,b;
} hlcolor;

struct editorConfig {
    int cx,cy;  /* Cursor x and y position in characters */
    int rowoff;     /* Offset of row displayed. */
    int coloff;     /* Offset of column displayed. */
    int numrows;    /* Number of rows */
    erow *row;      /* Rows */
    int dirty;      /* File modified but not saved. */
    char *filename; /* Currently open filename */
    struct editorSyntax *syntax;    /* Current syntax highlight, or NULL. */
};

static struct editorConfig E;

/* =========================== Syntax highlights DB =========================
 *
 * In order to add a new syntax, define two arrays with a list of file name
 * matches and keywords. The file name matches are used in order to match
 * a given syntax with a given file name: if a match pattern starts with a
 * dot, it is matched as the last past of the filename, for example ".c".
 * Otherwise the pattern is just searched inside the filenme, like "Makefile").
 *
 * The list of keywords to highlight is just a list of words, however if they
 * a trailing '|' character is added at the end, they are highlighted in
 * a different color, so that you can have two different sets of keywords.
 *
 * Finally add a stanza in the HLDB global variable with two two arrays
 * of strings.
 *
 * The characters for single and multi line comments must be exactly two
 * and must be provided as well (see the C language example).
 *
 * There is no support to highlight patterns currently. */

/* Lua */
char *Lua_HL_keywords[] = {
    /* keywords */
    "do", "end", "function", "return", "require", "local",
    "if", "then", "else", "elseif",
    "while", "for", "repeat", "until", "break",
    "and", "or", "not", "in",

    /* types */
    "nil|", "false|", "true|",

    NULL
};

/* Here we define an array of syntax highlights by extensions, keywords,
 * comments delimiters. */
struct editorSyntax HLDB[] = {
    {
        Lua_HL_keywords,
        "--",  /* line comment */
        "--[[",  /* multiline comment start */
        "--]]"  /* multline comment stop */
    }
};

#define HLDB_ENTRIES (sizeof(HLDB)/sizeof(HLDB[0]))

/* ====================== Syntax highlight color scheme  ==================== */

static int is_separator(int c) {
    return c == '\0' || isspace(c) || strchr(",.()+-/*=~%[];",c) != NULL;
}

static int starts_with(const char* s, const char* prefix) {
    return strncmp(prefix, s, strlen(prefix)) == 0;
}

/* Return true if the specified row last char is part of a multi line comment
 * that starts at this row or at one before, and does not end at the end
 * of the row but spawns to the next row. */
static int editorRowHasOpenComment(erow *row, char *mce) {
    if (row->hl && row->rsize && row->hl[row->rsize-1] == HL_MLCOMMENT &&
        (row->rsize < strlen(mce) || strcmp(&row->render[row->rsize-strlen(mce)], mce) != 0)) return 1;
    return 0;
}

/* Set every byte of row->hl (that corresponds to every character in the line)
 * to the right syntax highlight type (HL_* defines). */
static void editorUpdateSyntax(erow *row) {
    row->hl = realloc(row->hl,row->rsize);
    memset(row->hl,HL_NORMAL,row->rsize);

    if (E.syntax == NULL) return; /* No syntax, everything is HL_NORMAL. */

    int i, prev_sep, in_string, in_comment;
    char *p;
    char **keywords = E.syntax->keywords;
    char *scs = E.syntax->singleline_comment_start;
    char *mcs = E.syntax->multiline_comment_start;
    char *mce = E.syntax->multiline_comment_end;

    /* Point to the first non-space char. */
    p = row->render;
    i = 0; /* Current char offset */
    while(*p && isspace(*p)) {
        p++;
        i++;
    }
    prev_sep = 1; /* Tell the parser if 'i' points to start of word. */
    in_string = 0; /* Are we inside "" or '' ? */
    in_comment = 0; /* Are we inside multi-line comment? */

    /* If the previous line has an open comment, this line starts
     * with an open comment state. */
    if (row->idx > 0 && editorRowHasOpenComment(&E.row[row->idx-1], mce))
        in_comment = 1;

    while(*p) {
        /* Handle multi line comments. */
        if (in_comment) {
            row->hl[i] = HL_MLCOMMENT;
            if (starts_with(p, mce)) {
                memset(&row->hl[i],HL_MLCOMMENT, strlen(mce));
                p += strlen(mce); i += strlen(mce);
                in_comment = 0;
                prev_sep = 1;
                continue;
            } else {
                prev_sep = 0;
                p++; i++;
                continue;
            }
        } else if (starts_with(p, mcs)) {
            memset(&row->hl[i],HL_MLCOMMENT, strlen(mcs));
            p += strlen(mcs); i += strlen(mcs);
            in_comment = 1;
            prev_sep = 0;
            continue;
        }

        /* Handle single-line comments. */
        if (prev_sep && *p == scs[0] && *(p+1) == scs[1]) {
            /* From here to end is a comment */
            memset(row->hl+i,HL_COMMENT,row->rsize-i);
            return;
        }

        /* Handle "" and '' */
        if (in_string) {
            row->hl[i] = HL_STRING;
            if (*p == '\\' && *(p+1)) {
                row->hl[i+1] = HL_STRING;
                p += 2; i += 2;
                prev_sep = 0;
                continue;
            }
            if (*p == in_string) in_string = 0;
            p++; i++;
            continue;
        } else {
            if (*p == '"' || *p == '\'') {
                in_string = *p;
                row->hl[i] = HL_STRING;
                p++; i++;
                prev_sep = 0;
                continue;
            }
        }

        /* Handle non printable chars. */
        if (!isprint(*p)) {
            row->hl[i] = HL_NONPRINT;
            p++; i++;
            prev_sep = 0;
            continue;
        }

        /* Handle numbers */
        if ((isdigit(*p) && (prev_sep || row->hl[i-1] == HL_NUMBER)) ||
            (*p == '.' && i > 0 && row->hl[i-1] == HL_NUMBER)) {
            row->hl[i] = HL_NUMBER;
            p++; i++;
            prev_sep = 0;
            continue;
        }

        /* Handle keywords and lib calls */
        if (prev_sep) {
            int j;
            int ileft = row->rsize-i;
            for (j = 0; keywords[j]; j++) {
                int klen = strlen(keywords[j]);
                int kw2 = keywords[j][klen-1] == '|';
                if (kw2) klen--;

                if (klen <= ileft && !memcmp(p,keywords[j],klen) &&
                    is_separator(*(p+klen)))
                {
                    /* Keyword */
                    memset(row->hl+i,kw2 ? HL_KEYWORD2 : HL_KEYWORD1,klen);
                    p += klen;
                    i += klen;
                    break;
                }
            }
            if (keywords[j] != NULL) {
                prev_sep = 0;
                continue; /* We had a keyword match */
            }
        }

        /* Not special chars */
        prev_sep = is_separator(*p);
        p++; i++;
    }

    /* Propagate syntax change to the next row if the open comment
     * state changed. This may recursively affect all the following rows
     * in the file. */
    int oc = editorRowHasOpenComment(row, mce);
    if (row->hl_oc != oc && row->idx+1 < E.numrows)
        editorUpdateSyntax(&E.row[row->idx+1]);
    row->hl_oc = oc;
}

/* Maps syntax highlight token types to terminal colors. */
static int editorSyntaxToColorPair(int hl) {
    switch(hl) {
    case HL_COMMENT:
    case HL_MLCOMMENT: return COLOR_PAIR_LUA_COMMENT;
    case HL_KEYWORD1: return COLOR_PAIR_LUA_KEYWORD;
    case HL_KEYWORD2: return COLOR_PAIR_LUA_CONSTANT;
    case HL_STRING: return COLOR_PAIR_LUA_CONSTANT;
    case HL_NUMBER: return COLOR_PAIR_LUA_CONSTANT;
    case HL_MATCH: return COLOR_PAIR_MATCH;
    default: return COLOR_PAIR_NORMAL;
    }
}

/* ======================= Editor rows implementation ======================= */

/* Update the rendered version and the syntax highlight of a row. */
static void editorUpdateRow(erow *row) {
    unsigned int tabs = 0, nonprint = 0;
    int j, idx;

   /* Create a version of the row we can directly print on the screen,
     * respecting tabs, substituting non printable characters with '?'. */
    free(row->render);
    for (j = 0; j < row->size; j++)
        if (row->chars[j] == TAB) tabs++;

    unsigned long long allocsize =
        (unsigned long long) row->size + tabs*8 + nonprint*9 + 1;
    if (allocsize > UINT32_MAX) {
        printf("Some line of the edited file is too long for kilo\n");
        exit(1);
    }

    row->render = malloc(row->size + tabs*8 + nonprint*9 + 1);
    idx = 0;
    for (j = 0; j < row->size; j++) {
        if (row->chars[j] == TAB) {
            row->render[idx++] = ' ';
            while((idx+1) % 8 != 0) row->render[idx++] = ' ';
        } else {
            row->render[idx++] = row->chars[j];
        }
    }
    row->rsize = idx;
    row->render[idx] = '\0';

    /* Update the syntax highlighting attributes of the row. */
    editorUpdateSyntax(row);
}

/* Insert a row at the specified position, shifting the other rows on the bottom
 * if required. */
static void editorInsertRow(int at, char *s, size_t len) {
    if (at > E.numrows) return;
    E.row = realloc(E.row,sizeof(erow)*(E.numrows+1));
    if (at != E.numrows) {
        memmove(E.row+at+1,E.row+at,sizeof(E.row[0])*(E.numrows-at));
        for (int j = at+1; j <= E.numrows; j++) E.row[j].idx++;
    }
    E.row[at].size = len;
    E.row[at].chars = malloc(len+1);
    memcpy(E.row[at].chars,s,len+1);
    E.row[at].hl = NULL;
    E.row[at].hl_oc = 0;
    E.row[at].render = NULL;
    E.row[at].rsize = 0;
    E.row[at].idx = at;
    editorUpdateRow(E.row+at);
    E.numrows++;
    E.dirty++;
}

/* Free row's heap allocated stuff. */
static void editorFreeRow(erow *row) {
    free(row->render);
    free(row->chars);
    free(row->hl);
}

/* Remove the row at the specified position, shifting the remaining on the
 * top. */
static void editorDelRow(int at) {
    erow *row;

    if (at >= E.numrows) return;
    row = E.row+at;
    editorFreeRow(row);
    memmove(E.row+at,E.row+at+1,sizeof(E.row[0])*(E.numrows-at-1));
    for (int j = at; j < E.numrows-1; j++) E.row[j].idx++;
    E.numrows--;
    E.dirty++;
}

void clearEditor(void) {
    E.cx = E.cy = 0;
    E.rowoff = E.coloff = 0;
    for (int j = E.numrows-1; j >= 0; j--)
      editorDelRow(j);
}

/* Turn the editor rows into a single heap-allocated string.
 * Returns the pointer to the heap-allocated string and populate the
 * integer pointed by 'buflen' with the size of the string, excluding
 * the final null term. */
static char *editorRowsToString(int *buflen) {
    char *buf = NULL, *p;
    int totlen = 0;
    int j;

    /* Compute count of bytes */
    for (j = 0; j < E.numrows; j++)
        totlen += E.row[j].size+1; /* +1 is for "\n" at end of every row */
    *buflen = totlen;
    totlen++; /* Also make space for null term */

    p = buf = malloc(totlen);
    for (j = 0; j < E.numrows; j++) {
        memcpy(p,E.row[j].chars,E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }
    *p = '\0';
    return buf;
}

/* Insert a character at the specified position in a row, moving the remaining
 * chars on the right if needed. */
static void editorRowInsertChar(erow *row, int at, int c) {
    if (at > row->size) {
        /* Pad the string with spaces if the insert location is outside the
         * current length by more than a single character. */
        int padlen = at-row->size;
        /* In the next line +2 means: new char and null term. */
        row->chars = realloc(row->chars,row->size+padlen+2);
        memset(row->chars+row->size,' ',padlen);
        row->chars[row->size+padlen+1] = '\0';
        row->size += padlen+1;
    } else {
        /* If we are in the middle of the string just make space for 1 new
         * char plus the (already existing) null term. */
        row->chars = realloc(row->chars,row->size+2);
        memmove(row->chars+at+1,row->chars+at,row->size-at+1);
        row->size++;
    }
    row->chars[at] = c;
    editorUpdateRow(row);
    E.dirty++;
}

/* Append the string 's' at the end of a row */
static void editorRowAppendString(erow *row, char *s, size_t len) {
    row->chars = realloc(row->chars,row->size+len+1);
    memcpy(row->chars+row->size,s,len);
    row->size += len;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
    E.dirty++;
}

/* Delete the character at offset 'at' from the specified row. */
static void editorRowDelChar(erow *row, int at) {
    if (row->size <= at) return;
    memmove(row->chars+at,row->chars+at+1,row->size-at);
    editorUpdateRow(row);
    row->size--;
    E.dirty++;
}

/* Insert the specified char at the current prompt position. */
static void editorInsertChar(int c) {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    /* If the row where the cursor is currently located does not exist in our
     * logical representaion of the file, add enough empty rows as needed. */
    if (!row) {
        while(E.numrows <= filerow)
            editorInsertRow(E.numrows,"",0);
    }
    row = &E.row[filerow];
    editorRowInsertChar(row,filecol,c);
    if (E.cx == COLS-1)
        E.coloff++;
    else
        E.cx++;
    E.dirty++;
}

/* Inserting a newline is slightly complex as we have to handle inserting a
 * newline in the middle of a line, splitting the line as needed. */
static void editorInsertNewline(void) {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    if (!row) {
        if (filerow == E.numrows) {
            editorInsertRow(filerow,"",0);
            goto fixcursor;
        }
        return;
    }
    /* If the cursor is over the current line size, we want to conceptually
     * think it's just over the last character. */
    if (filecol >= row->size) filecol = row->size;
    if (filecol == 0) {
        editorInsertRow(filerow,"",0);
    } else {
        /* We are in the middle of a line. Split it between two rows. */
        editorInsertRow(filerow+1,row->chars+filecol,row->size-filecol);
        row = &E.row[filerow];
        row->chars[filecol] = '\0';
        row->size = filecol;
        editorUpdateRow(row);
    }
fixcursor:
    if (E.cy == LINES-1-1) {
        E.rowoff++;
    } else {
        E.cy++;
    }
    E.cx = 0;
    E.coloff = 0;
}

/* Delete the char at the current prompt position. */
static void editorDelChar() {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    if (!row || (filecol == 0 && filerow == 0)) return;
    if (filecol == 0) {
        /* Handle the case of column 0, we need to move the current line
         * on the right of the previous one. */
        filecol = E.row[filerow-1].size;
        editorRowAppendString(&E.row[filerow-1],row->chars,row->size);
        editorDelRow(filerow);
        row = NULL;
        if (E.cy == 0)
            E.rowoff--;
        else
            E.cy--;
        E.cx = filecol;
        if (E.cx >= COLS) {
            int shift = (COLS-E.cx)+1;
            E.cx -= shift;
            E.coloff += shift;
        }
    } else {
        editorRowDelChar(row,filecol-1);
        if (E.cx == 0 && E.coloff)
            E.coloff--;
        else
            E.cx--;
    }
    if (row) editorUpdateRow(row);
    E.dirty++;
}

static void editorUncommentCursorRow() {
    erow *row = &E.row[E.rowoff+E.cy];
    E.coloff = 0;
    E.cx = 4;
    editorUpdateRow(row);
    editorDelChar();
    editorDelChar();
    editorDelChar();
    editorDelChar();
}

static void editorCommentCursorRow() {
    erow *row = &E.row[E.rowoff+E.cy];
    editorRowInsertChar(row, 0, ' ');
    editorRowInsertChar(row, 0, '?');
    editorRowInsertChar(row, 0, '-');
    editorRowInsertChar(row, 0, '-');
    E.coloff = 0;
    E.cx = 0;
}

/* Load the specified program in the editor memory and returns 0 on success
 * or 1 on error. */
int editorOpen(char *filename) {
    FILE *fp;

    E.dirty = 0;
    free(E.filename);
    size_t fnlen = strlen(filename)+1;
    E.filename = malloc(fnlen);
    memcpy(E.filename,filename,fnlen);

    fp = fopen(filename,"r");
    if (!fp) {
        if (errno != ENOENT) {
            perror("Opening file");
            exit(1);
        }
        return 1;
    }

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while((linelen = getline(&line,&linecap,fp)) != -1) {
        if (linelen && (line[linelen-1] == '\n' || line[linelen-1] == '\r'))
            line[--linelen] = '\0';
        editorInsertRow(E.numrows,line,linelen);
    }
    free(line);
    fclose(fp);
    E.dirty = 0;
    return 0;
}

/* Save the current file on disk. Return 0 on success, 1 on error. */
static int editorSaveToDisk(void) {
    int len;
    char *buf = editorRowsToString(&len);
    int fd = open(E.filename,O_RDWR|O_CREAT,0644);
    if (fd == -1) goto writeerr;

    /* Use truncate + a single write(2) call in order to make saving
     * a bit safer, under the limits of what we can do in a small editor. */
    if (ftruncate(fd,len) == -1) goto writeerr;
    if (write(fd,buf,len) != len) goto writeerr;

    close(fd);
    free(buf);
    E.dirty = 0;
    return 0;

writeerr:
    free(buf);
    if (fd != -1) close(fd);
    /* TODO: better error handling. */
    /* I haven't gotten to this yet since we have version control. */
    endwin();
    printf("Can't save! I/O error: %s",strerror(errno));
    exit(1);
    return 1;
}

/* ============================= Terminal update ============================ */

extern char *Previous_error;
extern void draw_menu_item(const char* key, const char* name);

static void editorMenu(void) {
    attrset(A_REVERSE);
    for (int x = 0; x < COLS; ++x)
      mvaddch(LINES-1, x, ' ');
    attrset(A_NORMAL);
    extern int menu_column;
    menu_column = 2;
    draw_menu_item("^e", "run");
    if (Previous_error != NULL) {
      attron(A_BOLD);
      draw_menu_item("^c", "abort");
      attroff(A_BOLD);
    }
    draw_menu_item("^g", "go");
    draw_menu_item("^b", "big picture");
    draw_menu_item("^f", "find");
    draw_menu_item("^/|^-|^_", "(un)comment line");
    draw_menu_item("^h", "back up cursor");
    draw_menu_item("^l", "end of line");
    attrset(A_NORMAL);
}

extern void draw_string_on_menu (const char* s);
static void editorFindMenu(void) {
    attrset(A_REVERSE);
    for (int x = 0; x < COLS; ++x)
      mvaddch(LINES-1, x, ' ');
    attrset(A_NORMAL);
    extern int menu_column;
    menu_column = 2;
    draw_menu_item("^x", "cancel");
    draw_menu_item("Enter", "submit");
    draw_menu_item("^h", "back up cursor");
    draw_menu_item("^u", "clear");
    /* draw_menu_item("←|↑", "previous"); */
    attroff(A_REVERSE);
    mvaddstr(LINES-1, menu_column, " ←|↑ ");
    menu_column += 5;  /* strlen isn't sufficient */
    attron(A_REVERSE);
    draw_string_on_menu("previous");
    /* draw_menu_item("↓|→", "next"); */
    attroff(A_REVERSE);
    mvaddstr(LINES-1, menu_column, " ↓|→ ");
    menu_column += 5;  /* strlen isn't sufficient */
    attron(A_REVERSE);
    draw_string_on_menu("next");
    attrset(A_NORMAL);
}

static void editorGoMenu(void) {
    attrset(A_REVERSE);
    for (int x = 0; x < COLS; ++x)
      mvaddch(LINES-1, x, ' ');
    attrset(A_NORMAL);
    extern int menu_column;
    menu_column = 2;
    draw_menu_item("^x", "cancel");
    draw_menu_item("Enter", "submit");
    draw_menu_item("^h", "back up cursor");
    draw_menu_item("^u", "clear");
    attrset(A_NORMAL);
}

extern int render_previous_error();
static void editorRefreshScreen(void (*menu_func)(void)) {
    int y;
    erow *r;
    int current_color = -1;
    curs_set(0);
    clear();
    attrset(A_NORMAL);
    for (y = 0; y < LINES-1; y++) {
        int filerow = E.rowoff+y;

        if (filerow >= E.numrows) {
            continue;
        }

        r = &E.row[filerow];

        int len = r->rsize - E.coloff;
        mvaddstr(y, 0, "");
        if (len > 0) {
            if (len > COLS) len = COLS;
            char *c = r->render+E.coloff;
            unsigned char *hl = r->hl+E.coloff;
            int j;
            for (j = 0; j < len; j++) {
                if (hl[j] == HL_NONPRINT) {
                    char sym;
                    attron(A_REVERSE);
                    if (c[j] <= 26)
                        sym = '@'+c[j];
                    else
                        sym = '?';
                    addch(sym);
                    attroff(A_REVERSE);
                } else if (hl[j] == HL_NORMAL) {
                    if (current_color != -1) {
                        attrset(A_NORMAL);
                        current_color = -1;
                    }
                    addch(c[j]);
                } else {
                    int color = editorSyntaxToColorPair(hl[j]);
                    if (color != current_color) {
                        attrset(COLOR_PAIR(color));
                        current_color = color;
                    }
                    addch(c[j]);
                }
            }
        }
    }

    render_previous_error();

    (*menu_func)();

    /* Put cursor at its current position. Note that the horizontal position
     * at which the cursor is displayed may be different compared to 'E.cx'
     * because of TABs. */
    int j;
    int cx = 0;
    int filerow = E.rowoff+E.cy;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];
    if (row) {
        for (j = E.coloff; j < (E.cx+E.coloff); j++) {
            if (j < row->size && row->chars[j] == TAB) cx += 7-((cx)%8);
            cx++;
        }
    }
    mvaddstr(E.cy, cx, "");
    curs_set(1);
}

/* =============================== Find mode ================================ */

#define KILO_QUERY_LEN 256

static void editorFind() {
    char query[KILO_QUERY_LEN+1] = {0};
    int qlen = 0;
    int last_match = -1; /* Last line where a match was found. -1 for none. */
    int find_next = 0; /* if 1 search next, if -1 search prev. */
    int saved_hl_line = -1;  /* No saved HL */
    char *saved_hl = NULL;

#define FIND_RESTORE_HL do { \
    if (saved_hl) { \
        memcpy(E.row[saved_hl_line].hl,saved_hl, E.row[saved_hl_line].rsize); \
        free(saved_hl); \
        saved_hl = NULL; \
    } \
} while (0)

    /* Save the cursor position in order to restore it later. */
    int saved_cx = E.cx, saved_cy = E.cy;
    int saved_coloff = E.coloff, saved_rowoff = E.rowoff;

    while(1) {
        editorRefreshScreen(editorFindMenu);
        mvprintw(LINES-2, 0, "Find: %s", query);

        int c = getch();
        if (c == KEY_BACKSPACE || c == DELETE || c == CTRL_H) {
            if (qlen != 0) query[--qlen] = '\0';
            last_match = -1;
        } else if (c == CTRL_X || c == ENTER) {
            if (c == CTRL_X) {
                E.cx = saved_cx; E.cy = saved_cy;
                E.coloff = saved_coloff; E.rowoff = saved_rowoff;
            }
            FIND_RESTORE_HL;
            return;
        } else if (c == CTRL_U) {
            qlen = 0;
            query[qlen] = '\0';
        } else if (c == KEY_RIGHT || c == KEY_DOWN) {
            find_next = 1;
        } else if (c == KEY_LEFT || c == KEY_UP) {
            find_next = -1;
        } else if (isprint(c)) {
            if (qlen < KILO_QUERY_LEN) {
                query[qlen++] = c;
                query[qlen] = '\0';
                last_match = -1;
            }
        }

        /* Search occurrence. */
        if (last_match == -1) find_next = 1;
        if (find_next) {
            char *match = NULL;
            int match_offset = 0;
            int i, current = last_match;

            for (i = 0; i < E.numrows; i++) {
                current += find_next;
                if (current == -1) current = E.numrows-1;
                else if (current == E.numrows) current = 0;
                match = strstr(E.row[current].render,query);
                if (match) {
                    match_offset = match-E.row[current].render;
                    break;
                }
            }
            find_next = 0;

            /* Highlight */
            FIND_RESTORE_HL;

            if (match) {
                erow *row = &E.row[current];
                last_match = current;
                if (row->hl) {
                    saved_hl_line = current;
                    saved_hl = malloc(row->rsize);
                    memcpy(saved_hl,row->hl,row->rsize);
                    memset(row->hl+match_offset,HL_MATCH,qlen);
                }
                E.cy = 0;
                E.cx = match_offset;
                E.rowoff = current;
                E.coloff = 0;
                /* Scroll horizontally as needed. */
                if (E.cx > COLS) {
                    int diff = E.cx - COLS;
                    E.cx -= diff;
                    E.coloff += diff;
                }
            }
        }
    }
}

/* ========================= Editor events handling  ======================== */

static int editorAtStartOfLine() {
    return E.coloff == 0 && E.cx == 0;
}

/* Handle cursor position change because arrow keys were pressed. */
static void editorMoveCursor(int key) {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    int rowlen;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    switch(key) {
    case KEY_LEFT:
        if (E.cx == 0) {
            if (E.coloff) {
                E.coloff--;
            } else {
                if (filerow > 0) {
                    E.cy--;
                    E.cx = E.row[filerow-1].size;
                    if (E.cx > COLS-1) {
                        E.coloff = E.cx-COLS+1;
                        E.cx = COLS-1;
                    }
                }
            }
        } else {
            E.cx -= 1;
        }
        break;
    case KEY_RIGHT:
        if (row && filecol < row->size) {
            if (E.cx == COLS-1) {
                E.coloff++;
            } else {
                E.cx += 1;
            }
        } else if (row && filecol == row->size) {
            E.cx = 0;
            E.coloff = 0;
            if (E.cy == LINES-1-1) {
                E.rowoff++;
            } else {
                E.cy += 1;
            }
        }
        break;
    case KEY_UP:
        if (E.cy == 0) {
            if (E.rowoff) E.rowoff--;
        } else {
            E.cy -= 1;
        }
        break;
    case KEY_DOWN:
        if (filerow < E.numrows) {
            if (E.cy == LINES-1-1) {
                E.rowoff++;
            } else {
                E.cy += 1;
            }
        }
        break;
    }
    /* Fix cx if the current line has not enough chars. */
    filerow = E.rowoff+E.cy;
    filecol = E.coloff+E.cx;
    row = (filerow >= E.numrows) ? NULL : &E.row[filerow];
    rowlen = row ? row->size : 0;
    if (filecol > rowlen) {
        E.cx -= filecol-rowlen;
        if (E.cx < 0) {
            E.coloff += E.cx;
            E.cx = 0;
        }
    }
}

int identifier_char(char c) {
    /* keep sync'd with llex */
    return isalnum(c) || c == '_';
}

void word_at_cursor(char* out, int capacity) {
    erow* row = &E.row[E.rowoff + E.cy];
    int cidx = E.coloff + E.cx;
    int len = 0;
    memset(out, 0, capacity);
    if (row == NULL) return;
    /* scan back to first identifier char */
    while (cidx > 0) {
        --cidx;
        if (!identifier_char(row->chars[cidx])) {
            ++cidx;
            break;
        }
    }
    /* now scan forward */
    for (len = 0; cidx+len < row->size; ++len) {
        if (!identifier_char(row->chars[cidx+len]))
            break;
    }
    if (len < capacity)
        strncpy(out, &row->chars[cidx], len);
}

/* Jump to some definition. */
extern void save_to_current_definition_and_editor_buffer(lua_State *L, char *name);
extern void load_editor_buffer_to_current_definition_in_image(lua_State *L);
extern char Current_definition[];
#define CURRENT_DEFINITION_LEN 256
static void editorGo(lua_State* L) {
    char query[CURRENT_DEFINITION_LEN+1] = {0};
    int qlen = 0;

    if (strlen(Current_definition) > 0) {
      /* We're currently editing a definition. Save it. */
      editorSaveToDisk();
      load_editor_buffer_to_current_definition_in_image(L);
    }

    word_at_cursor(query, CURRENT_DEFINITION_LEN);
    qlen = strlen(query);

    while(1) {
        editorRefreshScreen(editorGoMenu);
        mvprintw(LINES-2, 0, "Go to: %s", query);

        int c = getch();
        if (c == KEY_BACKSPACE || c == DELETE || c == CTRL_H) {
            if (qlen != 0) query[--qlen] = '\0';
        } else if (c == CTRL_X || c == ENTER) {
            if (c == ENTER) {
              save_to_current_definition_and_editor_buffer(L, query);
              clearEditor();
              editorOpen("teliva_editor_buffer");
            }
            return;
        } else if (c == CTRL_U) {
          qlen = 0;
          query[qlen] = '\0';
        } else if (isprint(c)) {
            if (qlen < CURRENT_DEFINITION_LEN) {
                query[qlen++] = c;
                query[qlen] = '\0';
            }
        }
    }
}

/* Process events arriving from the standard input, which is, the user
 * is typing stuff on the terminal. */
static int Quit = 0;
static int Back_to_big_picture = 0;
extern void save_editor_state(int rowoff, int coloff, int cy, int cx);
static void editorProcessKeypress(lua_State* L) {
    int c = getch();
//?     mvprintw(LINES-3, 60, "key: %d\n", c);
//?     getch();
    switch(c) {
    case ENTER:
        {
            erow* oldrow = &E.row[E.rowoff + E.cy];
            editorInsertNewline();
            /* auto-indent */
            for (int x = 0; x < oldrow->size && oldrow->chars[x] == ' '; ++x)
                editorInsertChar(' ');
        }
        break;
    case CTRL_C:
        if (Previous_error != NULL)
            exit(1);
        break;
    case CTRL_E:
        /* Save and quit. */
        editorSaveToDisk();
        save_editor_state(E.rowoff, E.coloff, E.cy, E.cx);
        Quit = 1;
        break;
    case CTRL_G:
        /* Go to a different definition. */
        editorGo(L);
        break;
    case CTRL_B:
        /* Go to big-picture view. */
        editorSaveToDisk();
        Quit = 1;
        Back_to_big_picture = 1;
        break;
    case CTRL_F:
        editorFind();
        break;
    case KEY_BACKSPACE:
    case DELETE:
    case CTRL_H:
        editorDelChar();
        break;
    case KEY_NPAGE:
    case KEY_PPAGE:
        if (c == KEY_PPAGE && E.cy != 0)
            E.cy = 0;
        else if (c == KEY_NPAGE && E.cy != LINES-1-1)
            E.cy = LINES-1-1;
        {
            int times = LINES-1;
            while(times--)
                editorMoveCursor(c == KEY_PPAGE ? KEY_UP : KEY_DOWN);
        }
        break;
    case CTRL_A:
        while (!editorAtStartOfLine())
            editorMoveCursor(KEY_LEFT);
        break;
    case CTRL_L:
        while (1) {
            editorMoveCursor(KEY_RIGHT);
            if (editorAtStartOfLine()) {
                editorMoveCursor(KEY_LEFT);
                break;
            }
        }
        break;
    case CTRL_U:
        while (!editorAtStartOfLine())
            editorDelChar();
        break;
    case CTRL_K:
        while (1) {
            editorMoveCursor(KEY_RIGHT);
            if (editorAtStartOfLine()) {
                editorMoveCursor(KEY_LEFT);
                break;
            }
            editorDelChar();
        }
        break;
    case CTRL_SLASH:  /* same as CTRL_UNDERSCORE */
        if (starts_with(E.row[E.rowoff+E.cy].chars, "--? "))
            editorUncommentCursorRow();
        else
            editorCommentCursorRow();
        break;
    case KEY_UP:
    case KEY_DOWN:
    case KEY_LEFT:
    case KEY_RIGHT:
        editorMoveCursor(c);
        break;
    case TAB:
        /* insert 2 spaces */
        editorInsertChar(' ');
        editorInsertChar(' ');
        break;
    default:
        if (c >= ' ')
            editorInsertChar(c);
        break;
    }
}

static void initEditor(void) {
    E.cx = 0;
    E.cy = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.row = NULL;
    E.dirty = 0;
    E.filename = NULL;
    E.syntax = &HLDB[0];
}

/* return true if user chose to back into the big picture view */
int edit(lua_State* L, char* filename) {
    Quit = 0;
    Back_to_big_picture = 0;
    initEditor();
    editorOpen(filename);
    while(!Quit) {
        editorRefreshScreen(editorMenu);
        editorProcessKeypress(L);
    }
    return Back_to_big_picture;
}

/* return true if user chose to back into the big picture view */
int edit_from(lua_State* L, char* filename, int rowoff, int coloff, int cy, int cx) {
    Quit = 0;
    Back_to_big_picture = 0;
    initEditor();
    E.rowoff = rowoff;
    E.coloff = coloff;
    E.cy = cy;
    E.cx = cx;
    editorOpen(filename);
    while(!Quit) {
        editorRefreshScreen(editorMenu);
        editorProcessKeypress(L);
    }
    return Back_to_big_picture;
}

int resumeEdit(lua_State* L) {
    Quit = 0;
    Back_to_big_picture = 0;
    while(!Quit) {
        editorRefreshScreen(editorMenu);
        editorProcessKeypress(L);
    }
    return Back_to_big_picture;
}

/* vim:tabstop=4:shiftwidth=0:expandtab:softtabstop=-1
 */
