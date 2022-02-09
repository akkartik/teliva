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
  spaces:
    >function spaces(n)
    >  for i=1,n do
    >    curses.addch(' ')
    >  end
    >end
- __teliva_timestamp: original
  window:
    >window = curses.stdscr()
- __teliva_timestamp: original
  menu:
    >-- To show app-specific hotkeys in the menu bar, add hotkey/command
    >-- arrays of strings to the menu array.
    >menu = {
    >  {'^e', 'edit'},
    >}
- __teliva_timestamp: original
  init_colors:
    >function init_colors()
    >  -- light background
    >  curses.init_pair(view_settings.current_zettel_bg, 236, 230)
    >  curses.init_pair(1, 236, 250)
    >  curses.init_pair(2, 236, 252)
    >  -- dark background
    >--?   curses.init_pair(view_settings.current_zettel_bg, 252, 130)
    >--?   curses.init_pair(1, 252, 240)
    >--?   curses.init_pair(2, 252, 242)
    >end
- __teliva_timestamp: original
  main:
    >function main()
    >  init_colors()
    >  current_zettel_id = zettels.root
    >
    >  while true do
    >    render(window)
    >    update(window)
    >  end
    >end
- __teliva_timestamp: original
  depth:
    >function depth(zettel)
    >  local result = 0
    >  while zettel.parent do
    >    result = result+1
    >    zettel = zettel.parent
    >  end
    >  return result
    >end
- __teliva_timestamp: original
  render_zettel:
    >function render_zettel(window, bg, indent, starty, startx, zettel)
    >  window:attrset(curses.color_pair(bg))
    >  for y=0,view_settings.height-1 do
    >    for x=0,view_settings.width-1 do
    >      window:mvaddch(y+starty, x+startx, ' ')
    >    end
    >  end
    >  local y, x = 0, indent+1
    >  for i=1,#zettel.data do
    >    local c = zettel.data[i]
    >    if c == '\n' then
    >      y = y+1
    >      x = indent+1
    >    else
    >      window:mvaddstr(y+starty, x+startx, c)
    >      x = x+1
    >      if x >= startx + view_settings.width then
    >        y = y+1
    >        x = indent+1
    >      end
    >    end
    >    if y >= view_settings.height then
    >      break
    >    end
    >  end
    >end
- __teliva_timestamp: original
  current_zettel_id:
    >current_zettel_id = ''
- __teliva_timestamp: original
  view_settings:
    >view_settings = {
    >  -- dimensions for rendering a single zettel; extra text gets truncated
    >  width=50,
    >  height=3,
    >  -- spacing between zettels
    >  hmargin=1,
    >  vmargin=1,
    >  --
    >  indent=2,  -- how children of a zettel are indicated
    >  current_zettel_bg=3,  -- color pair index initialized in init_colors
    >}
- __teliva_timestamp: original
  zettels:
    >zettels = {
    >  root="a",
    >  a={
    >    data="abc\ndef",
    >    child="c",
    >    next="b",
    >  },
    >  b={
    >    data="ghi\njklm",
    >    prev="a",
    >  },
    >  c={
    >    data="c",
    >    parent="a",
    >    next="d",
    >  },
    >  d={
    >    data="d",
    >    parent="a",
    >    prev="c",
    >  }
    >}
- __teliva_timestamp: original
  render_state:
    >-- some information about what's been drawn on screen
    >render_state = {
    >  -- where the current zettel is, in units of zettels
    >  curr_h = 1,
    >  curr_w = 1,
    >  -- what zettel is at each position on screen, in units of zettels
    >  hw2id = {},
    >}
