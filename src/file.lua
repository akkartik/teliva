-- primitive for reading files from a file system (or, later, network)
-- returns a channel or nil on error
-- read lines from the channel using :recv()
-- recv() on the channel will indicate end of file.
function start_reading(fs, filename)
  local result = task.Channel:new()
  local infile = io.open(filename)
  if infile == nil then return nil end
  task.spawn(reading_task, infile, result)
  return result
end

local function reading_task(infile, chanout)
  for line in infile:lines() do
    chanout:send(line)
  end
  chanout:send(nil)  -- eof
end

-- primitive for writing files to a file system (or, later, network)
-- returns a channel or nil on error
-- write to the channel using :send()
-- indicate you're done writing by calling :close()
-- file will not be externally visible until :close()
function start_writing(fs, filename)
  local result = task.Channel:new()
  local initial_filename = os.tmpname()
  local outfile = io.open(initial_filename, 'w')
  if outfile == nil then return nil end
  result.close = function()
    result:send(nil)  -- end of file
    outfile:close()
    os.rename(initial_filename, filename)
  end
  task.spawn(writing_task, outfile, result)
  return result
end

local function writing_task(outfile, chanin)
  while true do
    local line = chanin:recv()
    if line == nil then break end  -- end of file
    outfile:write(line)
  end
end
