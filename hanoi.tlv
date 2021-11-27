teliva_program = {
  {
    __teliva_timestamp = [==[
original]==],
    render = [==[
function render(window)
  window:clear()
  local lines, cols = window:getmaxyx()
  local line = math.floor(lines/2)
  local col = math.floor(cols/4)
  for i,t in ipairs(tower) do
    render_tower(window, line, i*col, i, t)
  end
  curses.refresh()
end]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    lines = [==[
function lines(window)
  local lines, cols = window:getmaxyx()
  return lines
end]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    pop = [==[
function pop(array)
  return table.remove(array)
end]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    window = [==[
window = curses.stdscr()]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    render_tower = [==[
function render_tower(window, line, col, tower_index, tower)
  window:attron(curses.A_BOLD)
  window:mvaddch(line+2, col, string.char(96+tower_index))
  window:attroff(curses.A_BOLD)
  window:attron(curses.color_pair(7))
  window:mvaddstr(line+1, col-6, "              ")
  window:attroff(curses.color_pair(7))
  for i, n in ipairs(tower) do
    render_disk(window, line, col, n)
    line = line - 1
  end
  for i=1,5-len(tower)+1 do
    window:attron(curses.color_pair(7))
    window:mvaddstr(line, col, "  ")
    window:attroff(curses.color_pair(7))
    line = line - 1
  end
end]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    tower = [==[
tower = {{6, 5, 4, 3, 2}, {}, {}}]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    render_disk = [==[
function render_disk(window, line, col, size)
  col = col-size+1
  for i=1,size do
    window:attron(curses.color_pair(size))
    window:mvaddstr(line, col, "  ")
    window:attroff(curses.color_pair(size))
    col = col+2
  end
end]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    main = [==[
function main()
  for i=1,7 do
    curses.init_pair(i, 0, i)
  end

  while true do
    render(window)
    update(window)
  end
end
]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    len = [==[
function len(array)
  local result = 0
  for k in pairs(array) do
    result = result+1
  end
  return result
end]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    update = [==[
function update(window)
  window:mvaddstr(lines(window)-2, 5, "tower to remove top disk from? ")
  local from = curses.getch() - 96
  window:mvaddstr(lines(window)-1, 5, "tower to stack it on? ")
  local to = curses.getch() - 96
  make_move(from, to)
end]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    make_move = [==[
function make_move(from, to)
  local disk = pop(tower[from])
  table.insert(tower[to], disk)
end]==],
  },
  {
    __teliva_timestamp = [==[
original]==],
    cols = [==[
function cols(window)
  local lines, cols = window:getmaxyx()
  return cols
end]==],
  },
}