- __teliva_timestamp: original
  update:
    >function update(window)
    >  local key = curses.getch()
    >  local curr = zettels[current_zettel_id]
    >  -- graph-based navigation
    >  if key == string.byte('j') then
    >    if curr.child then
    >      current_zettel_id = curr.child
    >    elseif curr.next then
    >      current_zettel_id = curr.next
    >    elseif curr.parent and zettels[curr.parent].next then
    >      current_zettel_id = zettels[curr.parent].next
    >    end
    >  elseif key == string.byte('k') then
    >    if curr.parent then current_zettel_id = curr.parent end
    >  elseif key == string.byte('h') then
    >    if curr.prev then
    >      current_zettel_id = curr.prev
    >    elseif curr.parent then
    >      current_zettel_id = curr.parent
    >    end
    >  elseif key == string.byte('l') then
    >    if curr.next then
    >      current_zettel_id = curr.next
    >    elseif curr.parent and zettels[curr.parent].next then
    >      current_zettel_id = zettels[curr.parent].next
    >    end
    >  -- screen-based navigation
    >  elseif key == curses.KEY_UP then
    >    if render_state.curr_h > 1 then
    >      current_zettel_id = render_state.wh2id[render_state.curr_w][render_state.curr_h - 1]
    >    end
    >  elseif key == curses.KEY_DOWN then
    >    if render_state.wh2id[render_state.curr_w][render_state.curr_h + 1] then
    >      current_zettel_id = render_state.wh2id[render_state.curr_w][render_state.curr_h + 1]
    >    end
    >  elseif key == curses.KEY_LEFT then
    >    if render_state.curr_w > 1 then
    >      current_zettel_id = render_state.wh2id[render_state.curr_w - 1][render_state.curr_h]
    >    end
    >  elseif key == curses.KEY_RIGHT then
    >    if render_state.wh2id[render_state.curr_w + 1] and render_state.wh2id[render_state.curr_w + 1][render_state.curr_h] then
    >      current_zettel_id = render_state.wh2id[render_state.curr_w + 1][render_state.curr_h]
    >    end
    >  --
    >  elseif key == 5 then  -- ctrl-e
    >    local old_menu = menu
    >    editz(window)
    >    menu = old_menu
    >  end
    >end
- __teliva_timestamp: original
  render:
    >function render(window)
    >  window:clear()
    >  local lines, cols = window:getmaxyx()
    >  local bg=1
    >  local y, x = 0, 0 -- units of characters (0-based)
    >  local w, h = 1, 1 -- units of zettels (1-based)
    >  -- render zettels depth-first, while tracking relative positions
    >  local done = {}
    >  local inprogress = {zettels.root}
    >  render_state.wh2id = {{}}
    >  while #inprogress > 0 do
    >    local currid = table.remove(inprogress)
    >    if not done[currid] then
    >      done[currid] = true
    >      table.insert(render_state.wh2id[w], currid)
    >      local zettel = zettels[currid]
    >      if currid == current_zettel_id then
    >        render_state.curr_w = w
    >        render_state.curr_h = h
    >      end
    >      local currbg = (currid == current_zettel_id) and view_settings.current_zettel_bg or bg
    >      render_zettel(window, currbg, depth(zettel) * view_settings.indent, y, x, zettel)
    >      if zettel.next then table.insert(inprogress, zettel.next) end
    >      if zettel.child then table.insert(inprogress, zettel.child) end
    >      bg = 3 - bg  -- toggle between color pairs 1 and 2
    >      y = y + view_settings.height + view_settings.vmargin
    >      h = h + 1
    >      if y + view_settings.height > lines then
    >        y = 0
    >        h = 1
    >        x = x + view_settings.width + view_settings.hmargin
    >        w = w + 1
    >        if x + view_settings.width > cols then break end
    >        table.insert(render_state.wh2id, {})
    >      end
    >    end
    >  end
    >  window:mvaddstr(lines-2, 0, '')
    >  for i=1,3 do
    >    window:attrset(curses.color_pair(i%2+1))
    >    window:addstr('')
    >    spaces(view_settings.width-string.len(''))
    >    window:attrset(curses.color_pair(0))
    >    window:addstr(' ')  -- margin
    >  end
    >  window:mvaddstr(lines-1, 0, '? ')
    >  curses.refresh()
    >end
- __teliva_timestamp: original
  view_settings:
    >view_settings = {
    >  -- dimensions for rendering a single zettel; extra text gets truncated
    >  width=50,
    >  height=3,
    >  -- spacing between zettels
    >  hmargin=1,
    >  vmargin=1,
    >  --
    >  indent=2,  -- how children of a zettel are indicated
    >  current_zettel_bg=3,  -- color pair index initialized in init_colors
    >}
- __teliva_timestamp: original
  editz:
    >function editz()
    >  menu = { {'^e', 'back to browsing'},}
    >  local top = (render_state.curr_h - 1) * (view_settings.height + view_settings.vmargin)
    >  local bottom = top + view_settings.height
    >  local left = (render_state.curr_w - 1) * (view_settings.width + view_settings.hmargin)
    >  local right = left + view_settings.width
    >  local cursor = 1
    >  curses.curs_set(0)
    >  local quit = false
    >  while not quit do
    >    editz_render(window, zettels[current_zettel_id].data, cursor, top, bottom, left, right)
    >    quit, zettels[current_zettel_id].data, cursor = editz_update(window, zettels[current_zettel_id].data, cursor)
    >  end
    >  curses.curs_set(1)
    >end
