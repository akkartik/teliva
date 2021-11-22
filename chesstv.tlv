teliva_program = {
  window = [==[
window = curses.stdscr()
-- animation-based app
window:nodelay(true)
lines, cols = window:getmaxyx()]==],
  current_game = [==[current_game = {}]==],
  piece_glyph = [==[
piece_glyph = {
  K = 0x2654,
  Q = 0x2655,
  R = 0x2656,
  B = 0x2657,
  N = 0x2658,
  P = 0x2659,
  k = 0x265a,
  q = 0x265b,
  r = 0x265c,
  b = 0x265d,
  n = 0x265e,
  p = 0x265f,
}]==],
  top_player = [==[
function top_player(current_game)
  if current_game.players[1].color ~= current_game.orientation then
    return current_game.players[1]
  end
  return current_game.players[2]
end]==],
  bottom_player = [==[
function bottom_player(current_game)
  if current_game.players[1].color == current_game.orientation then
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
function render_square(current_game, rank, file)
  if (rank+file)%2 == 1 then
    curses.attrset(curses.A_REVERSE)
  end
  curses.mvaddstr(rank*3,   file*5, "     ")
  curses.mvaddstr(rank*3+1, file*5, "     ")
  curses.mvaddstr(rank*3+2, file*5, "     ")
  curses.attrset(curses.A_NORMAL)
end]==],
  render_fen_rank = [==[
function render_fen_rank(rank, fen_rank)
  local file = 1
  for x in fen_rank:gmatch(".") do
    if x:match("%d") then
      file = file + tonumber(x)
    else  -- if x ~= nil then
      if (rank+file)%2 == 1 then
        curses.attrset(curses.A_REVERSE)
      end
      curses.mvaddstr(rank*3+1, file*5+2, utf8(piece_glyph[x]))
      curses.attrset(curses.A_NORMAL)
      file = file + 1
    end
  end
end]==],
  render_board = [==[
function render_board(current_game)
  render_player(2, 5, top_player(current_game))
  for rank=1,8 do
    for file=1,8 do
      render_square(current_game, rank, file)
    end
    render_fen_rank(rank, current_game.fen_rank[rank])
  end
  render_player(27, 5, bottom_player(current_game))
end]==],
  render = [==[
function render(chunk)
  local o = json.decode(chunk)
  if o.t == "featured" then
    current_game = o.d
  else
    current_game.fen = o.d.fen
    current_game.wc = o.d.wc
    current_game.bc = o.d.bc
    -- todo: o.d.lm to highlight the last move?
  end
  current_game.fen_rank = split(current_game.fen, "%w+")
  render_board(current_game)
  curses.refresh()
end]==],
  main = [==[
function main()
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
