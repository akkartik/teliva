--- Lua bindings for curses
local M = curses

function M.addch (c) return curses.stdscr():addch(c) end
function M.mvaddch (y, x, c) return curses.stdscr():mvaddch(y, x, c) end
function M.addstr (s) return curses.stdscr():addstr(s) end
function M.mvaddstr (y, x, s) return curses.stdscr():mvaddstr(y, x, s) end

function M.attron (a) return curses.stdscr():attron(a) end
function M.attroff (a) return curses.stdscr():attroff(a) end
function M.attrset (a) return curses.stdscr():attrset(a) end

function M.clear ()    return curses.stdscr():clear() end
function M.clrtobot () return curses.stdscr():clrtobot() end
function M.clrtoeol () return curses.stdscr():clrtoeol() end

function M.getch () return curses.stdscr():getch() end
function M.mvgetch (y, x) return curses.stdscr():getch(y, x) end
function M.getstr (s) return curses.stdscr():getstr(s) end
function M.mvgetstr (y, x, s) return curses.stdscr():mvgetstr(y, x, s) end

function M.getyx ()    return curses.stdscr():getyx() end
function M.getmaxyx () return curses.stdscr():getmaxyx() end
function M.keypad (b)  return curses.stdscr():keypad(b) end
function M.move (y,x)  return curses.stdscr():move(y,x) end
function M.refresh ()  return curses.stdscr():refresh() end
function M.timeout (t) return curses.stdscr():timeout(t) end

return M