- __teliva_timestamp: original
  editz_render:
    >function editz_render(window, s, cursor, top, minbottom, left, right)
    >  local h, w = window:getmaxyx()
    >  window:attrset(curses.color_pair(view_settings.current_zettel_bg))
    >  for y=top,minbottom-1 do
    >    for x=left,right-1 do
    >      window:mvaddch(y, x, ' ')
    >    end
    >  end
    >  local y, x = top, left + 1  -- left padding; TODO: indent
    >  window:mvaddstr(y, x, '')
    >  for i=1,string.len(s) do
    >    -- render character
    >    if i == cursor then
    >      if s[i] == '\n' then
    >        -- newline at cursor = render extra space in reverse video before jumping to new line
    >        window:attron(curses.A_REVERSE)
    >        window:addch(' ')
    >        window:attroff(curses.A_REVERSE)
    >      else
    >        -- most characters at cursor = render in reverse video
    >        window:attron(curses.A_REVERSE)
    >        window:addstr(s[i])
    >        window:attroff(curses.A_REVERSE)
    >      end
    >    else
    >      if s[i] ~= '\n' then
    >        window:addstr(s[i])
    >      end
    >    end
    >    -- update cursor position
    >    if s[i] == '\n' then
    >      if i == cursor then x = x + 1; end
    >      for col=x,right-1 do window:addch(' '); end
    >      x = left
    >      y = y + 1
    >      if y >= h-2 then return end
    >      window:mvaddstr(y, x, '')
    >      for col=x,right-1 do window:addch(' '); end
    >      x = left + 1  -- left padding; TODO: indent
    >      window:mvaddstr(y, x, '')
    >    else
    >      x = x + 1
    >      if x >= right then
    >        y = y + 1
    >        if y >= h-2 then return end
    >        x = left + 1  -- left padding; TODO: indent
    >        window:mvaddstr(y, x, '')
    >      end
    >    end
    >  end
    >  if cursor > string.len(s) then
    >    window:attron(curses.A_REVERSE)
    >    window:addch(' ')
    >    window:attroff(curses.A_REVERSE)
    >  else
    >    window:addch(' ')
    >  end
    >end
- __teliva_timestamp: original
  editz_update:
    >function editz_update(window, prose, cursor)
    >  local key = curses.getch()
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
    >  elseif key == 5 then  -- ctrl-e
    >    return true, prose, cursor
    >  elseif key == 10 or (key >= 32 and key < 127) then
    >    prose = prose:insert(string.char(key), cursor-1)
    >    cursor = cursor+1
    >  end
    >  return false, prose, cursor
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
  __teliva_note:
    >initial commit: show/edit zettels
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
    >Wed Feb  9 08:15:25 2022
  render:
    >function render(window)
    >  window:clear()
    >  local lines, cols = window:getmaxyx()
    >  local bg=1
    >  local y, x = 0, 0 -- units of characters (0-based)
    >  local w, h = 1, 1 -- units of zettels (1-based)
    >  -- render zettels depth-first, while tracking relative positions
    >  local done = {}
    >  local inprogress = {zettels.root}
    >  render_state.wh2id = {{}}
    >  while #inprogress > 0 do
    >    local currid = table.remove(inprogress)
    >    if not done[currid] then
    >      done[currid] = true
    >      table.insert(render_state.wh2id[w], currid)
    >      local zettel = zettels[currid]
    >      if currid == current_zettel_id then
    >        render_state.curr_w = w
    >        render_state.curr_h = h
    >      end
    >      local currbg = (currid == current_zettel_id) and view_settings.current_zettel_bg or bg
    >      render_zettel(window, currbg, depth(zettel) * view_settings.indent, y, x, zettel)
    >      if zettel.next then table.insert(inprogress, zettel.next) end
    >      if zettel.child then table.insert(inprogress, zettel.child) end
    >      bg = 3 - bg  -- toggle between color pairs 1 and 2
    >      y = y + view_settings.height + view_settings.vmargin
    >      h = h + 1
    >      if y + view_settings.height > lines then
    >        y = 0
    >        h = 1
    >        x = x + view_settings.width + view_settings.hmargin
    >        w = w + 1
    >        if x + view_settings.width > cols then break end
    >        table.insert(render_state.wh2id, {})
    >      end
    >    end
    >  end
    >  window:mvaddstr(lines-1, 0, '')
    >  for i=1,3 do
    >    window:attrset(curses.color_pair(i%2+1))
    >    window:addstr('')
    >    spaces(view_settings.width-string.len(''))
    >    window:attrset(curses.color_pair(0))
    >    window:addstr(' ')  -- margin
    >  end
    >  curses.refresh()
    >end
