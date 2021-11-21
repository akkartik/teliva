/*
 * Curses binding for Lua 5.1, 5.2 & 5.3.
 *
 * (c) Gary V. Vaughan <gary@vaughan.pe> 2013-2017
 * (c) Reuben Thomas <rrt@sc3d.org> 2009-2012
 * (c) Tiago Dionizio <tiago.dionizio AT gmail.com> 2004-2007
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/***
 Curses Window objects.

 The methods documented on this page are available on any Curses Window
 object, such as created by:

     stdscr = curses.initscr ()
     window = curses.newwin (25, 80, 0, 0)

@classmod curses.window
*/

#include "_helpers.c"

#include "chstr.c"


static const char *WINDOWMETA = "curses:window";

static void
lc_newwin(lua_State *L, WINDOW *nw)
{
	if (nw)
	{
		WINDOW **w = lua_newuserdata(L, sizeof(WINDOW*));
		luaL_getmetatable(L, WINDOWMETA);
		lua_setmetatable(L, -2);
		*w = nw;
	}
	else
	{
		lua_pushliteral(L, "failed to create window");
		lua_error(L);
	}
}


static WINDOW **
lc_getwin(lua_State *L, int offset)
{
	WINDOW **w = (WINDOW**)luaL_checkudata(L, offset, WINDOWMETA);
	if (w == NULL)
		luaL_argerror(L, offset, "bad curses window");
	return w;
}


static WINDOW *
checkwin(lua_State *L, int offset)
{
	WINDOW **w = lc_getwin(L, offset);
	if (*w == NULL)
		luaL_argerror(L, offset, "attempt to use closed curses window");
	return *w;
}


/***
Unique string representation of a @{curses.window}.
@function __tostring
@treturn string unique string representation of the window object.
*/
static int
W__tostring(lua_State *L)
{
	WINDOW **w = lc_getwin(L, 1);
	char buff[34];
	if (*w == NULL)
		strcpy(buff, "closed");
	else
		sprintf(buff, "%p", lua_touserdata(L, 1));
	lua_pushfstring(L, "curses window (%s)", buff);
	return 1;
}


/***
Free all the resources associated with a window.
@function close
@see delwin(3x)
*/
static int
Wclose(lua_State *L)
{
	WINDOW **w = lc_getwin(L, 1);
	if (*w != NULL && *w != stdscr)
	{
		delwin(*w);
		*w = NULL;
	}
	return 0;
}


/***
Move the position of a window.
@function move_window
@int y offset frow top of screen
@int x offset from left of screen
@treturn bool `true`, if successful
@see mvwin(3x)
*/
static int
Wmove_window(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int x = checkint(L, 3);
	return pushokresult(mvwin(w, y, x));
}


/***
Create a new subwindow.
@function sub
@treturn window a new absolutely positioned subwindow
@int nlines number of window lines
@int ncols number of window columns
@int begin_y top line of window
@int begin_x leftmost column of window
@see subwin(3x)
@see derive
*/
static int
Wsub(lua_State *L)
{
	WINDOW *orig = checkwin(L, 1);
	int nlines  = checkint(L, 2);
	int ncols   = checkint(L, 3);
	int begin_y = checkint(L, 4);
	int begin_x = checkint(L, 5);

	lc_newwin(L, subwin(orig, nlines, ncols, begin_y, begin_x));
	return 1;
}


/***
Create a new derived window.
@function derive
@int nlines number of window lines
@int ncols number of window columns
@int begin_y top line of window
@int begin_x leftmost column of window
@treturn window a new relatively positioned subwindow
@see derwin(3x)
@see sub
*/
static int
Wderive(lua_State *L)
{
	WINDOW *orig = checkwin(L, 1);
	int nlines  = checkint(L, 2);
	int ncols   = checkint(L, 3);
	int begin_y = checkint(L, 4);
	int begin_x = checkint(L, 5);

	lc_newwin(L, derwin(orig, nlines, ncols, begin_y, begin_x));
	return 1;
}


/***
Move the position of a derived window.
@function move_derived
@int par_y lines from top of parent window
@int par_x columns from left of parent window
@treturn bool `true`, if successful
@see mvderwin(3x)
*/
static int
Wmove_derived(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int par_y = checkint(L, 2);
	int par_x = checkint(L, 3);
	return pushokresult(mvderwin(w, par_y, par_x));
}


/***
Change the size of a window.
@function resize
@int height new number of lines
@int width new number of columns
@treturn bool `true`, if successful
@fixme ncurses only?
*/
static int
Wresize(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int height = checkint(L, 2);
	int width = checkint(L, 3);

	int c = wresize(w, height, width);
	if (c == ERR) return 0;

	return pushokresult(true);
}


/***
Make a duplicate of a window.
@function clone
@treturn window a new duplicate of this window
@see dupwin(3x)
*/
static int
Wclone(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	lc_newwin(L, dupwin(w));
	return 1;
}


/***
Mark ancestors of a window for refresh.
@function syncup
@see wsyncup(3x)
*/
static int
Wsyncup(lua_State *L)
{
	wsyncup(checkwin(L, 1));
	return 0;
}


