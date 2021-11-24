teliva_program = {
    window = [==[
window = curses.stdscr()
-- animation-based app
window:nodelay(true)
lines, cols = window:getmaxyx()]==],
    grid = [==[
-- main data structure
grid = {}
for i=1,lines*4 do
  grid[i] = {}
  for j=1,cols*2 do
    grid[i][j] = 0
  end
end
]==],
    grid_char = [==[
-- grab a 4x2 chunk of grid
function grid_char(line, col)
  result = {}
  for l, row in ipairs({unpack(grid, (line-1)*4+1, line*4)}) do
    result[l] = {unpack(row, (col-1)*2+1, col*2)}
  end
  return result
end]==],
    print_grid_char = [==[
function print_grid_char(window, x)
  result = {}
  for l, row in ipairs(x) do
    for c, val in ipairs(row) do
      window:mvaddstr(l, c, val)
    end
  end
  return result
end]==],
    glyph = [==[
-- look up the braille pattern corresponding to a 4x2 chunk of grid
-- https://en.wikipedia.org/wiki/Braille_Patterns
-- not obviously programmatic because Unicode added 4x2 after 3x2
glyph = {
  0x2800, 0x2801, 0x2802, 0x2803, 0x2804, 0x2805, 0x2806, 0x2807,   0x2840, 0x2841, 0x2842, 0x2843, 0x2844, 0x2845, 0x2846, 0x2847,
  0x2808, 0x2809, 0x280a, 0x280b, 0x280c, 0x280d, 0x280e, 0x280f,   0x2848, 0x2849, 0x284a, 0x284b, 0x284c, 0x284d, 0x284e, 0x284f,
  0x2810, 0x2811, 0x2812, 0x2813, 0x2814, 0x2815, 0x2816, 0x2817,   0x2850, 0x2851, 0x2852, 0x2853, 0x2854, 0x2855, 0x2856, 0x2857,
  0x2818, 0x2819, 0x281a, 0x281b, 0x281c, 0x281d, 0x281e, 0x281f,   0x2858, 0x2859, 0x285a, 0x285b, 0x285c, 0x285d, 0x285e, 0x285f,
  0x2820, 0x2821, 0x2822, 0x2823, 0x2824, 0x2825, 0x2826, 0x2827,   0x2860, 0x2861, 0x2862, 0x2863, 0x2864, 0x2865, 0x2866, 0x2867,
  0x2828, 0x2829, 0x282a, 0x282b, 0x282c, 0x282d, 0x282e, 0x282f,   0x2868, 0x2869, 0x286a, 0x286b, 0x286c, 0x286d, 0x286e, 0x286f,
  0x2830, 0x2831, 0x2832, 0x2833, 0x2834, 0x2835, 0x2836, 0x2837,   0x2870, 0x2871, 0x2872, 0x2873, 0x2874, 0x2875, 0x2876, 0x2877,
  0x2838, 0x2839, 0x283a, 0x283b, 0x283c, 0x283d, 0x283e, 0x283f,   0x2878, 0x2879, 0x287a, 0x287b, 0x287c, 0x287d, 0x287e, 0x287f,

  0x2880, 0x2881, 0x2882, 0x2883, 0x2884, 0x2885, 0x2886, 0x2887,   0x28c0, 0x28c1, 0x28c2, 0x28c3, 0x28c4, 0x28c5, 0x28c6, 0x28c7,
  0x2888, 0x2889, 0x288a, 0x288b, 0x288c, 0x288d, 0x288e, 0x288f,   0x28c8, 0x28c9, 0x28ca, 0x28cb, 0x28cc, 0x28cd, 0x28ce, 0x28cf,
  0x2890, 0x2891, 0x2892, 0x2893, 0x2894, 0x2895, 0x2896, 0x2897,   0x28d0, 0x28d1, 0x28d2, 0x28d3, 0x28d4, 0x28d5, 0x28d6, 0x28d7,
  0x2898, 0x2899, 0x289a, 0x289b, 0x289c, 0x289d, 0x289e, 0x289f,   0x28d8, 0x28d9, 0x28da, 0x28db, 0x28dc, 0x28dd, 0x28de, 0x28df,
  0x28a0, 0x28a1, 0x28a2, 0x28a3, 0x28a4, 0x28a5, 0x28a6, 0x28a7,   0x28e0, 0x28e1, 0x28e2, 0x28e3, 0x28e4, 0x28e5, 0x28e6, 0x28e7,
  0x28a8, 0x28a9, 0x28aa, 0x28ab, 0x28ac, 0x28ad, 0x28ae, 0x28af,   0x28e8, 0x28e9, 0x28ea, 0x28eb, 0x28ec, 0x28ed, 0x28ee, 0x28ef,
  0x28b0, 0x28b1, 0x28b2, 0x28b3, 0x28b4, 0x28b5, 0x28b6, 0x28b7,   0x28f0, 0x28f1, 0x28f2, 0x28f3, 0x28f4, 0x28f5, 0x28f6, 0x28f7,
  0x28b8, 0x28b9, 0x28ba, 0x28bb, 0x28bc, 0x28bd, 0x28be, 0x28bf,   0x28f8, 0x28f9, 0x28fa, 0x28fb, 0x28fc, 0x28fd, 0x28fe, 0x28ff,
}]==],
    utf8 = [==[
-- https://stackoverflow.com/questions/7983574/how-to-write-a-unicode-symbol-in-lua
function utf8(decimal)
  local bytemarkers = { {0x7FF,192}, {0xFFFF,224}, {0x1FFFFF,240} }
  if decimal<128 then return string.char(decimal) end
  local charbytes = {}
  for bytes,vals in ipairs(bytemarkers) do
    if decimal<=vals[1] then
      for b=bytes+1,2,-1 do
        local mod = decimal%64
        decimal = (decimal-mod)/64
        charbytes[b] = string.char(128+mod)
      end
      charbytes[1] = string.char(vals[2]+decimal)
      break
    end
  end
  return table.concat(charbytes)
end]==],
    grid_char_to_glyph_index = [==[
-- convert a chunk of grid into a number
function grid_char_to_glyph_index(g)
  return g[1][1]    + g[2][1]*2  + g[3][1]*4  + g[4][1]*8 +
         g[1][2]*16 + g[2][2]*32 + g[3][2]*64 + g[4][2]*128 +
         1  -- 1-indexing
end]==],
    render = [==[
function render(window)
  window:clear()
  for line=1,lines do
    for col=1,cols do
      window:addstr(utf8(glyph[grid_char_to_glyph_index(grid_char(line, col))]))
    end
  end
  curses.refresh()
end
]==],
    state = [==[
function state(line, col)
  if line < 1 or line > table.getn(grid) or col < 1 or col > table.getn(grid[1]) then
    return 0
  end
  return grid[line][col]
end]==],
    num_live_neighbors = [==[
function num_live_neighbors(line, col)
  return state(line-1, col-1) + state(line-1, col) + state(line-1, col+1) +
         state(line,   col-1) +                      state(line,   col+1) +
         state(line+1, col-1) + state(line+1, col) + state(line+1, col+1)
end]==],
    step = [==[
function step()
  local new_grid = {}
  for line=1,table.getn(grid) do
    new_grid[line] = {}
    for col=1,table.getn(grid[1]) do
      local n = num_live_neighbors(line, col)
      if n == 3 then
        new_grid[line][col] = 1
      elseif n == 2 then
        new_grid[line][col] = grid[line][col]
      else
        new_grid[line][col] = 0
      end
    end
  end
  grid = new_grid
end]==],
    sleep = [==[
function sleep(a)
    local sec = tonumber(os.clock() + a);
    while (os.clock() < sec) do
    end
end]==],
    file_exists = [==[
function file_exists(filename)
  local f = io.open(filename, "r")
  if f ~= nil then
    io.close(f)
    return true
  else
    return false
  end
end]==],
    load_file = [==[
function load_file(window, filename)
  io.input(filename)
  local line_index = lines
  for line in io.lines() do
    if line:sub(1,1) ~= '!' then  -- comment; plaintext files can't have whitespace before comments
      local col_index = cols
      for c in line:gmatch(".") do
        if c == '\r' then break end  -- DOS line ending
        if c == '.' then
          grid[line_index][col_index] = 0
        else
          grid[line_index][col_index] = 1
        end
        col_index = col_index+1
      end
      line_index = line_index+1
    end
  end
end]==],
    update = [==[
menu = {arrow="pan"}

function update(window, c)
  if c == curses.KEY_LEFT then
    for i=1,lines*4 do
      for j=2,cols*2 do
        grid[i][j-1] = grid[i][j]
      end
      grid[i][cols*2] = 0
    end
  elseif c == curses.KEY_DOWN then
    for i=lines*4-1,1,-1 do
      for j=1,cols*2 do
        grid[i+1][j] = grid[i][j]
      end
    end
    for j=1,cols*2 do
      grid[1][j] = 0
    end
  elseif c == curses.KEY_UP then
    for i=2,lines*4 do
      for j=1,cols*2 do
        grid[i-1][j] = grid[i][j]
      end
    end
    for j=1,cols*2 do
      grid[lines*4][j] = 0
    end
  elseif c == curses.KEY_RIGHT then
    for i=1,lines*4 do
      for j=cols*2-1,1,-1 do
        grid[i][j+1] = grid[i][j]
      end
      grid[i][1] = 0
    end
  end
end]==],
    main = [==[
function main()
  for i=1,7 do
    curses.init_pair(i, i, -1)
  end

  -- initialize grid based on commandline args
  if (#arg == 0) then
    -- by default, start from a deterministically random state
    for i=1,lines*4 do
      for j=1,cols*2 do
        grid[i][j] = math.random(0, 1)
      end
    end
  elseif arg[1] == "random" then
    -- start from a non-deterministically random start state
    math.randomseed(os.time())
    for i=1,lines*4 do
      for j=1,cols*2 do
        grid[i][j] = math.random(0, 1)
      end
    end
  -- shortcuts for some common patterns
  elseif arg[1] == "pentomino" then
    -- https://www.conwaylife.com/wiki/Pentomino
    grid[83][172] = 1
    grid[83][173] = 1
    grid[84][173] = 1
    grid[84][174] = 1
    grid[85][173] = 1
  elseif arg[1] == "glider" then
    -- https://www.conwaylife.com/wiki/Glider
    grid[5][4] = 1
    grid[6][5] = 1
    grid[7][3] = 1
    grid[7][4] = 1
    grid[7][5] = 1
  elseif arg[1] == "blinker" then
    -- https://www.conwaylife.com/wiki/Blinker
    grid[7][3] = 1
    grid[7][4] = 1
    grid[7][5] = 1
  elseif arg[1] == "block" then
    -- https://www.conwaylife.com/wiki/Block
    grid[5][4] = 1
    grid[5][5] = 1
    grid[6][4] = 1
    grid[6][5] = 1
  elseif arg[1] == "loaf" then
    -- https://www.conwaylife.com/wiki/Loaf
    grid[5][4] = 1
    grid[5][5] = 1
    grid[6][6] = 1
    grid[7][6] = 1
    grid[8][5] = 1
    grid[7][4] = 1
    grid[6][3] = 1
  elseif file_exists(arg[1]) then
    -- Load a file in the standard "plaintext" format: https://www.conwaylife.com/wiki/Plaintext
    --
    -- Each pattern page at https://www.conwaylife.com/wiki provides its
    -- plaintext representation in a block called "Pattern Files" on the right.
    --
    -- For example, check out the list of Important Patterns at
    -- https://www.conwaylife.com/wiki/Category:Patterns_with_Catagolue_frequency_class_0
    load_file(window, arg[1])
  end

  -- main loop
  while true do
    render(window)
    c = curses.getch()
    update(window, c)
    step()
  end
end]==],
}