- __teliva_timestamp:
    >Wed Feb  9 08:15:35 2022
  main:
    >function main()
    >  init_colors()
    >  current_zettel_id = zettels.root
    >
    >  curses.curs_set(0)
    >  while true do
    >    render(window)
    >    update(window)
    >  end
    >end
- __teliva_timestamp:
    >Wed Feb  9 08:16:24 2022
  __teliva_note:
    >get rid of commandline
    >
    >There's a reason vim hides it. Confusing to have two cursors on screen.
  editz:
    >function editz()
    >  menu = { {'^e', 'back to browsing'},}
    >  local top = (render_state.curr_h - 1) * (view_settings.height + view_settings.vmargin)
    >  local bottom = top + view_settings.height
    >  local left = (render_state.curr_w - 1) * (view_settings.width + view_settings.hmargin)
    >  local right = left + view_settings.width
    >  local cursor = 1
    >  local quit = false
    >  while not quit do
    >    editz_render(window, zettels[current_zettel_id].data, cursor, top, bottom, left, right)
    >    quit, zettels[current_zettel_id].data, cursor = editz_update(window, zettels[current_zettel_id].data, cursor)
    >  end
    >end
- __teliva_timestamp:
    >Wed Feb  9 08:22:20 2022
  editz_render:
    >function editz_render(window, s, cursor, top, minbottom, left, right)
    >  local h, w = window:getmaxyx()
    >  local cursor_y, cursor_x, cursor_c = 0, 0, 'c'
    >  window:attrset(curses.color_pair(view_settings.current_zettel_bg))
    >  for y=top,minbottom-1 do
    >    for x=left,right-1 do
    >      window:mvaddch(y, x, ' ')
    >    end
    >  end
    >  local y, x = top, left + 1  -- left padding; TODO: indent
    >  window:mvaddstr(y, x, '')
    >  for i=1,string.len(s) do
    >    if i == cursor then
    >      cursor_y = y
    >      cursor_x = x
    >      cursor_c = s[i]
    >    end
    >    if s[i] ~= '\n' then
    >      window:addstr(s[i])
    >      x = x + 1
    >      if x >= right then
    >        y = y + 1
    >        if y >= h-2 then return end
    >        x = left + 1  -- left padding; TODO: indent
    >        window:mvaddstr(y, x, '')
    >      end
    >    else
    >      for col=x+1,right-1 do window:addch(' '); end
    >      x = left
    >      y = y + 1
    >      if y >= h-2 then return end
    >      window:mvaddstr(y, x, '')
    >      for col=x,right-1 do window:addch(' '); end
    >      x = left + 1  -- left padding; TODO: indent
    >      window:mvaddstr(y, x, '')
    >    end
    >  end
    >  if cursor_y == 0 and cursor_x == 0 then
    >    cursor_y = y
    >    cursor_x = x
    >  end
    >  window:mvaddstr(cursor_y, cursor_x, cursor_c)
    >end
- __teliva_timestamp:
    >Wed Feb  9 08:25:05 2022
  editz:
    >function editz()
    >  menu = { {'^e', 'back to browsing'},}
    >  local top = (render_state.curr_h - 1) * (view_settings.height + view_settings.vmargin)
    >  local bottom = top + view_settings.height
    >  local left = (render_state.curr_w - 1) * (view_settings.width + view_settings.hmargin)
    >  local right = left + view_settings.width
    >  local cursor = string.len(zettels[current_zettel_id].data)
    >  local quit = false
    >  curses.curs_set(1)
    >  while not quit do
    >    editz_render(window, zettels[current_zettel_id].data, cursor, top, bottom, left, right)
    >    quit, zettels[current_zettel_id].data, cursor = editz_update(window, zettels[current_zettel_id].data, cursor)
    >  end
    >  curses.curs_set(0)
    >end
- __teliva_timestamp:
    >Wed Feb  9 08:28:13 2022
  __teliva_note:
    >stop simulating the cursor
    >
    >editz_render is now much simpler
  editz_update:
    >function editz_update(window, prose, cursor)
    >  local key = curses.getch()
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
    >  elseif key == 5 then  -- ctrl-e
    >    return true, prose, cursor
    >  elseif key == 10 or (key >= 32 and key < 127) then
    >    prose = prose:insert(string.char(key), cursor)
    >    cursor = cursor+1
    >  end
    >  return false, prose, cursor
    >end
