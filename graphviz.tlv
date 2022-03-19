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
    >    end
    >    for k, v in pairs(b) do
    >      if a[k] ~= v then
    >        return false
    >      end
    >    end
    >    return true
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
    >function filter(h, f)
    >  result = {}
    >  for k, v in pairs(h) do
    >    if f(k, v) then
    >      result[k] = v
    >    end
    >  end
    >  return result
    >end
- __teliva_timestamp: original
  ifilter:
    >-- only for arrays
    >function ifilter(l, f)
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
  union:
    >function union(a, b)
    >  for k, v in pairs(b) do
    >    a[k] = v
    >  end
    >  return a
    >end
- __teliva_timestamp: original
  subtract:
    >-- set subtraction
    >function subtract(a, b)
    >  for k, v in pairs(b) do
    >    a[k] = nil
    >  end
    >  return a
    >end
- __teliva_timestamp: original
  all:
    >-- universal quantifier on sets
    >function all(s, f)
    >  for k, v in pairs(s) do
    >    if not f(k, v) then
    >      return false
    >    end
    >  end
    >  return true
    >end
- __teliva_timestamp: original
  to_array:
    >-- turn a set into an array
    >-- drops values
    >function to_array(h)
    >  local result = {}
    >  for k, _ in pairs(h) do
    >    table.insert(result, k)
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
  prepend:
    >-- concatenate list 'elems' into the start of 'l', modifying 'l' in the process
    >function prepend(l, elems)
    >  for i=1,#elems do
    >    table.insert(l, i, elems[i])
    >  end
    >end
- __teliva_timestamp: original
  all_but:
    >function all_but(x, idx)
    >  if type(x) == 'table' then
    >    local result = {}
    >    for i, elem in ipairs(x) do
    >      if i ~= idx then
    >        table.insert(result,elem)
    >      end
    >    end
    >    return result
    >  elseif type(x) == 'string' then
    >    if idx < 1 then return x:sub(1) end
    >    return x:sub(1, idx-1) .. x:sub(idx+1)
    >  else
    >    error('all_but: unsupported type '..type(x))
    >  end
    >end
    >
    >function test_all_but()
    >  check_eq(all_but('', 0), '', 'all_but: empty')
    >  check_eq(all_but('abc', 0), 'abc', 'all_but: invalid low index')
    >  check_eq(all_but('abc', 4), 'abc', 'all_but: invalid high index')
    >  check_eq(all_but('abc', 1), 'bc', 'all_but: first index')
    >  check_eq(all_but('abc', 3), 'ab', 'all_but: final index')
    >  check_eq(all_but('abc', 2), 'ac', 'all_but: middle index')
    >end
- __teliva_timestamp: original
  set:
    >function set(l)
    >  local result = {}
    >  for i, elem in ipairs(l) do
    >    result[elem] = true
    >  end
    >  return result
    >end
- __teliva_timestamp: original
  set_eq:
    >function set_eq(l1, l2)
    >  return eq(set(l1), set(l2))
    >end
    >
    >function test_set_eq()
    >  check(set_eq({1}, {1}), 'set_eq: identical')
    >  check(not set_eq({1, 2}, {1, 3}), 'set_eq: different')
    >  check(set_eq({1, 2}, {2, 1}), 'set_eq: order')
    >  check(set_eq({1, 2, 2}, {2, 1}), 'set_eq: duplicates')
    >end
- __teliva_timestamp: original
  clear:
    >function clear(lines)
    >  while #lines > 0 do
    >    table.remove(lines)
    >  end
    >end
- __teliva_timestamp: original
  zap:
    >function zap(target, src)
    >  clear(target)
    >  append(target, src)
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
  doc:blurb:
    >A REPL for queries about a graph in a .dot file.
- __teliva_timestamp: original
  main:
    >function main()
    >  if #arg == 0 then
    >    Window:clear()
    >    print('restart this app with the name of a .dot file')
    >    Window:refresh()
    >    while true do Window:getch(); end
    >  end
    >  Graph = read_dot_file(arg[1])
    >
    >  while true do
    >    render(Window)
    >    update(Window)
    >  end
    >end
- __teliva_timestamp: original
  Graph:
    >Graph = {}
- __teliva_timestamp: original
  read_dot_file:
    >function read_dot_file(filename)
    >  local graph = {}
    >  local infile = start_reading(nil, filename)
    >  if infile then
    >    local chars = graphviz_buffered_reader(infile)
    >    local tokens = graphviz_tokenizer(chars)
    >    parse_graph(tokens, graph)
    >  end
    >  return graph
    >end
