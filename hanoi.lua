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


local function render_tower(screen, line, col, tower_index, tower)
  screen:attron(curses.A_BOLD)
  screen:mvaddch(line+2, col, string.char(96+tower_index))
  screen:attroff(curses.A_BOLD)
  screen:mvaddstr(line+1, col-3, "-------")
  for i, n in ipairs(tower) do
    screen:mvaddstr(line, col, n)
    line = line - 1
  end
  for i=1,5-len(tower) do
    screen:mvaddstr(line, col, "|")
    line = line - 1
  end
end


local function render(screen)
  screen:clear()
  local lines, cols = screen:getmaxyx()
  local line = math.floor(lines/2)
  local col = math.floor(cols/4)
  for i,t in ipairs(tower) do
    render_tower(screen, line, i*col, i, t)
  end
end


local function make_move(from, to)
  local disk = pop(tower[from])
  table.insert(tower[to], disk)
end


local function update(screen)
  screen:mvaddstr(lines(screen)-2, 5, "tower to remove top disk from? ")
  local from = string.byte(curses.getch()) - 96
  curses.refresh()
  screen:mvaddstr(lines(screen)-1, 5, "tower to stack it on? ")
  local to = string.byte(curses.getch()) - 96
  curses.refresh()
  make_move(from, to)
end


local function main()
  local screen = curses.initscr()

  while true do
    render(screen)
    update(screen)
  end

  curses.endwin()
end


main()
