--
-- variant of https://github.com/rxi/json.lua decoding from channels of
-- characters rather than strings
--
-- Copyright (c) 2020 rxi
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy of
-- this software and associated documentation files (the "Software"), to deal in
-- the Software without restriction, including without limitation the rights to
-- use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
-- of the Software, and to permit persons to whom the Software is furnished to do
-- so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in all
-- copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
-- SOFTWARE.
--

local jsonf = { _version = "0.1.2" }

local escape_char_map = {
  [ "\\" ] = "\\",
  [ "\"" ] = "\"",
  [ "\b" ] = "b",
  [ "\f" ] = "f",
  [ "\n" ] = "n",
  [ "\r" ] = "r",
  [ "\t" ] = "t",
}

local escape_char_map_inv = { [ "/" ] = "/" }
for k, v in pairs(escape_char_map) do
  escape_char_map_inv[v] = k
end


-------------------------------------------------------------------------------
-- Decode
-------------------------------------------------------------------------------

local function create_set(...)
  local res = {}
  for i = 1, select("#", ...) do
    res[ select(i, ...) ] = true
  end
  return res
end

local space_chars   = create_set(" ", "\t", "\r", "\n")
local delim_chars   = create_set(" ", "\t", "\r", "\n", "]", "}", ",")
local escape_chars  = create_set("\\", "/", '"', "b", "f", "n", "r", "t", "u")
local literals      = create_set("true", "false", "null")

local literal_map = {
  [ "true"  ] = true,
  [ "false" ] = false,
  [ "null"  ] = nil,
}


local function skip_spaces(infile)
  while true do
    local c = infile:recv()
    if c == nil then break end
    if space_chars[c] == nil then return c end
  end
  return nil
end


local function next_chars(infile, set, firstc)
  local res = {firstc}
  local nextc
  while true do
    nextc = infile:recv()
    if nextc == nil then break end
    if set[nextc] then break end
    table.insert(res, nextc)
  end
  return table.concat(res), nextc
end


local function codepoint_to_utf8(n)
  -- http://scripts.sil.org/cms/scripts/page.php?site_id=nrsi&id=iws-appendixa
  local f = math.floor
  if n <= 0x7f then
    return string.char(n)
  elseif n <= 0x7ff then
    return string.char(f(n / 64) + 192, n % 64 + 128)
  elseif n <= 0xffff then
    return string.char(f(n / 4096) + 224, f(n % 4096 / 64) + 128, n % 64 + 128)
  elseif n <= 0x10ffff then
    return string.char(f(n / 262144) + 240, f(n % 262144 / 4096) + 128,
                       f(n % 4096 / 64) + 128, n % 64 + 128)
  end
  error( string.format("invalid unicode codepoint '%x'", n) )
end


local function parse_unicode_escape(s)
  local n1 = tonumber( s:sub(1, 4),  16 )
  local n2 = tonumber( s:sub(7, 10), 16 )
   -- Surrogate pair?
  if n2 then
    return codepoint_to_utf8((n1 - 0xd800) * 0x400 + (n2 - 0xdc00) + 0x10000)
  else
    return codepoint_to_utf8(n1)
  end
end


local function parse_string(infile, firstc)
  local res = {}

  while true do
    local chr = infile:recv()
    if chr == nil then break end
    local x = chr:byte()

    if x < 32 then
      error("control character in string")
    elseif c == '\\' then
      local c = infile:recv()
      if c == nil then break end
      if c == "u" then
        local hex = ''
        c = infile:recv()
        if c == nil then break end
        hex = hex..c
        c = infile:recv()
        if c == nil then break end
        hex = hex..c
        c = infile:recv()
        if c == nil then break end
        hex = hex..c
        c = infile:recv()
        if c == nil then break end
        hex = hex..c
        if not hex:match('^%x%x%x%x') then
          error('invalid unicode escape in string')
        end
        table.insert(res, parse_unicode_escape(hex))
      else
        if not escape_chars[c] then
          error("invalid escape char '" .. c .. "' in string")
        end
        table.insert(escape_char_map_inv[c])
      end
    elseif chr == '"' then
      return table.concat(res), infile:recv()
    else
      table.insert(res, chr)
    end
  end

  error("expected closing quote for string")
