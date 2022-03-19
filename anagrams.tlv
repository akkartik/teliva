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
    >menu = {
    >  {'^h', 'backspace'},
    >}
- __teliva_timestamp: original
  Window:
    >Window = curses.stdscr()
- __teliva_timestamp: original
  doc:blurb:
    >Show all anagrams of a given word
- __teliva_timestamp: original
  word:
    >word = ''
- __teliva_timestamp: original
  cursor:
    >cursor = 1
- __teliva_timestamp: original
  main:
    >function main()
    >  Window:nodelay(true)
    >  while true do
    >    render(Window)
    >    update(Window)
    >  end
    >end
- __teliva_timestamp: original
  update:
    >function update(window)
    >  local key
    >  while true do
    >    key = window:getch()
    >    if key then break end
    >  end
    >  if key == curses.KEY_LEFT then
    >    if cursor > 1 then
    >      cursor = cursor-1
    >    end
    >  elseif key == curses.KEY_RIGHT then
    >    if cursor <= #word then
    >      cursor = cursor+1
    >    end
    >  elseif key == curses.KEY_BACKSPACE or key == 8 or key == 127 then  -- ctrl-h, ctrl-?, delete
    >    if cursor > 1 then
    >      cursor = cursor-1
    >      word = word:remove(cursor)
    >    end
    >  elseif key >= 32 and key < 127 then
    >    word = word:insert(string.char(key), cursor-1)
    >    cursor = cursor+1
    >  end
    >end
- __teliva_timestamp: original
  render:
    >function render(window)
    >  window:clear()
    >
    >  local prompt_str = ' what is your name? '
    >  window:attron(curses.A_REVERSE)
    >  window:mvaddstr(0, 0, prompt_str)
    >  window:attroff(curses.A_REVERSE)
    >  window:addstr(' ')
    >  window:attron(curses.A_BOLD)
    >  window:addstr(word)
    >  window:attroff(curses.A_BOLD)
    >  window:mvaddstr(2, 0, '')
    >  local results = anagrams(word)
    >  if #results > 0 then
    >    window:attron(curses.A_REVERSE)
    >    print(#results..' anagrams')
    >    window:attroff(curses.A_REVERSE)
    >    for i, w in ipairs(results) do
    >      window:addstr(w)
    >      window:addstr(' ')
    >    end
    >  end
    >
    >  window:mvaddstr(0, string.len(prompt_str)+cursor, '')
    >  window:refresh()
    >end
- __teliva_timestamp:
    >Mon Feb 21 17:42:28 2022
  anagrams:
    >function anagrams(word)
    >  return gather(sort_letters(word))
    >end
- __teliva_timestamp:
    >Mon Feb 21 18:18:07 2022
  gather:
    >function gather(s)
    >  if s == '' then return {} end
    >  local result = {}
    >  for i=1, #s do
    >    if i == 1 or s[i] ~= s[i-1] then
    >      append(result, combine(s[i], gather(all_but(s, i))))
    >    end
    >  end
    >  return result
    >end
- __teliva_timestamp: original
  __teliva_note:
    >basic version
  combine:
    >-- return 'l' with each element prefixed with 'prefix'
    >function combine(prefix, l)
    >  if #l == 0 then return {prefix} end
    >  local result = {}
    >  for _, elem in ipairs(l) do
    >    table.insert(result, prefix..elem)
    >  end
    >  return result
    >end
    >
    >function test_combine()
    >  check_eq(combine('abc', {}), {'abc'}, 'test_combine: empty list')
    >end
- __teliva_timestamp:
    >Sat Mar  5 15:24:00 2022
  count_anagrams:
    >function count_anagrams(s)
    >  local result = factorial(s:len())
    >  local letter_counts = count_letters(s)
    >  for k, v in pairs(letter_counts) do
    >    result = result / factorial(v)
    >  end
    >  return result
    >end
- __teliva_timestamp:
    >Sat Mar  5 15:24:34 2022
  factorial:
    >function factorial(n)
    >  local result = 1
    >  for i=1,n do
    >    result = result*i
    >  end
    >  return result
    >end
- __teliva_timestamp:
    >Sat Mar  5 15:53:23 2022
  key_pressed:
    >-- only works when nodelay (non-blocking keyboard)
    >function key_pressed(window)
    >  local c = window:getch()
    >  if c == nil then return false end
    >  window:ungetch(c)
    >  return true
    >end
- __teliva_timestamp:
    >Sat Mar  5 15:55:34 2022
  render:
    >function render(window)
    >  window:clear()
    >
    >  local prompt_str = ' what is your name? '
    >  window:attron(curses.A_REVERSE)
    >  window:mvaddstr(0, 0, prompt_str)
    >  window:attroff(curses.A_REVERSE)
    >  window:addstr(' ')
    >  window:attron(curses.A_BOLD)
    >  window:addstr(word)
    >  window:attroff(curses.A_BOLD)
    >  window:mvaddstr(2, 0, '')
    >  if #word > 0 then
    >    local num_anagrams = count_anagrams(word)
    >    window:attron(curses.A_REVERSE)
    >    print(num_anagrams..' anagrams')
    >    window:attroff(curses.A_REVERSE)
    >    local results = anagrams(word)
    >    if results == nil then  -- interrupted
    >      window:addstr('...')
    >    else
    >      assert(#results == num_anagrams, "something's wrong; the count is unexpected")
    >      for i, w in ipairs(results) do
    >        window:addstr(w)
    >        window:addstr(' ')
    >        if key_pressed(window) then
    >          break
    >        end
    >      end
    >    end
    >  end
    >
    >  window:mvaddstr(0, string.len(prompt_str)+cursor, '')
    >  window:refresh()
    >end
- __teliva_timestamp:
    >Sat Mar  5 15:56:35 2022
  __teliva_note:
    >restart computation when a key is pressed
  gather:
    >-- return a list of unique permutations of a sorted string 's'
    >-- the letters in 's' must be in alphabetical order, so that duplicates are adjacent
    >-- this function can take a long time for long strings, so we make it interruptible
    >-- if a key is pressed, it returns nil
    >-- since it's recursive, we also need to handle recursive calls returning nil
    >function gather(s)
    >  if s == '' then return {} end
    >  local result = {}
    >  for i=1, #s do
    >    if i == 1 or s[i] ~= s[i-1] then
    >      local subresult = gather(all_but(s, i))
    >      if subresult == nil then return nil end  -- interrupted
    >      append(result, combine(s[i], subresult))
    >    end
    >    if key_pressed(Window) then return nil end  -- interrupted
    >  end
    >  return result
    >end
