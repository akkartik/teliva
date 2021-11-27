teliva_program = {
  {
    __teliva_timestamp = [==[original]==],
    window = [==[
window = curses.stdscr()]==],
  },
  {
    __teliva_timestamp = [==[original]==],
    n = [==[
n = 0]==],
  },
  {
    __teliva_timestamp = [==[original]==],
    render = [==[
function render(window)
  window:clear()
  window:attron(curses.A_BOLD)
  window:attron(curses.color_pair(6))
  window:mvaddstr(10, 10, "     ")
  window:mvaddstr(10, 11, n)
  window:attroff(curses.color_pair(6))
  window:attroff(curses.A_BOLD)
  curses.refresh()
end]==],
  },
  {
    __teliva_timestamp = [==[original]==],
    menu = [==[
menu = {Enter="increment"}]==],
  },
  {
    __teliva_timestamp = [==[original]==],
    update = [==[
function update(window)
  local key = curses.getch()
  if key == 10 then
    n = n+1
  end
end]==],
  },
  {
    __teliva_timestamp = [==[original]==],
    main = [==[
function main()
  for i=1,7 do
    curses.init_pair(i, 0, i)
  end

  while true do
    render(window)
    update(window)
  end
end]==],
  },
}