/***
Automatically mark ancestors of a changed window for refresh.
@function syncok
@bool bf
@treturn bool `true`, if successful
@fixme ncurses only
*/
static int
Wsyncok(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
#if LCURSES_POSIX_COMPLIANT
	int bf = lua_toboolean(L, 2);
	return pushokresult(syncok(w, bf));
#else
	return binding_notimplemented(L, "syncok", "curses");
#endif
}


/***
Mark cursor position of ancestors of a window for refresh.
@function cursyncup
@see wcursyncup(3x)
*/
static int
Wcursyncup(lua_State *L)
{
	wcursyncup(checkwin(L, 1));
	return 0;
}


/***
Mark child windows for refresh.
@function syncdown
@see syncdown(3x)
@see refresh
*/
static int
Wsyncdown(lua_State *L)
{
	wsyncdown(checkwin(L, 1));
	return 0;
}


/***
Refresh the window terminal display from the virtual screen.
@function refresh
@treturn bool `true`, if successful
@see wrefresh(3x)
@see curses.doupdate
@see noutrefresh
*/
extern void draw_menu (lua_State *L);
static int
Wrefresh(lua_State *L)
{
	int result = wrefresh(checkwin(L, 1));
	draw_menu(L);
	return pushokresult(result);
}


/***
Copy the window backing screen to the virtual screen.
@function noutrefresh
@treturn bool `true`, if successful
@see wnoutrefresh(3x)
@see curses.doupdate
@see refresh
*/
static int
Wnoutrefresh(lua_State *L)
{
	return pushokresult(wnoutrefresh(checkwin(L, 1)));
}


/***
Mark a window as having corrupted display that needs fully redrawing.
@function redrawwin
@treturn bool `true`, if successful
@see redrawwin(3x)
@see redrawln
*/
static int
Wredrawwin(lua_State *L)
{
	return pushokresult(redrawwin(checkwin(L, 1)));
}


/***
Mark a range of lines in a window as corrupted and in need of redrawing.
@function redrawln
@int beg_line
@int num_lines
@treturn bool `true`, if successful
@see wredrawln(3x)
*/
static int
Wredrawln(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int beg_line = checkint(L, 2);
	int num_lines = checkint(L, 3);
	return pushokresult(wredrawln(w, beg_line, num_lines));
}


/***
Change the cursor position.
@function move
@int y
@int x
@treturn bool `true`, if successful
@see wmove(3x)
*/
static int
Wmove(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int x = checkint(L, 3);
	return pushokresult(wmove(w, y, x));
}


/***
Scroll the window up *n* lines.
@function scrl
@int n
@treturn bool `true`, if successful
@see wscrl(3x)
*/
static int
Wscrl(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int n = checkint(L, 2);
	return pushokresult(wscrl(w, n));
}


/***
Set the changed state of a window since the last refresh.
@function touch
@param[opt] changed
@treturn bool `true`, if successful
@see touchwin(3x)
*/
static int
Wtouch(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int changed;
	if (lua_isnoneornil(L, 2))
		changed = TRUE;
	else
		changed = lua_toboolean(L, 2);

	if (changed)
		return pushokresult(touchwin(w));
	return pushokresult(untouchwin(w));
}


/***
Mark a range of lines as changed since the last refresh.
@function touchline
@int y
@int n
@param[opt] changed
@treturn bool `true`, if successful
@see wtouchln(3x)
*/
static int
Wtouchline(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int n = checkint(L, 3);
	int changed;
	if (lua_isnoneornil(L, 4))
		changed = TRUE;
	else
		changed = lua_toboolean(L, 4);
	return pushokresult(wtouchln(w, y, n, changed));
}


/***
Has a particular window line changed since the last refresh?
@function is_linetouched
@int line
@treturn bool `true`, if successful
@see is_linetouched(3x)
*/
static int
Wis_linetouched(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int line = checkint(L, 2);
	return pushboolresult(is_linetouched(w, line));
}


/***
Has a window changed since the last refresh?
@function is_wintouched
@treturn bool `true`, if successful
@see is_wintouched(3x)
*/
static int
Wis_wintouched(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	return pushboolresult(is_wintouched(w));
}


/***
Fetch the cursor position.
@function getyx
@treturn int y coordinate of top line
@treturn int x coordinate of left column
@see getyx(3x)
*/
static int
Wgetyx(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y, x;
	getyx(w, y, x);
	lua_pushinteger(L, y);
	lua_pushinteger(L, x);
	return 2;
}


/***
Fetch the parent-relative coordinates of a subwindow.
@function getparyx
@treturn int y coordinate of top line relative to parent window
@treturn int x coordinate of left column relative to parent window
@see getparyx(3x)
*/
static int
Wgetparyx(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y, x;
	getparyx(w, y, x);
	lua_pushinteger(L, y);
	lua_pushinteger(L, x);
	return 2;
}


/***
Fetch the absolute top-left coordinates of a window.
@function getbegyx
@treturn int y coordinate of top line
@treturn int x coordinate of left column
@treturn bool `true`, if successful
@see getbegyx(3x)
*/
static int
Wgetbegyx(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y, x;
	getbegyx(w, y, x);
	lua_pushinteger(L, y);
	lua_pushinteger(L, x);
	return 2;
}


