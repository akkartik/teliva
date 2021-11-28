teliva_program = {
  {
    __teliva_timestamp = [==[
original]==],
    window = [==[
window = curses.stdscr()]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    render = [==[
function render(window)
  window:clear()
  -- draw stuff to screen here
  window:attron(curses.A_BOLD)
  window:mvaddstr(1, 5, "example app")
  window:attrset(curses.A_NORMAL)
  for i=0,15 do
    window:attrset(curses.color_pair(i))
    window:mvaddstr(3+i, 5, "========================")
  end
  curses.refresh()
end]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    menu = [==[
menu = {}]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    update = [==[
function update(window)
  local key = curses.getch()
  -- process key here
end]==],
  },
  {
    init_colors = [==[
function init_colors()
  for i=0,7 do
    curses.init_pair(i, i, -1)
  end
  curses.init_pair(8, 7, 0)
  curses.init_pair(9, 7, 1)
  curses.init_pair(10, 7, 2)
  curses.init_pair(11, 7, 3)
  curses.init_pair(12, 7, 4)
  curses.init_pair(13, 7, 5)
  curses.init_pair(14, 7, 6)
  curses.init_pair(15, -1, 15)
end]==],
    __teliva_timestamp = [==[
original]==],
  },
  {
    main = [==[
function main()
  init_colors()

  while true do
    render(window)
    update(window)
  end
end
]==],
    __teliva_timestamp = [==[
original]==],
  },
}
