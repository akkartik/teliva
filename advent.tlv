teliva_program = {
  {
    __teliva_timestamp = [==[
original]==],
    norm = [==[
function norm()
  window:attrset(curses.A_NORMAL)
end]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    bold = [==[
function bold()
  window:attron(curses.A_BOLD)
end]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    rv = [==[
function rv()
  window:attron(curses.A_REVERSE)
end]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    color = [==[
function color(i)
  window:attron(curses.color_pair(i))
end]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    abbreviations = [==[
clear = curses.clear
refresh = curses.refresh
getch = curses.getch
addch = curses.addch
mvaddch = curses.mvaddch
pr = curses.addstr
mpr = curses.mvaddstr
str = tostring
num = tonumber
]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    str_helpers = [==[
-- some string helpers from http://lua-users.org/wiki/StringIndexing

-- index characters using []
getmetatable('').__index = function(str,i)
  if type(i) == 'number' then
    return string.sub(str,i,i)
  else
    return string[i]
  end
end

-- ranges using (), selected bytes using {}
getmetatable('').__call = function(str,i,j)
  if type(i)~='table' then
    return string.sub(str,i,j)
  else
    local t={}
    for k,v in ipairs(i) do
      t[k]=string.sub(str,v,v)
    end
    return table.concat(t)
  end
end

-- iterate over an ordered sequence
function q(x)
  if type(x) == 'string' then
    return x:gmatch('.')
  else
    return ipairs(x)
  end
end

-- TODO: backport utf-8 support from Lua 5.3
]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    add = [==[
add = table.insert]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    map = [==[
-- only for arrays
function map(l, f)
  result = {}
  for _, x in q(l) do
    add(result, f(x))
  end
  return result
end
]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    reduce = [==[
-- only for arrays
function reduce(l, f, init)
  result = init
  for _, x in q(l) do
    result = f(result, x)
  end
  return result
end
]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    filter = [==[
-- only for arrays
function filter(l, f)
  result = {}
  for _, x in q(l) do
    if f(x) then
      add(result, x)
    end
  end
  return result
end
]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    split = [==[
function split(s, d)
  result = {}
  for match in (s..d):gmatch("(.-)"..d) do
    add(result, match);
  end
  return result
end
]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    window = [==[
window = curses.stdscr()]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    render = [==[
function render(window)
  clear()
  -- draw stuff to screen here
  for line in io.lines("input") do
    pr(line)
    pr("\n")
  end
  refresh()
end]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    menu = [==[
menu = {}]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    update = [==[
function update(window)
  local key = curses.getch()
  -- process key here
end]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    init_colors = [==[
function init_colors()
  for i=0,7 do
    curses.init_pair(i, i, -1)
  end
  curses.init_pair(8, 7, 0)
  curses.init_pair(9, 7, 1)
  curses.init_pair(10, 7, 2)
  curses.init_pair(11, 7, 3)
  curses.init_pair(12, 7, 4)
  curses.init_pair(13, 7, 5)
  curses.init_pair(14, 7, 6)
  curses.init_pair(15, -1, 15)
  curses.init_pair(255, 15, 1)  -- reserved for Teliva error messages
end]==],
  },
  {
    main = [==[
function main()
  init_colors()

  while true do
    render(window)
    update(window)
  end
end
]==],
    __teliva_timestamp = [==[
original]==],
  },
}