/***
Fetch the absolute bottom-right coordinates of a window.
@function getmaxyx
@treturn int y coordinate of bottom line
@treturn int x coordinate of right column
@treturn bool `true`, if successful
@see getmaxyx(3x)
*/
static int
Wgetmaxyx(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y, x;
	getmaxyx(w, y, x);
	--y;  // set aside space for the menu bar
	lua_pushinteger(L, y);
	lua_pushinteger(L, x);
	return 2;
}


/***
Draw a border around a window.
@function border
@int[opt] ls
@int[opt] rs
@int[opt] ts
@int[opt] bs
@int[opt] tl
@int[opt] tr
@int[opt] bl
@int[opt] br
@treturn bool `true`, if successful
@see wborder(3x)
@usage
  win:border (curses.ACS_VLINE, curses.ACS_VLINE,
              curses.ACS_HLINE, curses.ACS_HLINE,
	      curses.ACS_ULCORNER, curses.ACS_URCORNER,
	      curses.ACS_LLCORNER, curses.ACS_LRCORNER)
*/
static int
Wborder(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	chtype ls = optch(L, 2, 0);
	chtype rs = optch(L, 3, 0);
	chtype ts = optch(L, 4, 0);
	chtype bs = optch(L, 5, 0);
	chtype tl = optch(L, 6, 0);
	chtype tr = optch(L, 7, 0);
	chtype bl = optch(L, 8, 0);
	chtype br = optch(L, 9, 0);

	return pushokresult(wborder(w, ls, rs, ts, bs, tl, tr, bl, br));
}


/***
Draw a box around a window.
@function box
@int verch
@int horch
@treturn bool `true`, if successful
@see box(3x)
@see border
@usage
  win:box (curses.ACS_VLINE, curses.ACS_HLINE)
*/
static int
Wbox(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	chtype verch = checkch(L, 2);
	chtype horch = checkch(L, 3);

	return pushokresult(box(w, verch, horch));
}


/***
Draw a row of characters from the current cursor position.
@function hline
@int ch
@int n
@treturn bool `true`, if successful
@see whline(3x)
@see mvhline
@see vline
@usage
  _, width = win:getmaxyx ()
  win:hline (curses.ACS_HLINE, width - curs_x)
*/
static int
Whline(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	chtype ch = checkch(L, 2);
	int n = checkint(L, 3);

	return pushokresult(whline(w, ch, n));
}


/***
Draw a column of characters from the current cursor position.
@function vline
@int ch
@int n
@treturn bool `true`, if successful
@see wvline(3x)
@see hline
@see mvvline
*/
static int
Wvline(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	chtype ch = checkch(L, 2);
	int n = checkint(L, 3);

	return pushokresult(wvline(w, ch, n));
}


/***
Move the cursor, then draw a row of characters from the new cursor position.
@function mvhline
@int y
@int x
@int ch
@int n
@treturn bool `true`, if successful
@see mvwhline(3x)
*/
static int
Wmvhline(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int x = checkint(L, 3);
	chtype ch = checkch(L, 4);
	int n = checkint(L, 5);

	return pushokresult(mvwhline(w, y, x, ch, n));
}


/***
Move the cursor, then draw a column of characters from the new cursor position.
@function mvvline
@int y
@int x
@int ch
@int n
@treturn bool `true`, if successful
@see mvwvline(3x)
*/
static int
Wmvvline(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int x = checkint(L, 3);
	chtype ch = checkch(L, 4);
	int n = checkint(L, 5);

	return pushokresult(mvwvline(w, y, x, ch, n));
}


/***
Write blanks to every character position in the window.
@function erase
@treturn bool `true`, if successful
@see werase(3x)
*/
static int
Werase(lua_State *L)
{
	return pushokresult(werase(checkwin(L, 1)));
}


/***
Call @{erase} and then @{clearok}.
@function clear
@treturn bool `true`, if successful
@see wclear(3x)
*/
static int
Wclear(lua_State *L)
{
	return pushokresult(wclear(checkwin(L, 1)));
}


/***
Write blanks to every character position after the cursor.
@function clrtobot
@treturn bool `true`, if successful
@see wclrtobot(3x)
*/
static int
Wclrtobot(lua_State *L)
{
	return pushokresult(wclrtobot(checkwin(L, 1)));
}


/***
Write blanks from the cursor to the end of the current line.
@function clrtoeol
@treturn bool `true`, if successful
@see wclrtoeol(3x)
*/
static int
Wclrtoeol(lua_State *L)
{
	return pushokresult(wclrtoeol(checkwin(L, 1)));
}


/***
Advance the cursor after writing a character at the old position.
@function addch
@int ch
@treturn bool `true`, if successful
@see waddch(3x)
*/
static int
Waddch(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	chtype ch = checkch(L, 2);
	return pushokresult(waddch(w, ch));
}


/***
Call @{move}, then @{addch}.
@function mvaddch
@int y
@int x
@int ch
@treturn bool `true`, if successful
@see mvwaddch(3x)
*/
static int
Wmvaddch(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int x = checkint(L, 3);
	chtype ch = checkch(L, 4);
	return pushokresult(mvwaddch(w, y, x, ch));
}


/***
Call @{addch} then @{refresh}.
@function echoch
@int ch
@treturn bool `true`, if successful
@see wechochar(3x)
*/
static int
Wechoch(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	chtype ch = checkch(L, 2);
	return pushokresult(wechochar(w, ch));
}


