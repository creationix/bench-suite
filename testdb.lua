local net = require('net')
local Emitter = require('core').Emitter
local JSON = require('json')
local setInterval = require('timer').setInterval

local Queue = {};
function Queue.new()
  return {first = 0, last = -1}
end
function Queue.push (queue, value)
  local last = queue.last + 1
  queue.last = last
  queue[last] = value
end
function Queue.shift (queue)
  local first = queue.first
  if first > queue.last then error("queue is empty") end
  local value = queue[first]
  queue[first] = nil
  queue.first = first + 1
  return value
end


local function connect(port, callback)
  local callbacks = Queue.new()
  local db = Emitter:new()
  local socket
  function db.query(table, key, callback)
    socket:write(table .. "/" .. key .. "\n")
    Queue.push(callbacks, callback)
  end
  function db.close()
    socket:close()
  end
  socket = net.createConnection(port, function ()
    callback(nil, db)
  end)

  -- parse responses
  socket:on("data", function (chunk)
    -- TODO: Don't assume packets always contain exactly one response.
    Queue.shift(callbacks)(null, JSON.parse(chunk:sub(1, #chunk - 1)));
  end)
  
  socket:on("close", function ()
    error("Connection Closed")
  end)

  socket:on("error", function (err)
    db.close();
    error(err)
  end)
  
end

local done = 0
local function client()
  connect(5555, function (err, db)
    if err then error(err) end
    local function next()
      db.query("sessions", "eo299pqyw9791jie7yp", function (err, session)
        if err then error(err) end
--        p({session=session})
        db.query("users", session.username, function (err, user)
          if err then error(err) end
--          p({user=user})
          done = done + 1
          next()
        end)
      end)
    end
    next()
  end)
end

local hrtime = require('uv_native').hrtime

local before = hrtime()
setInterval(1000, function ()
  local now = hrtime();
  local delta = (now - before) / 10;
  before = now;
  local speed = done * 1000 / delta;
  print(done .. " cycles in " .. delta .. "ms (" .. speed .. "/second)");
  done = 0;
end)

client()
client()

