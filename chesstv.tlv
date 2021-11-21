teliva_program = {
  main = [==[
function main()
  local body = http.request("https://example.com")
  curses.mvaddstr(5, 5, body)
  curses.refresh()
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