- __teliva_timestamp: original
  graphviz_buffered_reader:
    >-- a stream of characters that can peek up to two characters ahead at a time
    >-- returns nil when there's nothing left to read
    >function graphviz_buffered_reader(infile)
    >  return {
    >    infile = infile,
    >    peek = infile.read(1),
    >    peek2 = infile.read(1),
    >    read = function(self)
    >      local result = self.peek
    >      self.peek = self.peek2
    >      self.peek2 = self.infile.read(1)
    >      return result
    >    end,
    >  }
    >end
- __teliva_timestamp: original
  graphviz_tokenizer:
    >-- a stream of tokens that can peek up to one token at a time
    >-- returns nil when there's nothing left to read
    >function graphviz_tokenizer(chars)
    >  return {
    >    chars = chars,
    >    peek = function(self)
    >      if not self.buffer then
    >        self.buffer = self:next_token()
    >      end
    >      return self.buffer
    >    end,
    >    read = function(self)
    >      local result
    >      if self.buffer then
    >        result = self.buffer
    >        self.buffer = nil
    >      else
    >        result = self:next_token()
    >      end
    >      return result
    >    end,
    >    next_token = function(self)
    >      self:skip_whitespace_and_comments()
    >      local c = self.chars.peek
    >      if c == nil then return nil end
    >      if string.pos('/*;,', c) then
    >        -- should be skipped as comments
    >        error('unexpected character '..c)
    >      elseif string.pos('[]{}():=', c) then
    >        -- single-char tokens
    >        return self.chars:read()
    >      elseif c == '"' then
    >        return self:string()
    >      elseif c == '<' then
    >        error('html strings are not implemented yet')
    >      elseif c == '-' then
    >        if self.chars.peek2 == '-' or self.chars.peek2 == '>' then
    >          return self:edgeop()
    >        else
    >          return self:numeral()
    >        end
    >      elseif string.pos('.0123456789', c) then
    >        return self:numeral()
    >      elseif string.match(c, '[%a_]') then
    >        return self:identifier()
    >      else
    >        error('unexpected character '..str(c))
    >      end
    >    end,
    >    skip_whitespace_and_comments = function(self)
    >      while true do
    >        local c = self.chars.peek
    >        if c == nil then
    >          break  -- end of chars
    >        elseif string.match(c, '%s') then
    >          self.chars:read()
    >        elseif string.pos(',;', c) then
    >          self.chars:read()
    >        elseif c == '#' then
    >          self.chars:read()  -- skip
    >          while self.chars:read() ~= '\n' do end
    >        elseif c == '/' then
    >          local c2 = self.chars.peek2
    >          if c2 == '*' then
    >            self.chars:read()  -- skip '/'
    >            self.chars:read()  -- skip '*'
    >            while true do
    >              if self.chars.peek == '*' and self.chars.peek2 == '/' then
    >                self.chars:read()  -- skip '*'
    >                self.chars:read()  -- skip '/'
    >                break
    >              end
    >            end
    >          elseif c2 == '/' then
    >            self.chars:read()  -- skip '/'
    >            self.chars:read()  -- skip '/'
    >            while self.chars:read() ~= '\n' do end
    >          else
    >            error('unexpected character after "/": '..c)
    >          end
    >        else
    >          break
    >        end
    >      end
    >    end,
    >    string = function(self)
    >      assert(self.chars.peek == '"')
    >      local result = self.chars:read()
    >      while true do
    >        local c = self.chars.peek
    >        if c == nil then
    >          error('unterminated string literal')
    >        end
    >        result = result..self.chars:read()
    >        if c == '\\' then
    >          result = result..self.chars:read()  -- unconditionally read next char
    >        elseif c == '"' then
    >          break
    >        end
    >      end
    >      return result
    >    end,
    >    numeral = function(self)
    >      local result = ''
    >      while true do
    >        local c = self.chars.peek
    >        if c == nil then
    >          return result
    >        elseif string.pos('-.0123456789', c) then
    >          result = result..self.chars:read()
    >        else
    >          break
    >        end
    >      end
    >      if string.match(self.chars.peek, '%w') then
    >        error('invalid character after numeral '..result)
    >      end
    >      return result
    >    end,
    >    identifier = function(self)
    >      local result = ''
    >      while true do
    >        local c = self.chars.peek
    >        if c == nil then
    >          error('unterminated string literal')
    >        elseif string.match(c, '[%w_]') then
    >          result = result..self.chars:read()
    >        else
    >          break
    >        end
    >      end
    >      return result
    >    end,
    >    edgeop = function(self)
    >      return self.chars:read()..self.chars:read()
    >    end,
    >  }
    >end
    >
    >function check_tokenizer(stream, expected, msg)
    >  local infile = fake_file_stream(stream)
    >  local chars = graphviz_buffered_reader(infile)
    >  local tokens = graphviz_tokenizer(chars)
    >  check_eq(tokens:read(), expected, msg)
    >end
    >
    >function test_graphviz_tokenizer()
    >  check_tokenizer('123',           '123',       'tokenizer: numeral')
    >  check_tokenizer('  123',         '123',       'tokenizer: skips whitespace')
    >  check_tokenizer('123 124',       '123',       'tokenizer: numeral')
    >  check_tokenizer('-12.3 124',     '-12.3',     'tokenizer: numeral')
    >  check_tokenizer('a123 124',      'a123',      'tokenizer: identifier')
    >  check_tokenizer('"abc def" 124', '"abc def"', 'tokenizer: string')
    >  check_tokenizer('',              nil,         'tokenizer: eof')
    >end
