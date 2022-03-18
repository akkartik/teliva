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

-- fake file object with the same 'shape' as that returned by start_reading
function fake_file_stream(s)
  local i = 1
  local max = string.len(s)
  return {
    read = function(format)
      if i > max then
        return nil
      end
      if type(format) == 'number' then
        local result = s:sub(i, i+format-1)
        i = i+format
        return result
      elseif format == '*a' then
        local result = s:sub(i)
        i = max+1
        return result
      elseif format == '*l' then
        local start = i
        while i <= max do
          if s:sub(i, i) == '\n' then
            break
          end
          i = i+1
        end
        local result = s:sub(start, i)
        i = i+1
        return result
      elseif format == '*n' then
        error('fake file streams: *n not yet supported')
      end
    end,
  }
end

function test_fake_file_system()
  local s = fake_file_stream('abcdefgh\nijk\nlmn')
  check_eq(s.read(1), 'a', 'fake_file_system: 1 char')
  check_eq(s.read(1), 'b', 'fake_file_system: 1 more char')
  check_eq(s.read(3), 'cde', 'fake_file_system: multiple chars')
  check_eq(s.read('*l'), 'fgh\n', 'fake_file_system: line')
  check_eq(s.read('*a'), 'ijk\nlmn', 'fake_file_system: all')
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
