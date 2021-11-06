local curses = require "curses"


tower = {{5, 4, 3, 2, 1}, {}, {}}


local function len(array)
  local result = 0
  for k in pairs(array) do
    result = result+1
  end
  return result
end


local function pop(array)
  return table.remove(array)
end

local function lines(window)
  local lines, cols = window:getmaxyx()
  return lines
end

local function cols(window)
  local lines, cols = window:getmaxyx()
  return cols
end


local function render_tower(window, line, col, tower_index, tower)
  window:attron(curses.A_BOLD)
  window:mvaddch(line+2, col, string.char(96+tower_index))
  window:attroff(curses.A_BOLD)
  window:mvaddstr(line+1, col-3, "-------")
  for i, n in ipairs(tower) do
    window:mvaddstr(line, col, n)
    line = line - 1
  end
  for i=1,5-len(tower) do
    window:mvaddstr(line, col, "|")
    line = line - 1
  end
end


local function render(window)
  window:clear()
  local lines, cols = window:getmaxyx()
  local line = math.floor(lines/2)
  local col = math.floor(cols/4)
  for i,t in ipairs(tower) do
    render_tower(window, line, i*col, i, t)
  end
  curses.refresh()
end


local function make_move(from, to)
  local disk = pop(tower[from])
  table.insert(tower[to], disk)
end


local function update(window)
  window:mvaddstr(lines(window)-2, 5, "tower to remove top disk from? ")
  local from = curses.getch() - 96
  curses.refresh()
  window:mvaddstr(lines(window)-1, 5, "tower to stack it on? ")
  local to = curses.getch() - 96
  curses.refresh()
  make_move(from, to)
end


local function main()
  local window = curses.stdscr()

  while true do
    render(window)
    update(window)
  end
end


main()