- __teliva_timestamp: original
  parse_graph:
    >-- https://graphviz.org/doc/info/lang.html
    >function parse_graph(tokens, graph)
    >  local t = tokens:read()
    >  if t == 'strict' then
    >    t = tokens:read()
    >  end
    >  if t == 'graph' then
    >    error('undirected graphs not supported just yet')
    >  elseif t == 'digraph' then
    >    return parse_directed_graph(tokens, graph)
    >  else
    >    error('parse_graph: unexpected token '..t)
    >  end
    >end
- __teliva_timestamp:
    >Fri Mar 18 16:52:39 2022
  skip_attr_list:
    >function skip_attr_list(tokens)
    >  while true do
    >    local tok = tokens:read()
    >    if tok == nil then
    >      error('unterminated attr list; looked for "]" in vain')
    >    end
    >    if tok == ']' then break end
    >  end
    >end
- __teliva_timestamp:
    >Fri Mar 18 17:01:49 2022
  parse_directed_graph:
    >function parse_directed_graph(tokens, graph)
    >  tokens:read()  -- skip name
    >  assert(tokens:read() == '{')
    >  while true do
    >    if tokens:peek() == nil then
    >      error('file not terminated with "}"')
    >    end
    >    if tokens:peek() == '}' then break end
    >    local tok1 = tokens:read()
    >    if tok1 == '[' then
    >      skip_attr_list(tokens)
    >    else
    >      local tok2 = tokens:read()
    >      if tok2 == '[' then
    >        -- node_stmt
    >        skip_attr_list(tokens)
    >        -- otherwise ignore node declarations;
    >        -- we'll assume the graph is fully connected
    >        -- and we can focus on just edges
    >      elseif tok2 == '->' then
    >        -- edge_stmt
    >        local tok3 = tokens:read()
    >        if graph[tok1] == nil then
    >          graph[tok1] = {}
    >        end
    >        graph[tok1][tok3] = true
    >      elseif tok2 == '--' then
    >        error('unexpected token "--" in digraph; edges should be directed using "->"')
    >      elseif tok2 == '=' then
    >        -- id '=' id
    >        -- skip
    >        tokens:read()
    >      else
    >        error('unexpected token '..tok2)
    >      end
    >    end
    >  end
    >  assert(tokens:read() == '}')
    >end
- __teliva_timestamp:
    >Fri Mar 18 17:21:16 2022
  Focus:
    >-- The focus is a set of nodes we're constantly running
    >-- certain queries against.
    >Focus = {}
- __teliva_timestamp:
    >Fri Mar 18 17:21:40 2022
  Graph:
    >-- The set of edges parsed from the given .dot file.
    >Graph = {}
- __teliva_timestamp:
    >Fri Mar 18 17:29:25 2022
  render_focus:
    >function render_focus(window)
    >  window:attrset(curses.A_BOLD)
    >  window:mvaddstr(5, 1, 'focus: ')
    >  window:attrset(curses.A_NORMAL)
    >  for _, node in ipairs(Focus) do
    >    window:addstr(node)
    >    window:addstr(' ')
    >  end
    >
    >  window:mvaddstr(8, 0, '')
    >  local lines, cols = window:getmaxyx()
    >  for col=1,cols do
    >    window:addstr('_')
    >  end
    >end
