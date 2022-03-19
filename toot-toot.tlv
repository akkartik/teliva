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
  menu:
    >-- To show app-specific hotkeys in the menu bar, add hotkey/command
    >-- arrays of strings to the menu array.
    >menu = {
    >  {'^k', 'clear'},
    >  {'^w', 'write prose to file "toot" (edit hotkey does NOT save)'},
    >}
- __teliva_timestamp: original
  Window:
    >Window = curses.stdscr()
    >curses.curs_set(0)  -- we'll simulate our own cursor
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
  prose:
    >prose = ''
- __teliva_timestamp: original
  cursor:
    >cursor = 1
- __teliva_timestamp: original
  render:
    >function render(window)
    >  window:clear()
    >  debugy = 5
    >  local toots = split(prose, '\n\n===\n\n')
    >  pos = 1
    >  for i, toot in ipairs(toots) do
    >    if i > 1 then
    >      pos = render_delimiter(window, '\n\n===\n\n', pos, cursor)
    >    end
    >    pos = render_text(window, toot, pos, cursor)
    >    print('')
    >    window:attron(curses.A_BOLD)
    >    window:addstr(toot:len())
    >    window:attroff(curses.A_BOLD)
    >  end
    >  window:refresh()
    >end
- __teliva_timestamp: original
  render_delimiter:
    >function render_delimiter(window, s, pos, cursor)
    >  local newpos = pos
    >  for i=1,s:len() do
    >    if newpos == cursor and i ~= 1 then
    >      if s[i] == '\n' then
    >        -- newline at cursor = render extra space in reverse video before jumping to new line
    >        window:attron(curses.A_REVERSE)
    >        window:addch(' ')
    >        window:attroff(curses.A_REVERSE)
    >        window:addstr(s[i])
    >      else
    >        -- most characters at cursor = render in reverse video
    >        window:attron(curses.A_REVERSE)
    >        window:addstr(s[i])
    >        window:attroff(curses.A_REVERSE)
    >      end
    >    else
    >      window:addstr(s[i])
    >    end
    >    newpos = newpos+1
    >  end
    >  return newpos
    >end
- __teliva_timestamp: original
  render_text:
    >-- https://gankra.github.io/blah/text-hates-you
    >-- https://lord.io/text-editing-hates-you-too
    >
    >-- manual tests:
    >--   cursor on some character
    >--   cursor on (within) '\n\n===\n\n' delimiter (delimiter is hardcoded; things may break if you change it)
    >--   cursor at end of each line
    >--   render digits
    >
    >-- positions serve two purposes:
    >--   character to index into prose
    >--   cursor-printing
    >
    >-- sequence of stories
    >--   focus on rendering a single piece of text, first get that rock-solid
    >--   split prose into toots, manage transitions between toots in response to cursor movements
    >--   cursor movement: left/right vs up/down
    >
    >-- what is the ideal representation?
    >--   prose + cursor has issues in multi-toot context. when to display cursor?
    >function render_text(window, s, pos, cursor)
    >  local newpos = pos
    >--?   dbg(window, '--')
    >  for i=1,s:len() do
    >--?     dbg(window, tostring(newpos)..' '..tostring(string.byte(s[i])))
    >    if newpos == cursor then
    >--?       dbg(window, 'cursor: '..tostring(cursor))
    >      if s[i] == '\n' then
    >        -- newline at cursor = render extra space in reverse video before jumping to new line
    >        window:attron(curses.A_REVERSE)
    >        window:addch(' ')
    >        window:attroff(curses.A_REVERSE)
    >        window:addstr(s[i])
    >      else
    >        -- most characters at cursor = render in reverse video
    >        window:attron(curses.A_REVERSE)
    >        window:addstr(s[i])
    >        window:attroff(curses.A_REVERSE)
    >      end
    >    else
    >      window:addstr(s[i])
    >    end
    >    newpos = newpos+1
    >  end
    >  if newpos == cursor then
    >    window:attron(curses.A_REVERSE)
    >    window:addch(' ')
    >    window:attroff(curses.A_REVERSE)
    >  end
    >  return newpos
    >end
