# Benchmark Challenge

As I work on several platforms that are often used as web frameworks, the first example application is always an http server that serves the static string "Hello World" to all users.  Then these are benchmarked against each other to prove which platform is the best at being web scale.

Then there comes a flood of comments about how this benchmark is meaningless for any real-world work. So I hereby propose a new standard for http benchmarks.  Using this standard data, platforms can implement this challenge and see how well they perform.

## The Database

Most real apps have some sort of persistent database.  Here I'll have two "tables" named "users" and "sessions".  Their contents are simple.  This can be implemented using an in-memory object like I've done here, or it can be in redis, riak, or some other system.  Whatever makes sense for your platform.

```js
var users = {
  creationix: {
    name: "Tim Caswell",
    twitter: "creationix",
    github: "creationix",
    irc: "creationix",
    projects: ["node", "Luvit", "Luvmonkey", "candor.io", "vfs", "architect", "wheat", "step"],
    websites: ["http://howtonode.org/", "http://creationix.com/", "http://nodebits.org/"]
  },
};

var sessions = {
  eo299pqyw9791jie7yp: {
    username: "creationix",
    pageViews: 0,
  },
};
```

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

The http request should have a cookie called "SESSID" or something that contains the session key "eo299pqyw9791jie7yp".  In order to make this benchmark fair, no caching is allowed in the session or database lookups (If you use just one process, then an in-process object or table is fair game).  This is to simulate requests not being the same user and session every time.  The html template, however can be compiled and saved at startup since a site usually has a finite number of templates.

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

Notice that I said that it's legal to store the data in-memory.  This means that a simple node server can implement this without doing any I/O to gather the data.  But if you want to scale across CPU cores for maximum speed, then a shared data store is required.  This is a tradeoff in real applications.  Scaling to threads within a process, then to processes within a machine, then across machines, across data centers, etc is hard.  The larger the cluster, the harder is it to share data.  Since the data is read-only, this isn't too hard, but I don't want to make the benchmark too complicated.