teliva_program = {
  main = [==[
function main()
  local x = json.decode("[10, 20, 30]")
  curses.mvaddstr(5, 5, tostring(x[1]))
  curses.getch()
end]==],
}
