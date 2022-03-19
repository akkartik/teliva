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
    >    return str:sub(i,i)
    >  else
    >    return string[i]
    >  end
    >end
    >
    >-- ranges using (), selected bytes using {}
    >getmetatable('').__call = function(str,i,j)
    >  if type(i)~='table' then
    >    return str:sub(i,j)
    >  else
    >    local t={}
    >    for k,v in ipairs(i) do
    >      t[k]=str:sub(v,v)
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
    >function string.pos(s, sub)
    >  return string.find(s, sub, 1, true)  -- plain=true to disable regular expressions
    >end
    >
    >-- TODO: backport utf-8 support from Lua 5.3
- __teliva_timestamp: original
  debugy:
    >debugy = 5
- __teliva_timestamp: original
  dbg:
    >-- helper for debug by print; overlay debug information towards the right
    >-- reset debugy every time you refresh screen
    >function dbg(window, s)
    >  local oldy = 0
    >  local oldx = 0
    >  oldy, oldx = window:getyx()
    >  window:mvaddstr(debugy, 60, s)
    >  debugy = debugy+1
    >  window:mvaddstr(oldy, oldx, '')
    >end
- __teliva_timestamp: original
  check:
    >function check(x, msg)
    >  if x then
    >    Window:addch('.')
    >  else
    >    print('F - '..msg)
    >    print('  '..str(x)..' is false/nil')
    >    teliva_num_test_failures = teliva_num_test_failures + 1
    >    -- overlay first test failure on editors
    >    if teliva_first_failure == nil then
    >      teliva_first_failure = msg
    >    end
    >  end
    >end
- __teliva_timestamp: original
  check_eq:
    >function check_eq(x, expected, msg)
    >  if eq(x, expected) then
    >    Window:addch('.')
    >  else
    >    print('F - '..msg)
    >    print('  expected '..str(expected)..' but got '..str(x))
    >    teliva_num_test_failures = teliva_num_test_failures + 1
    >    -- overlay first test failure on editors
    >    if teliva_first_failure == nil then
    >      teliva_first_failure = msg
    >    end
    >  end
    >end
- __teliva_timestamp: original
  eq:
    >function eq(a, b)
    >  if type(a) ~= type(b) then return false end
    >  if type(a) == 'table' then
    >    if #a ~= #b then return false end
    >    for k, v in pairs(a) do
    >      if b[k] ~= v then
    >        return false
    >      end
    >      return true
    >    end
    >  end
    >  return a == b
    >end
- __teliva_timestamp: original
  str:
    >-- smarter tostring
    >-- slow; used only for debugging
    >function str(x)
    >  if type(x) == 'table' then
    >    local result = ''
    >    result = result..#x..'{'
    >    for k, v in pairs(x) do
    >      result = result..str(k)..'='..str(v)..', '
    >    end
    >    result = result..'}'
    >    return result
    >  elseif type(x) == 'string' then
    >    return '"'..x..'"'
    >  end
    >  return tostring(x)
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
  sort_letters:
    >function sort_letters(s)
    >  tmp = {}
    >  for i=1,#s do
    >    table.insert(tmp, s[i])
    >  end
    >  table.sort(tmp)
    >  local result = ''
    >  for _, c in pairs(tmp) do
    >    result = result..c
    >  end
    >  return result
    >end
    >
    >function test_sort_letters(s)
    >  check_eq(sort_letters(''), '', 'test_sort_letters: empty')
    >  check_eq(sort_letters('ba'), 'ab', 'test_sort_letters: non-empty')
    >  check_eq(sort_letters('abba'), 'aabb', 'test_sort_letters: duplicates')
    >end
- __teliva_timestamp: original
  count_letters:
    >function count_letters(s)
    >  local result = {}
    >  for i=1,s:len() do
    >    local c = s[i]
    >    if result[c] == nil then
    >      result[c] = 1
    >    else
    >      result[c] = result[c] + 1
    >    end
    >  end
    >  return result
    >end
- __teliva_timestamp: original
  append:
    >-- concatenate list 'elems' into 'l', modifying 'l' in the process
    >function append(l, elems)
    >  for i=1,#elems do
    >    table.insert(l, elems[i])
    >  end
    >end
