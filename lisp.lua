-- atom types:
--   nil
--   true
--   {num=3.4}
--   {char='a'}
--   {str='bc'}
--   {sym='foo'}
-- non-atom type:
--   {car={num=3.4}, cdr=nil}
--
-- should {} mean anything special? currently just '(nil)
function atom(x)
  return x == nil or x.num or x.char or x.str or x.sym
end

function car(x) return x.car end
function cdr(x) return x.cdr end
function cons(x, y) return {car=x, cdr=y} end

function iso(x, y)
  if x == nil then return y == nil end
  local done={}
  if done[x] then return done[x] == y end
  done[x] = y
  if atom(x) then
    if not atom(y) then return nil end
    for k, v in pairs(x) do
      if y[k] ~= v then return nil end
    end
    return true
  end
  for k, v in pairs(x) do
    if not iso(y[k], v) then return nil end
  end
  for k, v in pairs(y) do
    if not iso(x[k], v) then return nil end
  end
  return true
end

-- primitives; feel free to add more
-- format: lisp name = lua function that implements it
unary_functions = {
  atom=atom,
  car=car,
  cdr=cdr,
}

binary_functions = {
  cons=cons,
  iso=iso,
}

function lookup(env, s)
  if env[s] then return env[s] end
  if env.next then return lookup(env.next, s) end
end

function eval(x, env)
  function symeq(x, s)
    return x and x.sym == s
  end
  if x.sym then
    return lookup(env, x.sym)
  elseif atom(x) then
    return x
  -- otherwise x is a pair
  elseif symeq(x.car, 'quote') then
    return x.cdr
  elseif unary_functions[x.car.sym] then
    return eval_unary(x, env)
  elseif binary_functions[x.car.sym] then
    return eval_binary(x, env)
  -- special forms that don't always eval all their args
  elseif symeq(x.car, 'if') then
    return eval_if(x, env)
  elseif symeq(x.car.car, 'fn') then
    return eval_fn(x, env)
  elseif symeq(x.car.car, 'label') then
    return eval_label(x, env)
  end
end

function eval_unary(x, env)
  return unary_functions[x.car.sym](eval(x.cdr.car, env))
end

function eval_binary(x, env)
  return binary_functions[x.car.sym](eval(x.cdr.car, env),
                                     eval(x.cdr.cdr.car, env))
end

function eval_if(x, env)
  -- syntax: (if check b1 b2)
  local check = x.cdr.car
  local b1    = x.cdr.cdr.car
  local b2    = x.cdr.cdr.cdr.car
  if eval(check, env) then
    return eval(b1, env)
  else
    return eval(b2, env)
  end
end

function eval_fn(x, env)
  -- syntax: ((fn params body*) args*)
  local callee = x.car
  local args = x.cdr
  local params = callee.cdr.car
  local body = callee.cdr.cdr
  return eval_exprs(body,
                    bind_env(params, args, env))
end

function bind_env(params, args, env)
  if params == nil then return env end
  local result = {next=env}
  while true do
    result[params.car.sym] = eval(args.car, env)
    params = params.cdr
    args = args.cdr
    if params == nil then break end
  end
  return result
end

function eval_exprs(xs, env)
  local result = nil
  while xs do
    result = eval(xs.car, env)
    xs = xs.cdr
  end
  return result
end

function eval_label(x, env)
  -- syntax: ((label f (fn params body*)) args*)
  local callee = x.car
  local args = x.cdr
  local f = callee.cdr.car
  local fn = callee.cdr.cdr.car
  return eval({car=fn, cdr=args},
              bind_env({f}, {callee}, env))
end

-- testing
function num(n) return {num=n} end
function char(c) return {char=c} end
function str(s) return {str=s} end
function sym(s) return {sym=s} end
function list(...)
  -- gotcha: no element in arg can be nil; that short-circuits the ipairs below
  local result = nil
  local curr = nil
  for _, x in ipairs({...}) do
    if curr == nil then
      result = {car=x}
      curr = result
    else
      curr.cdr = {car=x}
      curr = curr.cdr
    end
  end
  return result
end

function p(x)
  p2(x)
  print()
end

function p2(x)
  if x == nil then
    io.write('nil')
  elseif x == true then
    io.write('true')
  elseif x.num then
    io.write(x.num)
  elseif x.char then
    io.write("\\"..x.char)
  elseif x.str then
    io.write('"'..x.str..'"')
  elseif x.sym then
    io.write(x.sym)
  elseif x.cdr == nil then
    io.write('(')
    p2(x.car)
    io.write(')')
  elseif atom(x.cdr) then
    io.write('(')
    p2(x.car)
    io.write(' . ')
    p2(x.cdr)
    io.write(')')
  else
    io.write('(')
    while true do
      p2(x.car)
      x = x.cdr
      if x == nil then break end
      if atom(x) then
        io.write(' . ')
        p2(x)
        break
      end
      io.write(' ')
    end
    io.write(')')
  end
end

x = {num=3.4}
p(x)

p(cons(x, nil))
p(list(x))

p(iso(cons(x, nil), cons(x, nil)))
p(iso(list(x), list(x)))
p(iso(list(x, x), list(x)))
p(iso(list(x, x), list(x, x)))
p(iso(x, cons(x, nil)))

p     (list(sym("cons"), num(42), num(1)))
p(eval(list(sym("cons"), num(42), num(1)), {}))

-- ((fn () 42)) => 42
-- can't use list here because of the gotcha above
assert(iso(eval(cons(cons(sym('fn'), cons(nil, cons(num(42))))), {}), num(42)))
-- ((fn (a) (cons a 1)) 42) => '(42 . 1)
assert(iso(eval(cons(cons(sym('fn'), cons(cons(sym('a')), cons(cons(sym('cons'), cons(sym('a'), cons(num(1))))))), cons(num(42)))), cons(num(42), num(1))))