/***
Copy a @{curses.chstr} starting at the current cursor position.
@function addchstr
@int chstr cs
@int[opt] n
@treturn bool `true`, if successful
@see waddchnstr(3x)
*/
static int
Waddchstr(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int n = optint(L, 3, -1);
	chstr *cs = checkchstr(L, 2);

	if (n < 0 || n > (int) cs->len)
		n = cs->len;

	return pushokresult(waddchnstr(w, cs->str, n));
}


/***
Call @{move} then @{addchstr}.
@function mvaddchstr
@int y
@int x
@int[opt] n
@treturn bool `true`, if successful
@see mvwaddchnstr(3x)
*/
static int
Wmvaddchstr(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int x = checkint(L, 3);
	int n = optint(L, 5, -1);
	chstr *cs = checkchstr(L, 4);

	if (n < 0 || n > (int) cs->len)
		n = cs->len;

	return pushokresult(mvwaddchnstr(w, y, x, cs->str, n));
}


/***
Copy a Lua string starting at the current cursor position.
@function addstr
@string str
@int[opt] n
@treturn bool `true`, if successful
@see waddnstr(3x)
*/
static int
Waddstr(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	const char *str = luaL_checkstring(L, 2);
	int n = optint(L, 3, -1);
	return pushokresult(waddnstr(w, str, n));
}


/***
Call @{move} then @{addstr}.
@function mvaddstr
@int y
@int x
@string str
@int[opt] n
@treturn bool `true`, if successful
@see mvwaddnstr(3x)
*/
static int
Wmvaddstr(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int x = checkint(L, 3);
	const char *str = luaL_checkstring(L, 4);
	int n = optint(L, 5, -1);
	return pushokresult(mvwaddnstr(w, y, x, str, n));
}


/***
Set the background attributes for subsequently written characters.
@function wbkgdset
@int ch
@see wbkgdset(3x)
*/
static int
Wwbkgdset(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	chtype ch = checkch(L, 2);
	wbkgdset(w, ch);
	return 0;
}


/***
Call @{wbkgdset} and then set the background of every position accordingly.
@function wbkgd
@int ch
@treturn bool `true`, if successful
@see wbkgd(3x)
*/
static int
Wwbkgd(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	chtype ch = checkch(L, 2);
	return pushokresult(wbkgd(w, ch));
}


/***
Fetch the current background attribute for the window.
@function getbkgd
@treturn int current window background attribute
@see getbkgd(3x)
@see wbkgd
*/
static int
Wgetbkgd(lua_State *L)
{
	return pushintresult(getbkgd(checkwin(L, 1)));
}


/***
Set the flush on interrupt behaviour for the window.
@function intrflush
@bool bf
@treturn bool `true`, if successful
@see intrflush(3x)
*/
static int
Wintrflush(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int bf = lua_toboolean(L, 2);
	return pushokresult(intrflush(w, bf));
}


/***
Set the single value keypad keys behaviour for the window.
@function keypad
@bool[opt] on
@treturn bool `true`, if successful
@see keypad(3x)
*/
static int
Wkeypad(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int bf = lua_isnoneornil(L, 2) ? 1 : lua_toboolean(L, 2);
	return pushokresult(keypad(w, bf));
}


/***
Force 8-bit (or 7-bit) input characters for the window.
@function meta
@bool on `true` to force 8-bit input characters
@treturn bool `true`, if successful
@see meta(3x)
*/
static int
Wmeta(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int bf = lua_toboolean(L, 2);
	return pushokresult(meta(w, bf));
}


/***
Force @{getch} to be non-blocking.
@function nodelay
@bool on
@treturn bool `true`, if successful
@see nodelay(3x)
*/
static int
Wnodelay(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int bf = lua_toboolean(L, 2);
	return pushokresult(nodelay(w, bf));
}


/***
For differentiating user input from terminal control sequences.
@function timeout
@int delay milliseconds, `0` for blocking, `-1` for non-blocking
@see wtimeout(3x)
*/
static int
Wtimeout(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int delay = checkint(L, 2);
	wtimeout(w, delay);
	return 0;
}


/***
Return input immediately from this window.
@function notimeout
@bool bf
@treturn bool `true`, if successful
@fixme ncurses only?
*/
static int
Wnotimeout(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int bf = lua_toboolean(L, 2);
	return pushokresult(notimeout(w, bf));
}


/***
The next call to @{refresh} will clear and completely redraw the window.
@function clearok
@bool bf
@treturn bool `true`, if successful
@see clearok(3x)
*/
static int
Wclearok(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int bf = lua_toboolean(L, 2);
	return pushokresult(clearok(w, bf));
}


/***
Use hardware character insert and delete on supporting terminals.
@function idcok
@bool bf
@treturn bool `true`, if successful
@see idcok(3x)
*/
static int
Widcok(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int bf = lua_toboolean(L, 2);
	idcok(w, bf);
	return 0;
}


/***
Use hardware line insert and delete on supporting terminals.
@function idlok
@bool bf
@treturn bool `true`, if successful
@see idlok(3x)
*/
static int
Widlok(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int bf = lua_toboolean(L, 2);
	return pushokresult(idlok(w, bf));
}


/***
No need to force synchronisation of hardware cursor.
@function leaveok
@bool bf
@treturn bool `true`, if successful
@see leaveok(3x)
*/
static int
Wleaveok(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int bf = lua_toboolean(L, 2);
	return pushokresult(leaveok(w, bf));
}


