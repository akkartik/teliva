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
  menu:
    >-- To show app-specific hotkeys in the menu bar, add hotkey/command
    >-- arrays of strings to the menu array.
    >menu = {}
- __teliva_timestamp: original
  Window:
    >Window = curses.stdscr()
- __teliva_timestamp: original
  main:
    >function main()
    >  task.spawn(main_task)
    >  task.scheduler()
    >  Window:refresh()
    >  Window:getch()
    >end
- __teliva_timestamp: original
  main_task:
    >function main_task()
    >  Window:clear()
    >  local c = task.Channel:new()
    >  task.spawn(counter, c)
    >  for i=1,10 do
    >    print(c:recv())
    >  end
    >end
- __teliva_timestamp:
    >Sat Feb 26 21:50:11 2022
  __teliva_note:
    >a simple counter
  counter:
    >function counter(c)
    >  local i = 2
    >  while true do
    >    c:send(i)
    >    i = i+1
    >  end
    >end
- __teliva_timestamp:
    >Sat Feb 26 21:54:53 2022
  filter_task:
    >function filter_task(p, cin, cout)
    >  while true do
    >    local i = cin:recv()
    >    if i%p ~= 0 then
    >      cout:send(i)
    >    end
    >  end
    >end
- __teliva_timestamp:
    >Sat Feb 26 21:55:46 2022
  main_task:
    >function main_task()
    >  local primes = task.Channel:new()
    >  task.spawn(sieve, primes)
    >  for i=1,10 do
    >    print(primes:recv())
    >  end
    >end
- __teliva_timestamp:
    >Sat Feb 26 21:59:37 2022
  __teliva_note:
    >filter out multiples of a single number
  sieve:
    >function sieve(ch)
    >  local iota = task.Channel:new()
    >  task.spawn(counter, iota)
    >  task.spawn(filter_task, 2, iota, ch)
    >end
- __teliva_timestamp:
    >Sat Feb 26 22:08:07 2022
  __teliva_note:
    >implement the complete sieve algorithm
  sieve:
    >-- Set up a Sieve of Eratosthenes (https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes)
    >-- for computing prime numbers by chaining tasks, one per prime.
    >-- Each task is responsible for filtering out all multiples of its prime.
    >function sieve(primes_ch)
    >  local c = task.Channel:new()
    >  task.spawn(counter, c)
    >  while true do
    >    local p, newc = c:recv(), task.Channel:new()
    >    primes_ch:send(p)
    >    task.spawn(filter_task, p, c, newc)
    >    c = newc
    >  end
    >end
- __teliva_timestamp:
    >Sat Feb 26 22:09:47 2022
  __teliva_note:
    >infinite primes
  main_task:
    >function main_task()
    >  local primes = task.Channel:new()
    >  task.spawn(sieve, primes)
    >  while true do
    >    Window:addstr(primes:recv())
    >    Window:addstr(' ')
    >    Window:refresh()
    >  end
    >end
- __teliva_timestamp:
    >Sat Feb 26 22:09:47 2022
  __teliva_note:
    >clear screen when it fills up; pause on keypress
    >
    >In Teliva getch() implicitly refreshes the screen.
  main_task:
    >function main_task()
    >  Window:nodelay(true)
    >  Window:clear()
    >  local primes = task.Channel:new()
    >  task.spawn(sieve, primes)
    >  local h, w = Window:getmaxyx()
    >  while true do
    >    Window:addstr(primes:recv())
    >    Window:addstr(' ')
    >    local c = Window:getch()
    >    if c then break end  -- key pressed
    >    local y, x = Window:getyx()
    >    if y > h-1 then
    >      Window:clear()
    >    end
    >  end
    >  print('key pressed; done')
    >  Window:nodelay(false)
    >end
- __teliva_timestamp:
    >Sat Feb 26 22:27:25 2022
  doc:blurb:
    >Sieve of Eratosthenes
    >https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes
    >
    >A demonstration of tasks and channels, the primitives for (cooperative) concurrency in Teliva.
    >
    >We string together a cascade of tasks connected by channels. Every prime number gets a new task that prints the first incoming number, and then filters out multiples of it from the incoming channel.
    >
    >This approach has the advantage that we don't need to create an array of n numbers to compute primes less than n.
    >
    >However, we still need to create p tasks and p channels if there are p primes less than n. Probably not worth it, given tasks and channels are much larger than numbers. This is just a demo.
    >
    >The noticeable periodic pauses are perhaps due to garbage collection.