- __teliva_timestamp:
    >Fri Mar 18 17:30:11 2022
  render_queries_on_focus:
    >function render_queries_on_focus(window)
    >  window:attrset(curses.A_BOLD)
    >  window:mvaddstr(10, 1, '')
    >  -- TODO
    >end
- __teliva_timestamp:
    >Fri Mar 18 17:30:39 2022
  render:
    >function render(window)
    >  window:clear()
    >  render_basic_stats(window)
    >  render_focus(window)
    >  render_queries_on_focus(window)
    >  window:refresh()
    >end
- __teliva_timestamp:
    >Fri Mar 18 17:35:20 2022
  sources:
    >function sources(Graph)
    >  local is_target = {}
    >  for source, targets in pairs(Graph) do
    >    for target, _ in pairs(targets) do
    >      is_target[target] = true
    >    end
    >  end
    >  local result = {}
    >  for source, _ in pairs(Graph) do
    >    if not is_target[source] then
    >      table.insert(result, source)
    >    end
    >  end
    >  return result
    >end
- __teliva_timestamp:
    >Fri Mar 18 17:35:57 2022
  render_basic_stats:
    >function render_basic_stats(window)
    >  window:attrset(curses.A_BOLD)
    >  window:mvaddstr(1, 1, 'sources: ')
    >  window:attrset(curses.A_NORMAL)
    >  local sources = sources(Graph)
    >  for _, node in ipairs(sources) do
    >    window:addstr(node)
    >    window:addstr(' ')
    >  end
    >  window:mvaddstr(3, 0, '')
    >  local lines, cols = window:getmaxyx()
    >  for col=1,cols do
    >    window:addstr('_')
    >  end
    >end
- __teliva_timestamp:
    >Fri Mar 18 17:51:18 2022
  main:
    >function main()
    >  if #arg == 0 then
    >    Window:clear()
    >    print('restart this app with the name of a .dot file')
    >    Window:refresh()
    >    while true do Window:getch(); end
    >  end
    >  for _, filename in ipairs(arg) do
    >    read_dot_file(filename, Graph)
    >  end
    >
    >  while true do
    >    render(Window)
    >    update(Window)
    >  end
    >end
- __teliva_timestamp:
    >Fri Mar 18 17:51:32 2022
  read_dot_file:
    >function read_dot_file(filename, graph)
    >  local infile = start_reading(nil, filename)
    >  if infile then
    >    local chars = graphviz_buffered_reader(infile)
    >    local tokens = graphviz_tokenizer(chars)
    >    parse_graph(tokens, graph)
    >  end
    >end
- __teliva_timestamp:
    >Fri Mar 18 18:59:24 2022
  count:
    >function count(h)
    >  local result = 0
    >  for k, v in pairs(h) do
    >    result = result+1
    >  end
    >  return result
    >end
- __teliva_timestamp:
    >Fri Mar 18 18:59:24 2022
  num_nodes:
    >function num_nodes(Graph)
    >  local nodes = {}
    >  for k, v in pairs(Graph) do
    >    nodes[k] = true
    >    for k, v in pairs(v) do
    >      nodes[k] = true
    >    end
    >  end
    >  local result = 0
    >  for k, v in pairs(nodes) do
    >    result = result+1
    >  end
    >  return result
    >end
- __teliva_timestamp:
    >Fri Mar 18 19:00:19 2022
  render_basic_stats:
    >function render_basic_stats(window)
    >  window:attrset(curses.A_BOLD)
    >  window:mvaddstr(1, 1, 'sources: ')
    >  window:attrset(curses.A_NORMAL)
    >  local sources = sources(Graph)
    >  for _, node in ipairs(sources) do
    >    window:addstr(node)
    >    window:addstr(' ')
    >  end
    >  window:attrset(curses.A_BOLD)
    >  window:addstr('size: ')
    >  window:attrset(curses.A_NORMAL)
    >  window:addstr(tostring(num_nodes(Graph)))
    >  window:addstr(' nodes')
    >  window:mvaddstr(3, 0, '')
    >  local lines, cols = window:getmaxyx()
    >  for col=1,cols do
    >    window:addstr('_')
    >  end
    >end