end


local function parse_number(infile, firstc)
--?   print('parse_number')
  local res = {firstc}
  local nextc
  while true do
    nextc = infile:recv()
    if nextc == nil then break end
    if delim_chars[nextc] then break end
    table.insert(res, nextc)
  end
  local s = table.concat(res)
--?   print('parse_number: '..s)
  local n = tonumber(s)
  if not n then
    error("invalid number '" .. s .. "'")
  end
  return n, nextc
end


local function parse_literal(infile, firstc)
--?   print('parse_literal')
  local word, nextc = next_chars(infile, delim_chars, firstc)
  if not literals[word] then
    error("invalid literal '" .. word .. "'")
  end
--?   print('parse_literal: '..word)
  return literal_map[word], nextc
end


local function parse_array(infile, firstc)
  local res = {}
  local x, nextc
  while true do
    nextc = skip_spaces(infile)
    if nextc == nil then
      error("expected ']' or ','")
    end
    if nextc == ']' then break end  -- empty array
    -- Read token
    x, nextc = parse(infile, nextc)
--?     print('array elem: '..str(x))
    table.insert(res, x)
    -- Next token
    if space_chars[nextc] then
      nextc = skip_spaces(infile)
    end
    if nextc == ']' then break end
    if nextc ~= ',' then
      error("expected ']' or ','")
    end
  end
  return res, skip_spaces(infile)
end


local function parse_object(infile, firstc)
  local res = {}
  local nextc
  while true do
    local key, val
    nextc = skip_spaces(infile)
    if nextc == nil then
      error("expected '}' or ','")
    end
    if nextc == '}' then break end  -- empty object
    -- Read key
    if nextc ~= '"' then
      error("expected string for key")
    end
    key, nextc = parse(infile, nextc)
--?     print('object key: '..key)
    -- Read ':' delimiter
    if space_chars[nextc] then
      nextc = skip_spaces(infile)
    end
    if nextc ~= ':' then
      error("expected ':' after key")
    end
    -- Read value
    nextc = skip_spaces(infile)
    val, nextc = parse(infile, nextc)
--?     print('object val: '..str(val))
    -- Set
    res[key] = val
    -- Next token
    if space_chars[nextc] then
      nextc = skip_spaces(infile)
    end
    if nextc == '}' then break end
    if nextc ~= ',' then
      error("expected '}' or ','")
    end
  end
  return res, skip_spaces(infile)
end


local char_func_map = {
  [ '"' ] = parse_string,
  [ "0" ] = parse_number,
  [ "1" ] = parse_number,
  [ "2" ] = parse_number,
  [ "3" ] = parse_number,
  [ "4" ] = parse_number,
  [ "5" ] = parse_number,
  [ "6" ] = parse_number,
  [ "7" ] = parse_number,
  [ "8" ] = parse_number,
  [ "9" ] = parse_number,
  [ "-" ] = parse_number,
  [ "t" ] = parse_literal,
  [ "f" ] = parse_literal,
  [ "n" ] = parse_literal,
  [ "[" ] = parse_array,
  [ "{" ] = parse_object,
}


parse = function(infile, chr)
  local f = char_func_map[chr]
  if f then
    return f(infile, chr)
  end
  error("unexpected character '" .. chr .. "'")
end


function jsonf.decode(infile)
  return decode2(character_by_character(infile))
end


function decode2(infile)
  if not ischannel(infile) then
    error("expected channel, got " .. type(f))
  end
  local firstc = skip_spaces(infile)
  local res, nextc = parse(infile, firstc)
  if nextc then
    error("trailing garbage")
  end
  return res
end


-- test cases:
--   "abc"
--   234
--   true
--   false
--   nil
--   ["abc", 234, true, false, nil]
--   ["abc", 234, true, false, nil
--   ["abc",
--   {"abc": 234, "def": true}

return jsonf
