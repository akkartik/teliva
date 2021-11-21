teliva_program = {
  main = [==[
function main()
  local server = assert(socket.bind("*", 8080))
  server:settimeout(1)
  curses.mvaddstr(1, 1, "Server bound and waiting for one request")
  curses.refresh()
  local available_sockets, _, error = socket.select({server}, nil)
  for _, available_socket in ipairs(available_sockets) do
    local client = available_socket:accept()
    curses.mvaddstr(2, 1, "Connection received")
    curses.refresh()
    client:settimeout(1)
    local line, error = client:receive()
    if error then
        curses.mvaddstr(3, 1, "error")
        curses.refresh()
        server:close()
    else
        curses.stdscr():mvaddstr(3, 1, "received:")
        curses.stdscr():mvaddstr(4, 3, line)
        curses.refresh()
    end
  end
  curses.getch()
end]==],
}