/***
Scroll up one line when the cursor writes to the last screen position.
@function scrollok
@bool bf
@treturn bool `true`, if successful
@see scrollok(3x)
*/
static int
Wscrollok(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int bf = lua_toboolean(L, 2);
	return pushokresult(scrollok(w, bf));
}


/***
Automatically call @{refresh} whenever the window content is changed.
@function immedok
@bool bf
@treturn bool `true`, if successful
@see immedok(3x)
*/
static int
Wimmedok(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
#if LCURSES_POSIX_COMPLIANT
	int bf = lua_toboolean(L, 2);
	immedok(w, bf);
	return 0;
#else
	return binding_notimplemented(L, "immedok", "curses");
#endif
}


/***
Set a software scrolling region for the window.
@function wsetscrreg
@int top top line of the scrolling region (line 0 is the first line)
@int bot bottom line of the scrolling region
@treturn bool `true`, if successful
@see wsetscrreg(3x)
*/
static int
Wwsetscrreg(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int top = checkint(L, 2);
	int bot = checkint(L, 3);
	return pushokresult(wsetscrreg(w, top, bot));
}


/***
Overlay this window on top of another non-destructively.
@function overlay
@tparam window dst destination window
@treturn bool `true`, if successful
@see overlay(3x)
@see overwrite
*/
static int
Woverlay(lua_State *L)
{
	WINDOW *srcwin = checkwin(L, 1);
	WINDOW *dstwin = checkwin(L, 2);
	return pushokresult(overlay(srcwin, dstwin));
}


/***
Destructively overwrite another window with this one.
@function overwrite
@tparam window dst destination window
@treturn bool `true`, if successful
@see overwrite(3x)
*/
static int
Woverwrite(lua_State *L)
{
	WINDOW *srcwin = checkwin(L, 1);
	WINDOW *dstwin = checkwin(L, 2);
	return pushokresult(overwrite(srcwin, dstwin));
}


/***
Overlay a rectangle of this window over another.
@function copywin
@tparam window dst destination window
@int st top row from this window
@int sl left column from this window
@int dt top row of rectangle
@int dl left column of rectangle
@int db bottom row of rectangle
@int dr right column of rectangle
@bool overlay if `true`, copy destructively like @{overlay}
@treturn bool `true`, if successful
@see copywin(3x)
*/
static int
Wcopywin(lua_State *L)
{
	WINDOW *srcwin = checkwin(L, 1);
	WINDOW *dstwin = checkwin(L, 2);
	int sminrow = checkint(L, 3);
	int smincol = checkint(L, 4);
	int dminrow = checkint(L, 5);
	int dmincol = checkint(L, 6);
	int dmaxrow = checkint(L, 7);
	int dmaxcol = checkint(L, 8);
	int woverlay = lua_toboolean(L, 9);
	return pushokresult(copywin(srcwin, dstwin, sminrow,
		smincol, dminrow, dmincol, dmaxrow, dmaxcol, woverlay));
}


/***
Delete the character under the cursor.
@function delch
@treturn bool `true`, if successful
@see wdelch(3x)
*/
static int
Wdelch(lua_State *L)
{
	return pushokresult(wdelch(checkwin(L, 1)));
}


/***
Call @{move} then @{delch}.
@function mvdelch
@int y
@int x
@treturn bool `true`, if successful
@see mvwdelch(3x)
*/
static int
Wmvdelch(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int x = checkint(L, 3);
	return pushokresult(mvwdelch(w, y, x));
}


/***
Move the lines below the cursor up, to delete the current line.
@function deleteln
@treturn bool `true`, if successful
@see wdeleteln(3x)
*/
static int
Wdeleteln(lua_State *L)
{
	return pushokresult(wdeleteln(checkwin(L, 1)));
}


/***
Move the current line and those below down one line, leaving a new blank line.
@function insertln
@treturn bool `true`, if successful
@see wdeleteln(3x)
*/
static int
Winsertln(lua_State *L)
{
	return pushokresult(winsertln(checkwin(L, 1)));
}


/***
Call @{deleteln} *n* times.
@function winsdelln
@int n
@treturn bool `true`, if successful
@see winsdelln(3x)
*/
static int
Wwinsdelln(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int n = checkint(L, 2);
	return pushokresult(winsdelln(w, n));
}


/***
Read a character from the window input.
@function getch
@treturn int character read from input buffer
@see wgetch(3x)
@see curses.cbreak
@see curses.echo
@see keypad
*/
extern void switch_to_editor (lua_State *L, const char *message);
static int
Wgetch(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int c = wgetch(w);

	if (c == ERR)
		return 0;
	if (c == 24)  /* ctrl-x */
		exit(0);
	if (c == 5)  /* ctrl-e */
		switch_to_editor(L, "");
	/* handle other standard menu hotkeys here */

	return pushintresult(c);
}


/***
Call @{move} then @{getch}
@function mvgetch
@int y
@int x
@treturn int character read from input buffer
@see mvwgetch(3x)
@see getch
*/
static int
Wmvgetch(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int x = checkint(L, 3);
	int c;

	if (wmove(w, y, x) == ERR)
		return 0;

	c = wgetch(w);

	if (c == ERR)
		return 0;

	return pushintresult(c);
}


