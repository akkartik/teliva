teliva_program = {
  window = [==[
window = curses.stdscr()
-- animation-based app
window:nodelay(true)
lines, cols = window:getmaxyx()]==],
  render = [==[
function render(chunk)
  curses.mvaddstr(5, 5, chunk)
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
  curses.getch()
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