- __teliva_timestamp: original
  update:
    >function update(window)
    >  local key = window:getch()
    >  local h, w = window:getmaxyx()
    >  if key == curses.KEY_LEFT then
    >    if cursor > 1 then
    >      cursor = cursor-1
    >    end
    >  elseif key == curses.KEY_RIGHT then
    >    if cursor <= #prose then
    >      cursor = cursor+1
    >    end
    >  elseif key == curses.KEY_DOWN then
    >    cursor = cursor_down(prose, cursor, w)
    >  elseif key == curses.KEY_UP then
    >    cursor = cursor_up(prose, cursor, w)
    >  elseif key == curses.KEY_BACKSPACE or key == 8 or key == 127 then  -- ctrl-h, ctrl-?, delete
    >    if cursor > 1 then
    >      cursor = cursor-1
    >      prose = prose:remove(cursor)
    >    end
    >  elseif key == 11 then  -- ctrl-k
    >    prose = ''
    >    cursor = 1
    >  elseif key == 23 then  -- ctrl-w
    >    local out = io.open('toot', 'w')
    >    if out ~= nil then
    >      out:write(prose, '\n')
    >      out:close()
    >    end
    >  elseif key == 10 or (key >= 32 and key < 127) then
    >    prose = prose:insert(string.char(key), cursor-1)
    >    cursor = cursor+1
    >  end
    >end
- __teliva_timestamp: original
  cursor_down:
    >function cursor_down(s, old_idx, width)
    >  local max = string.len(s)
    >  local i = 1
    >  -- compute oldcol, the screen column of old_idx
    >  local oldcol = 0
    >  local col = 0
    >  while true do
    >    if i > max then
    >      -- abnormal old_idx
    >      return old_idx
    >    end
    >    if i == old_idx then
    >      oldcol = col
    >      break
    >    end
    >    if s[i] == '\n' then
    >      col = 0
    >    else
    >      col = col+1
    >    end
    >    i = i+1
    >  end
    >  -- skip rest of line
    >  while true do
    >    if i > max then
    >      -- current line is at bottom
    >      if col >= width then
    >        return i
    >      end
    >      return old_idx
    >    end
    >    if s[i] == '\n' then
    >      break
    >    end
    >    if i - old_idx >= width then
    >      return i
    >    end
    >    col = col+1
    >    i = i+1
    >  end
    >  -- compute index at same column on next line
    >  -- i is at a newline
    >  i = i+1
    >  col = 0
    >  while true do
    >    if i > max then
    >      -- next line is at bottom and is too short; position at end of it
    >      return i
    >    end
    >    if s[i] == '\n' then
    >      -- next line is too short; position at end of it
    >      return i
    >    end
    >    if col == oldcol then
    >      return i
    >    end
    >    col = col+1
    >    i = i+1
    >  end
    >end
    >
    >function test_cursor_down()
    >  -- lines that don't wrap
    >  check_eq(cursor_down('abc\ndef', 1, 5), 5, 'cursor_down: non-bottom line first char')
    >  check_eq(cursor_down('abc\ndef', 2, 5), 6, 'cursor_down: non-bottom line mid char')
    >  check_eq(cursor_down('abc\ndef', 3, 5), 7, 'cursor_down: non-bottom line final char')
    >  check_eq(cursor_down('abc\ndef', 4, 5), 8, 'cursor_down: non-bottom line end')
    >  check_eq(cursor_down('abc\ndef', 5, 5), 5, 'cursor_down: bottom line first char')
    >  check_eq(cursor_down('abc\ndef', 6, 5), 6, 'cursor_down: bottom line mid char')
    >  check_eq(cursor_down('abc\ndef', 7, 5), 7, 'cursor_down: bottom line final char')
    >  check_eq(cursor_down('abc\n\ndef', 2, 5), 5, 'cursor_down: to shorter line')
    >
    >  -- within a single wrapping line
    >  --   |abcde|  <-- wrap, no newline
    >  --   |fgh  |
    >  check_eq(cursor_down('abcdefgh', 1, 5), 6, 'cursor_down from wrapping line: first char')
    >  check_eq(cursor_down('abcdefgh', 2, 5), 7, 'cursor_down from wrapping line: mid char')
    >  check_eq(cursor_down('abcdefgh', 5, 5), 9, 'cursor_down from wrapping line: to shorter line')
    >
    >  -- within a single very long wrapping line
    >  --   |abcde|  <-- wrap, no newline
    >  --   |fghij|  <-- wrap, no newline
    >  --   |klm  |
    >  check_eq(cursor_down('abcdefghijklm', 1, 5), 6, 'cursor_down within wrapping line: first char')
    >  check_eq(cursor_down('abcdefghijklm', 2, 5), 7, 'cursor_down within wrapping line: mid char')
    >  check_eq(cursor_down('abcdefghijklm', 5, 5), 10, 'cursor_down within wrapping line: final char')
    >end
