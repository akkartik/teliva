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

function reading_task(infile, chanout)
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
  if filename == nil then
    error('start_writing requires two arguments: a file-system (nil for real disk) and a filename')
  end
  local result = task.Channel:new()
  local initial_filename = temporary_filename_in_same_volume(filename)
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

function temporary_filename_in_same_volume(filename)
  -- opening in same directory will hopefully keep it on the same volume,
  -- so that a future rename works
  local i = 1
  while true do
    temporary_filename = 'teliva_tmp_'..filename..'_'..i
    if io.open(temporary_filename) == nil then
      -- file doesn't exist yet; create a placeholder and return it
      local handle = io.open(temporary_filename, 'w')
      if handle == nil then
        error("this is unexpected; I can't create temporary files..")
      end
      handle:close()
      return temporary_filename
    end
    i = i+1
  end
end

function writing_task(outfile, chanin)
  while true do
    local line = chanin:recv()
    if line == nil then break end  -- end of file
    outfile:write(line)
  end
end

-- start_reading reads line by line by default
-- this helper permits character-by-character reading
function character_by_character(chanin, buffer_size)
  local chanout = task.Channel:new(buffer_size or 50)
  task.spawn(character_splitting_task, chanin, chanout)
  return chanout
end

function character_splitting_task(chanin, chanout)
  while true do
    local line = chanin:recv()
    if line == nil then break end
    local linesz = line:len()
    for i=1,linesz do
      chanout:send(line:sub(i, i))
    end
  end
  chanout:send(nil)  -- end of file
end
