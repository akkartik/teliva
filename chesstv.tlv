teliva_program = {
  window = [==[
window = curses.stdscr()
-- animation-based app
window:nodelay(true)
lines, cols = window:getmaxyx()]==],
  current_game = [==[current_game = {}]==],
  piece_glyph = [==[
piece_glyph = {
  -- for legibility, white pieces also use unicode glyphs for black pieces
  -- we rely on colors to distinguish them
  K = 0x265a,
  Q = 0x265b,
  R = 0x265c,
  B = 0x265d,
  N = 0x265e,
  P = 0x265f,
  k = 0x265a,
  q = 0x265b,
  r = 0x265c,
  b = 0x265d,
  n = 0x265e,
  p = 0x265f,
}]==],
  top_player = [==[
function top_player(current_game)
  if current_game.players[1].color == "black" then
    return current_game.players[1]
  end
  return current_game.players[2]
end]==],
  bottom_player = [==[
function bottom_player(current_game)
  if current_game.players[1].color == "white" then
    return current_game.players[1]
  end
  return current_game.players[2]
end]==],
  render_player = [==[
function render_player(y, x, player)
  curses.mvaddstr(y, x, player.user.name)
  curses.addstr(" · ")
  curses.addstr(tostring(player.rating))
end]==],
  render_square = [==[
function render_square(current_game, rank, file, highlighted_squares)
  -- decide whether to highlight
  local hl = 0
  if (rank == highlighted_squares.from.rank and file == highlighted_squares.from.file)
      or (rank == highlighted_squares.to.rank and file == highlighted_squares.to.file) then
    hl = 4
  end
  if (rank+file)%2 == 1 then
    -- light square
    curses.attrset(curses.color_pair(1+hl))
  else
    -- dark square
    curses.attrset(curses.color_pair(3+hl))
  end
  curses.mvaddstr((8 - rank + 1)*3,   file*5, "     ")
  curses.mvaddstr((8 - rank + 1)*3+1, file*5, "     ")
  curses.mvaddstr((8 - rank + 1)*3+2, file*5, "     ")
  curses.attrset(curses.A_NORMAL)
end]==],
  render_fen_rank = [==[
function render_fen_rank(rank, fen_rank, highlighted_squares)
  local file = 1
  for x in fen_rank:gmatch(".") do
    if x:match("%d") then
      file = file + tonumber(x)
    else
      -- decide whether to highlight
      local hl = 0
      if (rank == highlighted_squares.from.rank and file == highlighted_squares.from.file)
          or (rank == highlighted_squares.to.rank and file == highlighted_squares.to.file) then
        hl = 4
      end
      if (rank+file)%2 == 1 then
        if x < 'Z' then
          -- white piece on light square
          curses.attrset(curses.color_pair(1+hl))
        else
          -- black piece on light square
          curses.attrset(curses.color_pair(2+hl))
        end
      else
        if x < 'Z' then
          -- white piece on dark square
          curses.attrset(curses.color_pair(3+hl))
        else
          -- black piece on dark square
          curses.attrset(curses.color_pair(4+hl))
        end
      end
      curses.mvaddstr((8 - rank + 1)*3+1, file*5+2, utf8(piece_glyph[x]))
      curses.attrset(curses.A_NORMAL)
      file = file + 1
    end
  end
end]==],
  render_time = [==[
function render_time(y, x, seconds)
  if seconds == nil then return end
  curses.mvaddstr(y, x, tostring(math.floor(seconds/60)))
  curses.addstr(string.format(":%02d", seconds%60))
end]==],
  render_board = [==[
function render_board(current_game)
--?   curses.mvaddstr(1, 50, dump(current_game.fen))
--?   curses.mvaddstr(6, 50, dump(current_game.previously_moved_squares))
  render_player(2, 5, top_player(current_game))
  render_time(2, 35, current_game.bc)
  for rank=8,1,-1 do
    for file=1,8 do
      render_square(current_game, rank, file, current_game.previously_moved_squares)
    end
    render_fen_rank(rank, current_game.fen_rank[8-rank+1], current_game.previously_moved_squares)
  end
  render_player(27, 5, bottom_player(current_game))
  render_time(27, 35, current_game.wc)
end]==],
  parse_lm = [==[
function parse_lm(move)
--?   curses.mvaddstr(4, 50, move)
  local file1 = string.byte(move:sub(1, 1)) - 96  -- 'a'-1
  local rank1 = string.byte(move:sub(2, 2)) - 48  -- '0'
  local file2 = string.byte(move:sub(3, 3)) - 96  -- 'a'-1
  local rank2 = string.byte(move:sub(4, 4)) - 48  -- '0'
--?   curses.mvaddstr(5, 50, dump({{rank1, file1}, {rank2, file2}}))
  return {from={rank=rank1, file=file1}, to={rank=rank2, file=file2}}
end]==],
  render = [==[
function render(chunk)
  local o = json.decode(chunk)
  if o.t == "featured" then
    current_game = o.d
--?     current_game.lm = "__"
    current_game.previously_moved_squares = {from={rank=0, file=0}, to={rank=0, file=0}}  -- no highlight
  else
    current_game.fen = o.d.fen
    current_game.wc = o.d.wc
    current_game.bc = o.d.bc
--?     current_game.lm = o.d.lm
    current_game.previously_moved_squares = parse_lm(o.d.lm)
--?     window:nodelay(false)
--?     curses.mvaddstr(3, 50, "paused")
  end
  current_game.fen_rank = split(current_game.fen, "%w+")
  render_board(current_game)
  curses.refresh()
end]==],
  init_colors = [==[
function init_colors()
  -- white piece on light square (assume light background)
  curses.init_pair(1, 1, -1)
  -- black piece on light square
  curses.init_pair(2, 0, -1)
  -- white piece on dark square
  curses.init_pair(3, 1, 3)
  -- black piece on dark square
  curses.init_pair(4, 0, 3)
  -- white piece on last-moved light square
  curses.init_pair(5, 1, 10)
  -- black piece on last-moved light square
  curses.init_pair(6, 0, 10)
  -- white piece on last-moved dark square
  curses.init_pair(7, 1, 2)
  -- black piece on last-moved dark square
  curses.init_pair(8, 0, 2)
end]==],
  main = [==[
function main()
  init_colors()
  local request = {
    url = "https://lichess.org/api/tv/feed",
    sink = function(chunk, err)
             if chunk then
               curses.clear()
               render(chunk)
               curses.getch()
             end
             return 1
           end,
  }
  http.request(request)
end]==],
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
  split = [==[
function split(s, pat)
  result = {}
  for x in s:gmatch(pat) do
    table.insert(result, x)
  end
  return result
end]==],
  dump = [==[
-- https://stackoverflow.com/questions/9168058/how-to-dump-a-table-to-console
function dump(o)
  if type(o) == 'table' then
    local s = '{ '
    for k,v in pairs(o) do
      if type(k) ~= 'number' then k = '"'..k..'"' end
      s = s .. '['..k..'] = ' .. dump(v) .. ','
    end
    return s .. '} '
  else
    return tostring(o)
  end
end]==],
}
