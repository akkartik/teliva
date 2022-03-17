-- primitive for reading files from a file system (or, later, network)
-- returns an object or nil on error
-- read lines from the object using .read() with args similar to file:read()
-- read() will indicate end of file by returning nil.
function start_reading(fs, filename)
  local infile = io.open(filename)
  if infile == nil then return nil end
  return {
    read = coroutine.wrap(function(format)
      while true do
        if format == nil then format = '*l' end
        format = coroutine.yield(infile:read(format))
      end
    end),
  }
end

-- primitive for writing files to a file system (or, later, network)
-- returns an object or nil on error
-- write to the object using .write()
-- indicate you're done writing by calling .close()
-- file will not be externally visible until .close()
function start_writing(fs, filename)
  if filename == nil then
    error('start_writing requires two arguments: a file-system (nil for real disk) and a filename')
  end
  local initial_filename = temporary_filename_in_same_volume(filename)
  local outfile = io.open(initial_filename, 'w')
  if outfile == nil then return nil end
  return {
    write = coroutine.wrap(function(x)
      while true do
        x = coroutine.yield(outfile:write(x))
      end
    end),
    close = function()
      outfile:close()
      os.rename(initial_filename, filename)
    end,
  }
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
