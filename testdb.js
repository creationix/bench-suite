var net = require('net');
var EventEmitter = require('events').EventEmitter;

function connect(port, callback) {
  var callbacks = [];
  var db = new EventEmitter();
  db.query = function (table, key, callback) {
    callbacks.push(callback);
    socket.write(table + "/" + key + "\0");
  };
  db.close = function () {
    socket.end();
  };
  var socket = net.connect(port, function() {
    callback(null, db);
  });

  // parse responses
  socket.on("data", function (chunk) {
    // TODO: Don't assume packets always contain exactly one response.
    callbacks.shift()(null, JSON.parse(chunk.toString('ascii', 0, chunk.length - 1)));
  });
  
  socket.on("close", function () {
    var err = new Error("Connection closed");
    callbacks.forEach(function (callback) {
      callback(err);
    });
    callbacks.length = 0;
  });

  socket.on("error", function (err) {
    callbacks.forEach(function (callback) {
      callback(err);
    });
    callbacks.length = 0;
    db.close();
  });
  
  function flush() {
    var json = parts.join("");
    parts.length = 0;
    var callback = callbacks.shift();
    try {
      var data = JSON.parse(json);
    } catch (err) {
      return callback(err);
    }
    callback(null, data);
  }
}

var done = 0;
var dbs = [];
function client() {
  connect(5555, function (err, db) {
    if (err) throw err;
    dbs.push(db);
    next();
    function next() {
      db.query("sessions", "eo299pqyw9791jie7yp", function (err, session) {
        if (err) throw err;
//        console.log({session: session});
        db.query("users", session.username, function (err, user) {
          if (err) throw err;
//          console.log({user: user});
          done++;
          next();
        });
      });
    }
  });
}

var before = Date.now();
setInterval(function () {
  var now = Date.now();
  var delta = now - before;
  before = now;
  var speed = done * 1000 / delta;
  console.log("%s cycles in %sms (%s/second)", done, delta, speed);
  done = 0;
}, 1000);

for (var i = 0; i < 2; i++) {
  client();
}