- __teliva_timestamp: original
  menu:
    >-- To show app-specific hotkeys in the menu bar, add hotkey/command
    >-- arrays of strings to the menu array.
    >menu = {}
- __teliva_timestamp: original
  Window:
    >Window = curses.stdscr()
- __teliva_timestamp: original
  window:
    >-- constructor for fake screen and window
    >-- call it like this:
    >--   local w = window{
    >--     kbd=kbd('abc'),
    >--     scr=scr{h=5, w=4},
    >--   }
    >-- eventually it'll do everything a real ncurses window can
    >function window(h)
    >  h.__index = h
    >  setmetatable(h, h)
    >  h.__index = function(table, key)
    >    return rawget(h, key)
    >  end
    >  h.attrset = function(self, x)
    >    self.scr.attrs = x
    >  end
    >  h.attron = function(self, x)
    >    -- currently same as attrset since Lua 5.1 doesn't have bitwise operators
    >    -- doesn't support multiple attrs at once
    >--    local old = self.scr.attrs
    >--    self.scr.attrs = old|x
    >    self.scr.attrs = x
    >  end
    >  h.attroff = function(self, x)
    >    -- currently borked since Lua 5.1 doesn't have bitwise operators
    >    -- doesn't support multiple attrs at once
    >--    local old = self.scr.attrs
    >--    self.scr.attrs = old & (~x)
    >    self.scr.attrs = curses.A_NORMAL
    >  end
    >  h.getch = function(self)
    >    local c = table.remove(h.kbd, 1)
    >    if c == nil then return c end
    >    return string.byte(c)  -- for verisimilitude with ncurses
    >  end
    >  h.addch = function(self, c)
    >    local scr = self.scr
    >    if c == '\n' then
    >      scr.cursy = scr.cursy+1
    >      scr.cursx = 0
    >      return
    >    end
    >    if scr.cursy <= scr.h then
    >      scr[scr.cursy][scr.cursx] = {data=c, attrs=scr.attrs}
    >      scr.cursx = scr.cursx+1
    >      if scr.cursx > scr.w then
    >        scr.cursy = scr.cursy+1
    >        scr.cursx = 1
    >      end
    >    end
    >  end
    >  h.addstr = function(self, s)
    >    for i=1,s:len() do
    >      self:addch(s[i])
    >    end
    >  end
    >  h.mvaddch = function(self, y, x, c)
    >    self.scr.cursy = y
    >    self.scr.cursx = x
    >    self:addch(c)
    >  end
    >  h.mvaddstr = function(self, y, x, s)
    >    self.scr.cursy = y
    >    self.scr.cursx = x
    >    self:addstr(s)
    >  end
    >  h.clear = function(self)
    >    clear_scr(self.scr)
    >  end
    >  h.refresh = function(self)
    >    -- nothing
    >  end
    >  return h
    >end
- __teliva_timestamp: original
  kbd:
    >function kbd(keys)
    >  local result = {}
    >  for i=1,keys:len() do
    >    table.insert(result, keys[i])
    >  end
    >  return result
    >end
- __teliva_timestamp: original
  scr:
    >function scr(props)
    >  props.cursx = 1
    >  props.cursy = 1
    >  clear_scr(props)
    >  return props
    >end
- __teliva_timestamp: original
  clear_scr:
    >function clear_scr(props)
    >  props.cursy = 1
    >  props.cursx = 1
    >  for y=1,props.h do
    >    props[y] = {}
    >    for x=1,props.w do
    >      props[y][x] = {data=' ', attrs=curses.A_NORMAL}
    >    end
    >  end
    >  return props
    >end
