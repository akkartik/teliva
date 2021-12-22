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
  append:
    >function append(target, src)
    >  for _, line in ipairs(src) do
    >    table.insert(target, line)
    >  end
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
  window:
    >window = curses.stdscr()
- __teliva_timestamp: original
  render_line:
    >function render_line(window, y, line)
    >  window:mvaddstr(y, 0, '')
    >  for i=1,string.len(line) do
    >    window:addstr(line[i])
    >  end
    >end
- __teliva_timestamp: original
  render_link:
    >function render_link(window, y, line)
    >  local rendered_line = string.gsub(line, '=>%s*%S*%s*', '')
    >  if trim(rendered_line) == '' then
    >    rendered_line = line
    >  end
    >  render_line(window, y, rendered_line)
    >end
- __teliva_timestamp: original
  state:
    >state = {
    >  lines={},
    >  history={},
    >  highlight_index=0,
    >  source=false,  -- show source (link urls, etc.)
    >}
- __teliva_timestamp: original
  render_page:
    >function render_page(window)
    >  local y = 0
    >  window:attron(curses.color_pair(6))
    >  print(state.url)
    >  window:attroff(curses.color_pair(6))
    >  y = y+2
    >--?   dbg(window, state.highlight_index)
    >  for i, line in pairs(state.lines) do
    >    if not state.source and string.find(line, '=> ') == 1 then
    >      if state.highlight_index == 0 or i == state.highlight_index then
    >        -- highlighted link
    >        state.highlight_index = i  -- TODO: ugly state update while rendering, just for first render after gemini_get
    >        curses.attron(curses.A_REVERSE)
    >        render_link(window, y, line)
    >        curses.attroff(curses.A_REVERSE)
    >      else
    >        -- link
    >        curses.attron(curses.A_BOLD)
    >        render_link(window, y, line)
    >        curses.attroff(curses.A_BOLD)
    >      end
    >    else
    >      -- non-link
    >      render_line(window, y, line)
    >    end
    >    y = y+1
    >  end
    >end
- __teliva_timestamp: original
  render:
    >function render(window, lines)
    >  window:clear()
    >  render_page(window, lines)
    >  curses.curs_set(0)
    >  window:refresh()
    >end
- __teliva_timestamp: original
  menu:
    >menu = {}
    >menu['enter'] = 'go to highlight'
    >menu['←'] = 'back'
    >menu['^g'] = 'enter url'
    >menu['^u'] = 'view source'
- __teliva_timestamp: original
  edit_line:
    >function edit_line(window)
    >  local result = ''
    >  local cursor = 1
    >  local screen_rows, screen_cols = window:getmaxyx()
    >  while true do
    >    window:mvaddstr(screen_rows-1, 9, '')
    >    window:clrtoeol()
    >    window:mvaddstr(screen_rows-1, 9, result)
    >    window:attron(curses.A_REVERSE)
    >    -- window:refresh()
    >    local key = curses.getch()
    >    window:attrset(curses.A_NORMAL)
    >    if key >= 32 and key < 127 then
    >      local screen_rows, screen_cols = window:getmaxyx()
    >      if #result < screen_cols then
    >        result = result:insert(string.char(key), cursor-1)
    >        cursor = cursor+1
    >      end
    >    elseif key == curses.KEY_LEFT then
    >      if cursor > 1 then
    >        cursor = cursor-1
    >      end
    >    elseif key == curses.KEY_RIGHT then
    >      if cursor <= #result then
    >        cursor = cursor+1
    >      end
    >    elseif key == curses.KEY_BACKSPACE then
    >      if cursor > 1 then
    >        cursor = cursor-1
    >        result = result:remove(cursor)
    >      end
    >    elseif key == 21 then  -- ctrl-u
    >      result = ''
    >      cursor = 1
    >    elseif key == 10 then  -- enter
    >      return result
    >    elseif key == 24 then  -- ctrl-x
    >      return nil
    >    end
    >  end
    >end
- __teliva_timestamp: original
  is_link:
    >function is_link(line)
    >  return string.find(line, '=>%s*%S*%s*') == 1
    >end
- __teliva_timestamp: original
  next_link:
    >function next_link()
    >  local new_index = state.highlight_index
    >  while true do
    >    new_index = new_index+1
    >    if new_index > #state.lines then return end
    >    if is_link(state.lines[new_index]) then break end
    >  end
    >  state.highlight_index = new_index
    >end
- __teliva_timestamp: original
  previous_link:
    >function previous_link()
    >  local new_index = state.highlight_index
    >  while true do
    >    new_index = new_index - 1
    >    if new_index < 1 then return end
    >    if is_link(state.lines[new_index]) then break end
    >  end
    >  state.highlight_index = new_index
    >end