- __teliva_timestamp: original
  cursor_up:
    >function cursor_up(s, old_idx, width)
    >  local max = string.len(s)
    >  local i = 1
    >  -- compute oldcol, the screen column of old_idx
    >  local oldcol = 0
    >  local col = 0
    >  local newline_before_current_line = 0
    >  while true do
    >    if i > max or i == old_idx then
    >      oldcol = col
    >      break
    >    end
    >    if s[i] == '\n' then
    >      col = 0
    >      newline_before_current_line = i
    >    else
    >      col = col+1
    >      if col == width then
    >        col = 0
    >      end
    >    end
    >    i = i+1
    >  end
    >  -- find previous newline
    >  i = i-col-1
    >  if old_idx - newline_before_current_line > width then
    >    -- we're in a wrapped line
    >    return old_idx - width
    >  end
    >  -- scan back to start of previous line
    >  if s[i] == '\n' then
    >    i = i-1
    >  end
    >  while true do
    >    if i < 1 then
    >      -- current line is at top
    >      break
    >    end
    >    if s[i] == '\n' then
    >      break
    >    end
    >    i = i-1
    >  end
    >  -- i is at a newline
    >  i = i+1
    >  -- skip whole screen lines within previous line
    >  while newline_before_current_line - i > width do
    >    i = i + width
    >  end
    >  -- compute index at same column on previous screen line
    >  col = 0
    >  while true do
    >    if i > max then
    >      -- next line is at bottom and is too short; position at end of it
    >      return i
    >    end
    >    if s[i] == '\n' then
    >      -- next line is too short; position at end of it
    >      return i
    >    end
    >    if col == oldcol then
    >      return i
    >    end
    >    col = col+1
    >    i = i+1
    >  end
    >end
    >
    >function test_cursor_up()
    >  -- lines that don't wrap
    >  check_eq(cursor_up('abc\ndef', 1, 5), 1, 'cursor_up: top line first char')
    >  check_eq(cursor_up('abc\ndef', 2, 5), 2, 'cursor_up: top line mid char')
    >  check_eq(cursor_up('abc\ndef', 3, 5), 3, 'cursor_up: top line final char')
    >  check_eq(cursor_up('abc\ndef', 4, 5), 4, 'cursor_up: top line end')
    >  check_eq(cursor_up('abc\ndef', 5, 5), 1, 'cursor_up: non-top line first char')
    >  check_eq(cursor_up('abc\ndef', 6, 5), 2, 'cursor_up: non-top line mid char')
    >  check_eq(cursor_up('abc\ndef', 7, 5), 3, 'cursor_up: non-top line final char')
    >  check_eq(cursor_up('abc\ndef\n', 8, 5), 4, 'cursor_up: non-top line end')
    >  check_eq(cursor_up('ab\ndef\n', 7, 5), 3, 'cursor_up: to shorter line')
    >
    >  -- within a single wrapping line
    >  --   |abcde|  <-- wrap, no newline
    >  --   |fgh  |
    >  check_eq(cursor_up('abcdefgh', 6, 5), 1, 'cursor_up from wrapping line: first char')
    >  check_eq(cursor_up('abcdefgh', 7, 5), 2, 'cursor_up from wrapping line: mid char')
    >  check_eq(cursor_up('abcdefgh', 8, 5), 3, 'cursor_up from wrapping line: final char')
    >  check_eq(cursor_up('abcdefgh', 9, 5), 4, 'cursor_up from wrapping line: wrapped line end')
    >
    >  -- within a single very long wrapping line
    >  --   |abcde|  <-- wrap, no newline
    >  --   |fghij|  <-- wrap, no newline
    >  --   |klm  |
    >  check_eq(cursor_up('abcdefghijklm', 11, 5), 6, 'cursor_up within wrapping line: first char')
    >  check_eq(cursor_up('abcdefghijklm', 12, 5), 7, 'cursor_up within wrapping line: mid char')
    >  check_eq(cursor_up('abcdefghijklm', 13, 5), 8, 'cursor_up within wrapping line: final char')
    >  check_eq(cursor_up('abcdefghijklm', 14, 5), 9, 'cursor_up within wrapping line: wrapped line end')
    >
    >  -- from below to (the bottom of) a wrapping line
    >  --   |abcde|  <-- wrap, no newline
    >  --   |fg   |
    >  --   |hij  |
    >  check_eq(cursor_up('abcdefg\nhij', 9, 5), 6, 'cursor_up to wrapping line: first char')
    >  check_eq(cursor_up('abcdefg\nhij', 10, 5), 7, 'cursor_up to wrapping line: mid char')
    >  check_eq(cursor_up('abcdefg\nhij', 11, 5), 8, 'cursor_up to wrapping line: final char')
    >  check_eq(cursor_up('abcdefg\nhij', 12, 5), 8, 'cursor_up to wrapping line: to shorter line')
    >end
