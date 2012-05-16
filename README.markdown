# Benchmark Challenge

As I work on several platforms that are often used as web frameworks, the first example application is always an http server that serves the static string "Hello World" to all users.  Then these are benchmarked against each other to prove which platform is the best at being web scale.

Then there comes a flood of comments about how this benchmark is meaningless for any real-world work. So I hereby propose a new standard for http benchmarks.  Using this standard data, platforms can implement this challenge and see how well they perform.  It's still not a real-world program, but it's a but closer and involves actual I/O.

## The Challenge

To accept the challenge, fork this repo on github, add an implementation of the benchmark server in your framework.  Include clear instructions on how to run it and show what results you get when testing against the other implementations.  Send a pull request when you're ready to merge back in.

## The Database

Most real apps have some sort of persistent database.  Provided in this repo is a simple mock database. The source is in `db.c`.  Build is using the included Makefile.  Make sure to pull in the libuv submodule.

This is a small and fast event-based tcp server.  To query, simply connect and write your key to the socket as a newling-terminated string with table first, then row key.  For example, the key `orange` in the table `colors` would be queried with `"colors/orange\n"`.

It will then respond with the JSON representation of that entry as a newline terminated string.  This single-threaded server is event-based so you can have as many concurrent connections to it as you want.

Also it supports pipelining.  Meaning you can send a second query on the same socket connection before waiting for the response to the first.

Use as many instances as you want.  Since this server is single threaded, a fast client may be able to saturate the one CPU core this server uses.  The port is passed in as argv[1].

This mock database has two tables, `users` and `sessions`.  In session responses, the `username` field is the key in the `users` table.

## The Request

There is one page in this made-up system, to render it, the user data needs to be merged with some html.

```js
function myprofile(user) {
  var html = '<h2>' + user.name + '</h2>\n';
  html += '<ul class="links">\n';
  if (user.twitter) {
    html += '  <li><a href="https://twitter.com/' + user.twitter + '/">Twitter</a></li>\n';
  }
  if (user.github) {
    html += '  <li><a href="https://github.com/' + user.github + '/">Github</a></li>\n';
  }
  if (user.websites) {
    user.websites.foreach(function (website) {
      html += '  <li><a href="' + website + '/">' + website + '</a></li>\n';
    });
  }
  html += '</ul>\n';
  return html;
}
```

The http request should have a cookie called "SESSID" or something that contains the session key "eo299pqyw9791jie7yp".  In order to make this benchmark fair, no caching is allowed in the session or database lookups.  This is to simulate requests not being the same user and session every time.  The html template, however can be compiled and saved at startup since a site usually has a finite number of templates.

After getting the session ID out of the cookie, the server will need to look up the session data.  Then from that load the html template and merge it with the user's data from the database.  The template itself can pull from the db or you can pull from the db and pass the data to the template.

It's up to you to use a fixed Content-Length or chunked encoding in the response, but the following headers must be in the response.

```
Date: the current timestamp
Server: A simple string identifying your server, eg "node"
Content-Type: "text/html"
```


## The Client

To test these, you need to send in the proper headers.  In apache-bench this can be done with the -H flag to pass in the cookie header.  The server should reject invalid requests.

## Points to Ponder

Even though the mock database is single-threaded and event based, your http server can be anything.  It can scale to multiple theads, processes, or even machines.  Run as many instances of the db as you wish.  Just report your architecture in your published benchmark results.

I've included a couple database clients that I used in testing.  One is written using node.js, the other using luvit.io.  They are simple and probably will break under high load, but it gives a good idea about how to use the mock database.