- __teliva_timestamp: original
  check_screen:
    >function check_screen(window, contents, message)
    >  local x, y = 1, 1
    >  for i=1,contents:len() do
    >    check_eq(window.scr[y][x].data, contents[i], message..'/'..y..','..x)
    >    x = x+1
    >    if x > window.scr.w then
    >      y = y+1
    >      x = 1
    >    end
    >  end
    >end
    >
    >-- putting it all together, an example test of both keyboard and screen
    >function test_check_screen()
    >  local lines = {
    >    c='123',
    >    d='234',
    >    a='345',
    >    b='456',
    >  }
    >  local w = window{
    >    kbd=kbd('abc'),
    >    scr=scr{h=3, w=5},
    >  }
    >  local y = 1
    >  while true do
    >    local b = w:getch()
    >    if b == nil then break end
    >    w:mvaddstr(y, 1, lines[string.char(b)])
    >    y = y+1
    >  end
    >  check_screen(w, '345  '..
    >                  '456  '..
    >                  '123  ',
    >              'test_check_screen')
    >end
- __teliva_timestamp: original
  check_reverse:
    >function check_reverse(window, contents, message)
    >  local x, y = 1, 1
    >  for i=1,contents:len() do
    >    if contents[i] ~= ' ' then
    >      -- hacky version while we're without bitwise operators on Lua 5.1
    >--      check(window.scr[y][x].attrs & curses.A_REVERSE, message..'/'..y..','..x)
    >      check_eq(window.scr[y][x].attrs, curses.A_REVERSE, message..'/'..y..','..x)
    >    else
    >      -- hacky version while we're without bitwise operators on Lua 5.1
    >--      check(window.scr[y][x].attrs & (~curses.A_REVERSE), message..'/'..y..','..x)
    >      check(window.scr[y][x].attrs ~= curses.A_REVERSE, message..'/'..y..','..x)
    >    end
    >    x = x+1
    >    if x > window.scr.w then
    >      y = y+1
    >      x = 1
    >    end
    >  end
    >end
- __teliva_timestamp: original
  check_bold:
    >function check_bold(window, contents, message)
    >  local x, y = 1, 1
    >  for i=1,contents:len() do
    >    if contents[i] ~= ' ' then
    >      -- hacky version while we're without bitwise operators on Lua 5.1
    >--      check(window.scr[y][x].attrs & curses.A_BOLD, message..'/'..y..','..x)
    >      check_eq(window.scr[y][x].attrs, curses.A_BOLD, message..'/'..y..','..x)
    >    else
    >      -- hacky version while we're without bitwise operators on Lua 5.1
    >--      check(window.scr[y][x].attrs & (~curses.A_BOLD), message..'/'..y..','..x)
    >      check(window.scr[y][x].attrs ~= curses.A_BOLD, message..'/'..y..','..x)
    >    end
    >    x = x+1
    >    if x > window.scr.w then
    >      y = y+1
    >      x = 1
    >    end
    >  end
    >end
- __teliva_timestamp: original
  check_color:
    >-- check which parts of a screen have the given color_pair
    >function check_color(window, cp, contents, message)
    >  local x, y = 1, 1
    >  for i=1,contents:len() do
    >    if contents[i] ~= ' ' then
    >      -- hacky version while we're without bitwise operators on Lua 5.1
    >--      check(window.scr[y][x].attrs & curses.color_pair(cp), message..'/'..y..','..x)
    >      check_eq(window.scr[y][x].attrs, curses.color_pair(cp), message..'/'..y..','..x)
    >    else
    >      -- hacky version while we're without bitwise operators on Lua 5.1
    >--      check(window.scr[y][x].attrs & (~curses.A_BOLD), message..'/'..y..','..x)
    >      check(window.scr[y][x].attrs ~= curses.color_pair(cp), message..'/'..y..','..x)
    >    end
    >    x = x+1
    >    if x > window.scr.w then
    >      y = y+1
    >      x = 1
    >    end
    >  end
    >end
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
    >  window:refresh()
    >end
- __teliva_timestamp: original
  update:
    >function update(window)
    >  local key = window:getch()
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
- __teliva_timestamp: original
  main:
    >function main()
    >  init_colors()
    >
    >  while true do
    >    render(Window)
    >    update(Window)
    >  end
    >end
- __teliva_timestamp: original
  doc:blurb:
    >To show a brief description of the app on the 'big picture' screen, put the text in a special buffer called 'doc:blurb'.
    >
    >You can also override the default big picture screen entirely by creating a buffer called 'doc:main'.
