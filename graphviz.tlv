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
    >    l[#l+1] = elems[i]
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
    >  local infile = start_reading(nil, filename)
    >  local chars = graphviz_buffered_reader(infile)
    >  local tokens = graphviz_tokenizer(chars)
    >  local graph = {}
    >  parse_graph(tokens, graph)
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
    >      if string.find('/*;,', c) then
    >        -- should be skipped as comments
    >        error('unexpected character '..c)
    >      elseif string.find('[]{}():=', c) then
    >        -- single-char tokens
    >        return self.chars:read()
    >      elseif c == '"' then
    >        return self:string()
    >      elseif c == '<' then
    >        error('html strings are not implemented yet')
    >      elseif string.find('-.0123456789', c) then
    >        return self:numeral()
    >      elseif string.match(c, '[%a_]') then
    >        return self:identifier()
    >      end
    >    end,
    >    skip_whitespace_and_comments = function(self)
    >      while true do
    >        local c = self.chars.peek
    >        if c == nil then
    >          break  -- end of chars
    >        elseif string.match(c, '%s') then
    >          self.chars:read()
    >        elseif string.match(',;', c) then
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
    >        elseif string.find('-.0123456789', c) then
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