- __teliva_timestamp: original
  update:
    >function update(window)
    >  local key = curses.getch()
    >  local screen_rows, screen_cols = window:getmaxyx()
    >  if key == 258 then  -- down arrow
    >    next_link()
    >  elseif key == 259 then  -- up arrow
    >    previous_link()
    >  elseif key == 260 then  -- left arrow
    >    if #state.history > 1 then
    >      table.remove(state.history)
    >      gemini_get(table.remove(state.history))
    >    end
    >  elseif key == 21 then  -- ctrl-u
    >    state.source = not state.source
    >  elseif key == 10 then  -- enter
    >    local s, e, new_url = string.find(state.lines[state.highlight_index], '=>%s*(%S*)')
    >    gemini_get(url.absolute(state.url, new_url))
    >  elseif key == 7 then  -- ctrl-g
    >    window:mvaddstr(screen_rows-2, 0, '')
    >    window:clrtoeol()
    >    window:mvaddstr(screen_rows-1, 0, '')
    >    window:clrtoeol()
    >    window:mvaddstr(screen_rows-1, 5, 'go: ')
    >    curses.curs_set(2)
    >    local new_url = edit_line(window)
    >    if new_url then
    >      state.url = new_url
    >      gemini_get(new_url)
    >    end
    >    curses.curs_set(0)
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
  main:
    >function main()
    >  init_colors()
    >  local lines = {}
    >  local url = ''
    >  if #arg > 0 then
    >    state.url = arg[1]
    >    lines = gemini_get(state.url)
    >  end
    >  while true do
    >    render(window, lines)
    >    update(window, lines)
    >  end
    >end
- __teliva_timestamp: original
  http_get:
    >function http_get(url)
    >  -- https://stackoverflow.com/questions/42445423/luasocket-serveraccept-timeout-tcp
    >  local parsed_url = socket.url.parse(url)
    >  local tcp = socket.tcp()
    >  tcp:connect(parsed_url.host, 80)
    >  tcp:send('GET / HTTP/1.1\r\n')
    >  -- http requires the Host header
    >  tcp:send(string.format('Host: %s\r\n', parsed_url.host))
    >  tcp:send('\r\n')
    >  -- tcp:receive('*a') doesn't seem to detect when a request is done
    >  -- so we have to manage the size of the expected response
    >  headers = {}
    >  while true do
    >    local s, status = tcp:receive()
    >    if s == nil then break end
    >    if s == '' then break end
    >    local header, value = string.match(s, '(.-): (.*)')
    >    if header == nil then
    >      print(s)
    >    else
    >      headers[string.lower(header)] = value
    >      print(header, value)
    >    end
    >  end
    >  local bytes_remaining = tonumber(headers['content-length'])
    >  body = ''
    >  while true do
    >    local s, status = tcp:receive(bytes_remaining)
    >    if s == nil then break end
    >    body = body .. s
    >    bytes_remaining = bytes_remaining - string.len(s)
    >    if bytes_remaining <= 0 then break end
    >  end
    >  return body
    >end
- __teliva_timestamp: original
  https_get:
    >-- http://notebook.kulchenko.com/programming/https-ssl-calls-with-lua-and-luasec
    >function https_get(url)
    >  local parsed_url = socket.url.parse(url)
    >  local params = {
    >    mode = 'client',
    >    protocol = 'any',
    >    verify = 'none',  -- I don't know what I'm doing
    >    options = 'all',
    >  }
    >  local conn = socket.tcp()
    >  conn:connect(parsed_url.host, parsed_url.port or 443)
    >  conn, err = ssl.wrap(conn, params)
    >  if conn == nil then
    >      io.write(err)
    >      os.exit(1)
    >  end
    >  conn:dohandshake()
    >
    >  conn:send(url .. "\r\n")
    >  local line, err = conn:receive()
    >  return line or err
    >end
- __teliva_timestamp: original
  parse_gemini_body:
    >function parse_gemini_body(conn, type)
    >  if type == 'text/gemini' then
    >    while true do
    >      local line, err = conn:receive()
    >      if line == nil then break end
    >      table.insert(state.lines, line)
    >    end
    >  elseif string.sub(type, 1, 5) == 'text/' then
    >    while true do
    >      local line, err = conn:receive()
    >      if line == nil then break end
    >      table.insert(state.lines, line)
    >    end
    >  end
    >end
- __teliva_timestamp: original
  gemini_get:
    >-- http://notebook.kulchenko.com/programming/https-ssl-calls-with-lua-and-luasec
    >-- https://tildegit.org/solderpunk/gemini-demo-2
    >-- returns an array of lines, containing either the body or just an error
    >function gemini_get(url)
    >  if string.find(url, "://") == nil then
    >    url = "gemini://" .. url
    >  end
    >  local parsed_url = socket.url.parse(url)
    >  local params = {
    >    mode = 'client',
    >    protocol = 'any',
    >    verify = 'none',  -- I don't know what I'm doing
    >    options = 'all',
    >  }
    >  local conn = socket.tcp()
    >  conn:connect(parsed_url.host, parsed_url.port or 1965)
    >  conn, err = ssl.wrap(conn, params)
    >  if conn == nil then
    >      io.write(err)
    >      os.exit(1)
    >  end
    >  conn:dohandshake()
    >  conn:send(url .. "\r\n")
    >  clear(state.lines)
    >  state.highlight_index = 0  -- highlighted link not computed yet
    >  local line, err = conn:receive()
    >  if line == nil then
    >    table.insert(state.lines, err)
    >    return
    >  end
    >  local status, meta = string.match(line, "(%S+) (%S+)")
    >  if status[1] == '2' then
    >    parse_gemini_body(conn, meta)
    >    state.url = url
    >    table.insert(state.history, url)
    >  elseif status[1] == '3' then
    >    gemini_get(socket.url.absolute(url, meta))
    >  elseif status[1] == '4' or line[1] == '5' then
    >    table.insert(state.lines, 'Error: '..meta)
    >  else
    >    table.insert(state.lines, 'invalid response from server: '..line)
    >  end
    >end