- __teliva_timestamp:
    >Fri Mar 18 19:01:49 2022
  main:
    >function main()
    >  if #arg == 0 then
    >    Window:clear()
    >    print('restart this app with the name of a .dot file')
    >    Window:refresh()
    >    while true do Window:getch(); end
    >  end
    >  for _, filename in ipairs(arg) do
    >    read_dot_file(filename, Graph)
    >  end
    >  Focus = sources(Graph)
    >
    >  while true do
    >    render(Window)
    >    update(Window)
    >  end
    >end
- __teliva_timestamp:
    >Fri Mar 18 19:09:56 2022
  reachable:
    >function reachable(graph, node)
    >  local reached = {}
    >  local todo = {node}
    >  while #todo > 0 do
    >    local curr = table.remove(todo)
    >    if reached[curr] == nil then
    >      reached[curr] = true
    >      local targets = graph[curr]
    >      if targets then
    >        for target, _ in pairs(graph[curr]) do
    >          table.insert(todo, target)
    >        end
    >      end
    >    end
    >  end
    >  return reached
    >end
- __teliva_timestamp:
    >Fri Mar 18 20:27:16 2022
  bold:
    >function bold(window, text)
    >  window:attrset(curses.A_BOLD)
    >  window:addstr(text)
    >  window:attrset(curses.A_NORMAL)
    >end
- __teliva_timestamp:
    >Fri Mar 18 20:30:39 2022
  render_queries_on_focus:
    >function render_queries_on_focus(window)
    >  render_reachable_sets(window)
    >end
- __teliva_timestamp:
    >Fri Mar 18 20:30:39 2022
  render_reachable_sets:
    >function render_reachable_sets(window)
    >  local deps = {}
    >  local needed_by = {}
    >  for _, node in ipairs(Focus) do
    >    deps[node] = reachable(Graph, node)
    >    for dep, _ in pairs(deps[node]) do
    >      if needed_by[dep] == nil then
    >        needed_by[dep] = {}
    >      end
    >      append(needed_by[dep], {node})
    >    end
    >  end
    >  for k, v in ipairs(needed_by) do
    >    table.sort(v)
    >  end
    >  window:mvaddstr(10, 0, '')
    >  local sets = {Focus}  -- queue
    >  local done = {}
    >  while #sets > 0 do
    >    local from_nodes = table.remove(sets, 1)
    >    if #from_nodes == 0 then break end
    >    table.sort(from_nodes)
    >    local key = table.concat(from_nodes)
    >    if done[key] == nil then
    >      done[key] = true
    >      local y, x = window:getyx()
    >      window:mvaddstr(y+2, 0, '')
    >      window:attrset(curses.A_BOLD)
    >      render_list(window, from_nodes)
    >      window:attrset(curses.A_NORMAL)
    >      window:addstr(' -> ')
    >      render_set(window, filter(needed_by, function(node, users) return set_eq(users, from_nodes) end))
    >      for i, elem in ipairs(from_nodes) do
    >        table.insert(sets, all_but(from_nodes, i))
    >      end
    >    end
    >  end
    >end
- __teliva_timestamp:
    >Fri Mar 18 20:32:18 2022
  render_list:
    >function render_list(window, l)
    >  window:addstr('{')
    >  for i, node in ipairs(l) do
    >    if i > 1 then window:addstr(' ') end
    >    window:addstr(node)
    >  end
    >  window:addstr('}')
    >end
- __teliva_timestamp:
    >Fri Mar 18 20:32:18 2022
  render_set:
    >function render_set(window, h)
    >  window:addstr('(')
    >  window:addstr(count(h))
    >  window:addstr(') ')
    >  for node, _ in pairs(h) do
    >    window:addstr(node)
    >    window:addstr(' ')
    >  end
    >end
- __teliva_timestamp:
    >Sat Mar 19 09:19:10 2022
  main:
    >function main()
    >  if #arg == 0 then
    >    Window:clear()
    >    print('restart this app with the name of a .dot file')
    >    Window:refresh()
    >    while true do Window:getch(); end
    >  end
    >  for _, filename in ipairs(arg) do
    >    read_dot_file(filename, Graph)
    >  end
    >  Focus = sources(Graph)
    >  Nodes = toposort(Graph)
    >
    >  while true do
    >    render(Window)
    >    update(Window)
    >  end
    >end
- __teliva_timestamp:
    >Sat Mar 19 09:32:33 2022
  nodes:
    >function nodes(graph)
    >  local result = {}
    >  for n, deps in pairs(graph) do
    >    result[n] = true
    >    for n, _ in pairs(deps) do
    >      result[n] = true
    >    end
    >  end
    >  return result
    >end
