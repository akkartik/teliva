- __teliva_timestamp: foo1
  window:
    >window = curses.stdscr()
- __teliva_timestamp: foo2
  n:
    >n = 0
- __teliva_timestamp: foo3
  render:
    >function render(window)
    >  window:clear()
    >  window:attron(curses.A_BOLD)
    >  window:attron(curses.color_pair(6))
    >  window:mvaddstr(10, 10, "     ")
    >  window:mvaddstr(10, 11, n)
    >  window:attroff(curses.color_pair(6))
    >  window:attroff(curses.A_BOLD)
    >  curses.refresh()
    >end
  __teliva_note: foo
- __teliva_timestamp: foo4
  menu:
    >menu = {Enter="increment"}
- __teliva_timestamp: foo5
  update:
    >function update(window)
    >  local key = curses.getch()
    >  if key == 10 then
    >    n = n+1
    >  end
    >end
- __teliva_timestamp: foo6
  main:
    >function main()
    >  for i=1,7 do
    >    curses.init_pair(i, 0, i)
    >  end
    >  curses.init_pair(255, 15, 1)  -- reserved for Teliva error messages
    >
    >  while true do
    >    render(window)
    >    update(window)
    >  end
    >end
