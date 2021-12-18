# .tlv file generated by https://github.com/akkartik/teliva
# You may edit it if you are careful; however, you may see cryptic errors if you
# violate Teliva's assumptions.
#
# .tlv files are representations of Teliva programs. Teliva programs consist of
# sequences of definitions. Each definition is a table of key/value pairs. Keys
# and values are both strings.
#
# Lines in .tlv files always follow exactly one of the following forms:
# - comment lines at the top of the file starting with '#' at column 0
# - beginnings of definitions starting with '- ' at column 0, followed by a
#   key/value pair
# - key/value pairs consisting of '  ' at column 0, containing either a
#   spaceless value on the same line, or a multi-line value
# - multiline values indented by more than 2 spaces, starting with a '>'
#
# If these constraints are violated, Teliva may unceremoniously crash. Please
# report bugs at http://akkartik.name/contact
- __teliva_timestamp: original
  str_helpers:
    >-- some string helpers from http://lua-users.org/wiki/StringIndexing
    >
    >-- index characters using []
    >getmetatable('').__index = function(str,i)
    >  if type(i) == 'number' then
    >    return string.sub(str,i,i)
    >  else
    >    return string[i]
    >  end
    >end
    >
    >-- ranges using (), selected bytes using {}
    >getmetatable('').__call = function(str,i,j)
    >  if type(i)~='table' then
    >    return string.sub(str,i,j)
    >  else
    >    local t={}
    >    for k,v in ipairs(i) do
    >      t[k]=string.sub(str,v,v)
    >    end
    >    return table.concat(t)
    >  end
    >end
    >
    >-- iterate over an ordered sequence
    >function q(x)
    >  if type(x) == 'string' then
    >    return x:gmatch('.')
    >  else
    >    return ipairs(x)
    >  end
    >end
    >
    >-- insert within string
    >function string.insert(str1, str2, pos)
    >  return str1:sub(1,pos)..str2..str1:sub(pos+1)
    >end
    >
    >function string.remove(s, pos)
    >  return s:sub(1,pos-1)..s:sub(pos+1)
    >end
    >
    >-- TODO: backport utf-8 support from Lua 5.3
- __teliva_timestamp: original
  debugy:
    >debugy = 5
- __teliva_timestamp: original
  dbg:
    >function dbg(window, s)
    >  local oldy = 0
    >  local oldx = 0
    >  oldy, oldx = window:getyx()
    >  window:mvaddstr(debugy, 60, s)
    >  debugy = debugy+1
    >  window:mvaddstr(oldy, oldx, '')
    >end
- __teliva_timestamp: original
  check_eq:
    >function check_eq(x, expected, msg)
    >  if x == expected then
    >    curses.addch('.')
    >  else
    >    print('F - '..msg)
    >    print('  expected '..tostring(expected)..' but got '..x)
    >    teliva_num_test_failures = teliva_num_test_failures + 1
    >    -- overlay first test failure on editors
    >    if teliva_first_failure == nil then
    >      teliva_first_failure = msg
    >    end
    >  end
    >end
- __teliva_timestamp: original
  map:
    >-- only for arrays
    >function map(l, f)
    >  result = {}
    >  for _, x in ipairs(l) do
    >    table.insert(result, f(x))
    >  end
    >  return result
    >end
- __teliva_timestamp: original
  reduce:
    >-- only for arrays
    >function reduce(l, f, init)
    >  result = init
    >  for _, x in ipairs(l) do
    >    result = f(result, x)
    >  end
    >  return result
    >end
- __teliva_timestamp: original
  filter:
    >-- only for arrays
    >function filter(l, f)
    >  result = {}
    >  for _, x in ipairs(l) do
    >    if f(x) then
    >      table.insert(result, x)
    >    end
    >  end
    >  return result
    >end
- __teliva_timestamp: original
  find_index:
    >function find_index(arr, x)
    >  for n, y in ipairs(arr) do
    >    if x == y then
    >      return n
    >    end
    >  end
    >end
- __teliva_timestamp: original
  trim:
    >function trim(s)
    >  return s:gsub('^%s*', ''):gsub('%s*$', '')
    >end
- __teliva_timestamp: original
  split:
    >function split(s, d)
    >  result = {}
    >  for match in (s..d):gmatch("(.-)"..d) do
    >    table.insert(result, match);
    >  end
    >  return result
    >end
- __teliva_timestamp: original
  window:
    >window = curses.stdscr()
- __teliva_timestamp: original
  render:
    >function render(window)
    >  window:clear()
    >  -- draw stuff to screen here
    >  window:attron(curses.A_BOLD)
    >  window:mvaddstr(1, 5, "example app")
    >  window:attrset(curses.A_NORMAL)
    >  for i=0,15 do
    >    window:attrset(curses.color_pair(i))
    >    window:mvaddstr(3+i, 5, "========================")
    >  end
    >  curses.refresh()
    >end
- __teliva_timestamp: original
  menu:
    >menu = {}
- __teliva_timestamp: original
  update:
    >function update(window)
    >  local key = curses.getch()
    >  -- process key here
    >end
- __teliva_timestamp: original
  init_colors:
    >function init_colors()
    >  for i=0,7 do
    >    curses.init_pair(i, i, -1)
    >  end
    >  curses.init_pair(8, 7, 0)
    >  curses.init_pair(9, 7, 1)
    >  curses.init_pair(10, 7, 2)
    >  curses.init_pair(11, 7, 3)
    >  curses.init_pair(12, 7, 4)
    >  curses.init_pair(13, 7, 5)
    >  curses.init_pair(14, 7, 6)
    >  curses.init_pair(15, -1, 15)
    >end
- main:
    >function main()
    >  init_colors()
    >
    >  while true do
    >    render(window)
    >    update(window)
    >  end
    >end
  __teliva_timestamp: original