- __teliva_timestamp:
    >Thu Feb 17 19:52:30 2022
  doc:blurb:
    >A tiny editor (no scrolling) for composing a series of toots or tweets. Always shows character counts for current state of prose.
    >
    >Typing '===' on its own lines, surrounded by empty lines, partitions prose and gives all segments independent character counts. Good for threads (tweetstorms).
- __teliva_timestamp:
    >Fri Mar 11 09:45:27 2022
  first_toot:
    >first_toot = 1
- __teliva_timestamp:
    >Fri Mar 11 11:47:34 2022
  update:
    >function update(window)
    >  local key = window:getch()
    >  local h, w = window:getmaxyx()
    >  if key == curses.KEY_LEFT then
    >    if cursor > 1 then
    >      cursor = cursor-1
    >    end
    >  elseif key == curses.KEY_RIGHT then
    >    if cursor <= #prose then
    >      cursor = cursor+1
    >    end
    >  elseif key == curses.KEY_DOWN then
    >    cursor = cursor_down(prose, cursor, w)
    >  elseif key == curses.KEY_UP then
    >    cursor = cursor_up(prose, cursor, w)
    >  elseif key == curses.KEY_BACKSPACE or key == 8 or key == 127 then  -- ctrl-h, ctrl-?, delete
    >    if cursor > 1 then
    >      cursor = cursor-1
    >      prose = prose:remove(cursor)
    >    end
    >  elseif key == 6 then  -- ctrl-f
    >    first_toot = first_toot+1
    >  elseif key == 2 then  -- ctrl-b
    >    if first_toot > 1 then
    >      first_toot = first_toot-1
    >    end
    >  elseif key == 11 then  -- ctrl-k
    >    prose = ''
    >    cursor = 1
    >  elseif key == 23 then  -- ctrl-w
    >    local out = io.open('toot', 'w')
    >    if out ~= nil then
    >      out:write(prose, '\n')
    >      out:close()
    >    end
    >  elseif key == 10 or (key >= 32 and key < 127) then
    >    prose = prose:insert(string.char(key), cursor-1)
    >    cursor = cursor+1
    >  end
    >end
- __teliva_timestamp:
    >Fri Mar 11 11:48:43 2022
  menu:
    >-- To show app-specific hotkeys in the menu bar, add hotkey/command
    >-- arrays of strings to the menu array.
    >menu = {
    >  {'^w', 'write to "toot"'},
    >  {'^f|^b', 'scroll'},
    >  {'^k', 'clear'},
    >}
- __teliva_timestamp:
    >Sat Mar 12 08:48:44 2022
  render:
    >function render(window)
    >  window:clear()
    >  debugy = 5
    >  local toots = split(prose, '\n\n===\n\n')
    >  pos = 1
    >  for i, toot in ipairs(toots) do
    >--?     dbg(window, "render: "..i.." pos "..pos.." cursor "..cursor)
    >    if i > 1 then
    >      pos = render_delimiter(window, '\n\n===\n\n', pos, cursor)
    >--?       dbg(window, "delim: "..pos.." cursor "..cursor)
    >    end
    >    if i <= first_toot then
    >      window:clear()
    >    end
    >    pos = render_text(window, toot, pos, cursor)
    >    print('')
    >--?     dbg(window, "text: "..pos.." cursor "..cursor)
    >    window:attron(curses.A_BOLD)
    >    window:addstr(toot:len())
    >    window:attroff(curses.A_BOLD)
    >  end
    >  window:refresh()
    >end
- __teliva_timestamp:
    >Sat Mar 12 08:57:41 2022
  doc:blurb:
    >A tiny editor (no scrolling) for composing a series of toots or tweets.
    >Always shows character counts for current state of prose.
    >
    >Typing '===' on its own lines, surrounded by empty lines, partitions prose and gives all segments independent character counts. Good for threads (tweetstorms).
- __teliva_timestamp:
    >Sat Mar 12 08:59:52 2022
  __teliva_note:
    >hacky scrolling support
    >
    >Since I started out rendering a toot at a time and tracking the position
    >as I rendered each toot, the easiest way to build this was to scroll a
    >toot at a time, always render each toot and just decide when to stop
    >clearing the screen. This way I don't mess with the position computation
    >logic which is carefully synced between render and cursor_up/cursor_down.
    >
    >But there may be a more elegant approach if I was building the current state
    >from scratch.
  doc:blurb:
    >A tiny editor for composing a short series of toots or tweets. Always shows character counts for current state of prose.
    >
    >Typing '===' on its own lines, surrounded by empty lines, partitions prose and gives all segments independent character counts. Good for threads (tweetstorms).
    >
    >Scrolling support is rudimentary. Keys to scroll are independent of cursor movement, so cursor can move off the screen and confusingly 'get lost'.