/***
Read characters up to the next newline from the window input.
@function getstr
@int[opt] n
@treturn string string read from input buffer
@see wgetnstr(3x)
*/
static int
Wgetstr(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int n = optint(L, 2, 0);
	char buf[LUAL_BUFFERSIZE];

	if (n == 0 || n >= LUAL_BUFFERSIZE)
		n = LUAL_BUFFERSIZE - 1;
	if (wgetnstr(w, buf, n) == ERR)
		return 0;

	return pushstringresult(buf);
}


/***
Call @{move} then @{getstr}.
@function mvgetstr
@int y
@int x
@int[opt=-1] n
@treturn string string read from input buffer
@see mvwgetnstr(3x)
*/
static int
Wmvgetstr(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int x = checkint(L, 3);
	int n = optint(L, 4, -1);
	char buf[LUAL_BUFFERSIZE];

	if (n == 0 || n >= LUAL_BUFFERSIZE)
		n = LUAL_BUFFERSIZE - 1;
	if (mvwgetnstr(w, y, x, buf, n) == ERR)
		return 0;

	return pushstringresult(buf);
}


/***
Fetch the attributed character at the current cursor position.
@function winch
@treturn int attributed character read from input buffer
@see winch(3x)
*/
static int
Wwinch(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	return pushintresult(winch(w));
}


/***
Call @{move} then @{winch}
@function mvwinch
@int y
@int x
@treturn int attributed character read from input buffer
@see mvwinch(3x)
*/
static int
Wmvwinch(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int x = checkint(L, 3);
	return pushintresult(mvwinch(w, y, x));
}


/***
Fetch attributed characters from cursor position up to rightmost window position.
@function winchnstr
@int n
@treturn curses.chstr characters from cursor to end of line
@see winchnstr(3x)
@see winnstr
*/
static int
Wwinchnstr(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int n = checkint(L, 2);
	chstr *cs = chstr_new(L, n);

	if (winchnstr(w, cs->str, n) == ERR)
		return 0;

	return 1;
}


/***
Call @{move} then @{winchnstr}.
@function mvwinchnstr
@int y
@int x
@int n
@treturn curses.chstr characters from cursor to end of line
@see mvwinchnstr(3x)
*/
static int
Wmvwinchnstr(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int x = checkint(L, 3);
	int n = checkint(L, 4);
	chstr *cs = chstr_new(L, n);

	if (mvwinchnstr(w, y, x, cs->str, n) == ERR)
		return 0;

	return 1;
}


/***
Fetch a string from cursor position up to rightmost window position.
@function winnstr
@int n
@treturn string string read from input buffer
@see winnstr(3x)
@see winchnstr
*/
static int
Wwinnstr(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int n = checkint(L, 2);
	char buf[LUAL_BUFFERSIZE];
	char *ptr = buf;

	if (abs(n) >= LUAL_BUFFERSIZE)
	    ptr = lua_newuserdata(L, (size_t) abs(n) + 1);

	n = winnstr(w, ptr, n);
	if (n == ERR)
	    return 0;

	lua_pushlstring(L, ptr, n);
	return 1;
}


/***
Call @{move} then @{winnstr}.
@function mvwinnstr
@int y
@int x
@int n
@treturn string string read from input buffer
@see mvwinnstr(3x)
*/
static int
Wmvwinnstr(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int x = checkint(L, 3);
	int n = checkint(L, 4);
	char buf[LUAL_BUFFERSIZE];
	char *ptr = buf;

	if (abs(n) >= LUAL_BUFFERSIZE)
	    ptr = lua_newuserdata(L, (size_t) abs(n) + 1);

	n = mvwinnstr(w, y, x, ptr, n);
	if (n == ERR)
	    return 0;

	lua_pushlstring(L, ptr, n);
	return 1;
}


/***
Insert an attributed character before the current cursor position.
@function winsch
@int ch
@treturn bool `true`, if successful
@see winsch(3x)
*/
static int
Wwinsch(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	chtype ch = checkch(L, 2);
	return pushokresult(winsch(w, ch));
}


/***
Call @{move} then @{winsch}.
@function mvwinsch
@int y
@int x
@int ch
@treturn bool `true`, if successful
@see mvwinsch(3x)
*/
static int
Wmvwinsch(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int x = checkint(L, 3);
	chtype ch = checkch(L, 4);
	return pushokresult(mvwinsch(w, y, x, ch));
}


/***
Insert a string of characters before the current cursor position.
@function winsstr
@string str
@treturn bool `true`, if successful
@see winsstr(3x)
*/
static int
Wwinsstr(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	const char *str = luaL_checkstring(L, 2);
	return pushokresult(winsnstr(w, str, lua_strlen(L, 2)));
}


/***
Call @{move} then @{winsstr}.
@function mvwinsstr
@int y
@int x
@string str
@treturn bool `true`, if successful
@see mvwinsstr(3x)
*/
static int
Wmvwinsstr(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int x = checkint(L, 3);
	const char *str = luaL_checkstring(L, 4);
	return pushokresult(mvwinsnstr(w, y, x, str, lua_strlen(L, 2)));
}