- __teliva_timestamp:
    >Sat Mar 19 16:27:32 2022
  toposort:
    >-- stable sort of nodes in a graph
    >-- nodes always occur before all their dependencies
    >-- disconnected nodes are in alphabetical order
    >function toposort(graph)
    >  -- non-map variables are arrays
    >  -- result = leaves in graph
    >  -- candidates = non-leaves
    >  local result = {}
    >  local resultMap = {}
    >  local candidatesMap = nodes(graph)
    >  local leavesMap = filter(candidatesMap, function(k, v) return graph[k] == nil end)
    >  local leaves = to_array(leavesMap)
    >  table.sort(leaves)
    >  union(resultMap, leavesMap)
    >  prepend(result, leaves)
    >  subtract(candidatesMap, leavesMap)
    >
    >  function in_result(x, _) return resultMap[x] end
    >  function all_deps_in_result(k, _) return all(graph[k], in_result) end
    >  while true do
    >    local oldcount = count(candidatesMap)
    >    if oldcount == 0 then break end
    >    local inducteesMap = filter(candidatesMap, all_deps_in_result)
    >    local inductees = to_array(inducteesMap)
    >    table.sort(inductees)
    >    union(resultMap, inducteesMap)
    >    prepend(result, inductees)
    >    subtract(candidatesMap, inducteesMap)
    >    if oldcount == count(candidatesMap) then
    >      error('toposort: graph is not connected')
    >    end
    >  end
    >  return result
    >end
- __teliva_timestamp:
    >Sat Mar 19 16:32:24 2022
  render_focus:
    >function render_focus(window)
    >  local y, _ = window:getyx()
    >  window:mvaddstr(y+1, 0, '')
    >  bold(window, 'focus: ')
    >  for _, node in ipairs(Focus) do
    >    window:addstr(node)
    >    window:addstr(' ')
    >  end
    >  y, _ = window:getyx()
    >  window:mvaddstr(y+1, 0, '')
    >  local lines, cols = window:getmaxyx()
    >  for col=1,cols do
    >    window:addstr('_')
    >  end
    >end
- __teliva_timestamp:
    >Sat Mar 19 16:33:19 2022
  render_basic_stats:
    >function render_basic_stats(window)
    >  bold(window, tostring(#Nodes)..' nodes: ')
    >  for i, node in ipairs(Nodes) do
    >    window:attrset(curses.A_REVERSE)
    >    window:addstr(i)
    >    window:attrset(curses.A_NORMAL)
    >    window:addstr(' ')
    >    window:addstr(node)
    >    window:addstr(' ')
    >  end
    >  local y, x = window:getyx()
    >  window:mvaddstr(y+1, 0, '')
    >  local lines, cols = window:getmaxyx()
    >  for col=1,cols do
    >    window:addstr('_')
    >  end
    >end
- __teliva_timestamp:
    >Sat Mar 19 16:35:34 2022
  render_reachable_sets:
    >function render_reachable_sets(window)
    >  local deps = {}
    >  local needed_by = {}
    >  for _, node in ipairs(Focus) do
    >    deps[node] = reachable(Graph, node)
    >    for dep, _ in pairs(deps[node]) do
    >      if needed_by[dep] == nil then
    >        needed_by[dep] = {}
    >      end
    >      append(needed_by[dep], {node})
    >    end
    >  end
    >  for k, v in ipairs(needed_by) do
    >    table.sort(v)
    >  end
    >  local sets = {Focus}  -- queue
    >  local done = {}
    >  while #sets > 0 do
    >    local from_nodes = table.remove(sets, 1)
    >    if #from_nodes == 0 then break end
    >    table.sort(from_nodes)
    >    local key = table.concat(from_nodes)
    >    if done[key] == nil then
    >      done[key] = true
    >      local y, x = window:getyx()
    >      window:mvaddstr(y+2, 0, '')
    >      window:attrset(curses.A_BOLD)
    >      render_list(window, from_nodes)
    >      window:attrset(curses.A_NORMAL)
    >      window:addstr(' -> ')
    >      render_set(window, filter(needed_by, function(node, users) return set_eq(users, from_nodes) end))
    >      for i, elem in ipairs(from_nodes) do
    >        table.insert(sets, all_but(from_nodes, i))
    >      end
    >    end
    >  end
    >end