/***
Like @{winsstr}, but no more than *n* characters.
@function winsnstr
@string str
@int n
@treturn bool `true`, if successful
@see winsnstr(3x)
*/
static int
Wwinsnstr(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	const char *str = luaL_checkstring(L, 2);
	int n = checkint(L, 3);
	return pushokresult(winsnstr(w, str, n));
}


/***
Call @{move} then @{winsnstr}.
@function mvwinsnstr
@int y
@int x
@string str
@int n
@treturn bool `true`, if successful
@see mvwinsnstr(3x)
*/
static int
Wmvwinsnstr(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int y = checkint(L, 2);
	int x = checkint(L, 3);
	const char *str = luaL_checkstring(L, 4);
	int n = checkint(L, 5);
	return pushokresult(mvwinsnstr(w, y, x, str, n));
}


/***
Return a new subpad window object.
@function subpad
@int nlines
@int ncols
@int begin_y
@int begin_x
@treturn window a new subpad window object
@see subpad(3x)
*/
static int
Wsubpad(lua_State *L)
{
	WINDOW *orig = checkwin(L, 1);
	int nlines  = checkint(L, 2);
	int ncols   = checkint(L, 3);
	int begin_y = checkint(L, 4);
	int begin_x = checkint(L, 5);

	lc_newwin(L, subpad(orig, nlines, ncols, begin_y, begin_x));
	return 1;
}


/***
Equivalent to @{refresh} for use with pad windows.
@function prefresh
@int st top row from this pad window
@int sl left column from this pad window
@int dt top row of rectangle
@int dl left column of rectangle
@int db bottom row of rectangle
@int dr right column of rectangle
@treturn bool `true`, if successful
@see prefresh(3x)
*/
static int
Wprefresh(lua_State *L)
{
	WINDOW *p = checkwin(L, 1);
	int pminrow = checkint(L, 2);
	int pmincol = checkint(L, 3);
	int sminrow = checkint(L, 4);
	int smincol = checkint(L, 5);
	int smaxrow = checkint(L, 6);
	int smaxcol = checkint(L, 7);

	return pushokresult(prefresh(p, pminrow, pmincol,
		sminrow, smincol, smaxrow, smaxcol));
}


/***
Equivalent to @{noutrefresh} for use with pad windows.
@function pnoutrefresh
@int st top row from this pad window
@int sl left column from this pad window
@int dt top row of rectangle
@int dl left column of rectangle
@int db bottom row of rectangle
@int dr right column of rectangle
@treturn bool `true`, if successful
@see pnoutrefresh(3x)
*/
static int
Wpnoutrefresh(lua_State *L)
{
	WINDOW *p = checkwin(L, 1);
	int pminrow = checkint(L, 2);
	int pmincol = checkint(L, 3);
	int sminrow = checkint(L, 4);
	int smincol = checkint(L, 5);
	int smaxrow = checkint(L, 6);
	int smaxcol = checkint(L, 7);

	return pushokresult(pnoutrefresh(p, pminrow, pmincol,
		sminrow, smincol, smaxrow, smaxcol));
}


/***
An efficient equivalent to @{addch} followed by @{refresh}.
@function pechochar
@int ch
@treturn bool `true`, if successful
@see pechochar(3x)
*/
static int
Wpechochar(lua_State *L)
{
	WINDOW *p = checkwin(L, 1);
	chtype ch = checkch(L, 2);
	return pushokresult(pechochar(p, ch));
}


/***
Turn off the given attributes for subsequent writes to the window.
@function attroff
@int attrs
@treturn bool `true`, if successful
@see wattroff(3x)
@see standend
*/
static int
Wattroff(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int attrs = checkint(L, 2);
	return pushokresult(wattroff(w, attrs));
}


/***
Turn on the given attributes for subsequent writes to the window.
@function attron
@int attrs
@treturn bool `true`, if successful
@see wattron(3x)
*/
static int
Wattron(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int attrs = checkint(L, 2);
	return pushokresult(wattron(w, attrs));
}


/***
Set the given attributes for subsequent writes to the window.
@function attrset
@int attrs
@treturn bool `true`, if successful
@see wattrset(3x)
*/
static int
Wattrset(lua_State *L)
{
	WINDOW *w = checkwin(L, 1);
	int attrs = checkint(L, 2);
	return pushokresult(wattrset(w, attrs));
}


/***
Turn off all attributes for subsequent writes to the window.
@function standend
@treturn bool `true`, if successful
@see wstandend(3x)
*/
static int
Wstandend(lua_State *L)
{
	return pushokresult(wstandend(checkwin(L, 1)));
}


/***
Set `A_STANDOUT` for subsequent writes to the window.
@function standout
@treturn bool `true`, if successful
@see wstandout(3x)
*/
static int
Wstandout(lua_State *L)
{
	return pushokresult(wstandout(checkwin(L, 1)));
}


static const luaL_Reg curses_window_fns[] =
{
	LCURSES_FUNC( W__tostring	),
	LCURSES_FUNC( Waddch		),
	LCURSES_FUNC( Waddchstr		),
	LCURSES_FUNC( Waddstr		),
	LCURSES_FUNC( Wattroff		),
	LCURSES_FUNC( Wattron		),
	LCURSES_FUNC( Wattrset		),
	LCURSES_FUNC( Wborder		),
	LCURSES_FUNC( Wbox		),
	LCURSES_FUNC( Wclear		),
	LCURSES_FUNC( Wclearok		),
	LCURSES_FUNC( Wclone		),
	LCURSES_FUNC( Wclose		),
	LCURSES_FUNC( Wclrtobot		),
	LCURSES_FUNC( Wclrtoeol		),
	LCURSES_FUNC( Wcopywin		),
	LCURSES_FUNC( Wcursyncup	),
	LCURSES_FUNC( Wdelch		),
	LCURSES_FUNC( Wdeleteln		),
	LCURSES_FUNC( Wderive		),
	LCURSES_FUNC( Wechoch		),
	LCURSES_FUNC( Werase		),
	LCURSES_FUNC( Wgetbegyx		),
	LCURSES_FUNC( Wgetbkgd		),
	LCURSES_FUNC( Wgetch		),
	LCURSES_FUNC( Wgetmaxyx		),
	LCURSES_FUNC( Wgetparyx		),
	LCURSES_FUNC( Wgetstr		),
	LCURSES_FUNC( Wgetyx		),
	LCURSES_FUNC( Whline		),
	LCURSES_FUNC( Widcok		),
	LCURSES_FUNC( Widlok		),
	LCURSES_FUNC( Wimmedok		),
	LCURSES_FUNC( Winsertln		),
	LCURSES_FUNC( Wintrflush	),
	LCURSES_FUNC( Wis_linetouched	),
	LCURSES_FUNC( Wis_wintouched	),
	LCURSES_FUNC( Wkeypad		),
	LCURSES_FUNC( Wleaveok		),
	LCURSES_FUNC( Wmeta		),
	LCURSES_FUNC( Wmove		),
	LCURSES_FUNC( Wmove_derived	),
	LCURSES_FUNC( Wmove_window	),
	LCURSES_FUNC( Wmvaddch		),
	LCURSES_FUNC( Wmvaddchstr	),
	LCURSES_FUNC( Wmvaddstr		),
	LCURSES_FUNC( Wmvdelch		),
	LCURSES_FUNC( Wmvgetch		),
	LCURSES_FUNC( Wmvgetstr		),
	LCURSES_FUNC( Wmvhline		),
	LCURSES_FUNC( Wmvvline		),
	LCURSES_FUNC( Wmvwinch		),
	LCURSES_FUNC( Wmvwinchnstr	),
	LCURSES_FUNC( Wmvwinnstr	),
	LCURSES_FUNC( Wmvwinsch		),
	LCURSES_FUNC( Wmvwinsnstr	),
	LCURSES_FUNC( Wmvwinsstr	),
	LCURSES_FUNC( Wnodelay		),
	LCURSES_FUNC( Wnotimeout	),
	LCURSES_FUNC( Wnoutrefresh	),
	LCURSES_FUNC( Woverlay		),
	LCURSES_FUNC( Woverwrite	),
	LCURSES_FUNC( Wpechochar	),
	LCURSES_FUNC( Wpnoutrefresh	),
	LCURSES_FUNC( Wprefresh		),
	LCURSES_FUNC( Wredrawln		),
	LCURSES_FUNC( Wredrawwin	),
	LCURSES_FUNC( Wrefresh		),
	LCURSES_FUNC( Wresize		),
	LCURSES_FUNC( Wscrl		),
	LCURSES_FUNC( Wscrollok		),
	LCURSES_FUNC( Wstandend		),
	LCURSES_FUNC( Wstandout		),
	LCURSES_FUNC( Wsub		),
	LCURSES_FUNC( Wsubpad		),
	LCURSES_FUNC( Wsyncdown		),
	LCURSES_FUNC( Wsyncok		),
	LCURSES_FUNC( Wsyncup		),
	LCURSES_FUNC( Wtimeout		),
	LCURSES_FUNC( Wtouch		),
	LCURSES_FUNC( Wtouchline	),
	LCURSES_FUNC( Wvline		),
	LCURSES_FUNC( Wwbkgd		),
	LCURSES_FUNC( Wwbkgdset		),
	LCURSES_FUNC( Wwinch		),
	LCURSES_FUNC( Wwinchnstr	),
	LCURSES_FUNC( Wwinnstr		),
	LCURSES_FUNC( Wwinsch		),
	LCURSES_FUNC( Wwinsdelln	),
	LCURSES_FUNC( Wwinsnstr		),
	LCURSES_FUNC( Wwinsstr		),
	LCURSES_FUNC( Wwsetscrreg	),
	{"__gc",     Wclose		}, /* rough safety net */
	{NULL, NULL}
};


LUALIB_API int
luaopen_curses_window(lua_State *L)
{
	int t, mt;

	luaL_register(L, "curses.window", curses_window_fns);
	t = lua_gettop(L);

	luaL_newmetatable(L, WINDOWMETA);
	mt = lua_gettop(L);

	lua_pushvalue(L, mt);
	lua_setfield(L, mt, "__index");		/* mt.__index = mt */
	lua_pushliteral(L, "CursesWindow");
	lua_setfield(L, mt, "_type");		/* mt._type = "Curses Window" */

	/* for k,v in pairs(t) do mt[k]=v end */
	for (lua_pushnil(L); lua_next(L, t) != 0;)
		lua_setfield(L, mt, lua_tostring(L, -2));

	lua_pop(L, 1);				/* pop mt */

	return 1;
}
