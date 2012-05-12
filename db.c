#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "uv.h"

typedef enum {
  START = 0,
  USERNAME,
  SESSID
} state_t;

typedef struct {
  uv_tcp_t handle;
  state_t state;
  int offset;
} client_t;

static const char user_key[] = "users/creationix";
static int user_key_len = sizeof(user_key);

static char user[] =      \
  "{\"name\":\"Tim Caswell\""   \
  ",\"twitter\":\"creationix\"" \
  ",\"github\":\"creationix\""  \
  ",\"irc\":\"creationix\""     \
  ",\"projects\":[\"node\",\"Luvit\",\"Luvmonkey\",\"candor.io\",\"vfs\",\"architect\",\"wheat\",\"step\"]"\
  ",\"websites\":[\"http://howtonode.org/\",\"http://creationix.com/\",\"http://nodebits.org/\"]"\
  "}";
static int user_len = sizeof(user);

static const char session_key[] = "sessions/eo299pqyw9791jie7yp";
static int session_key_len = sizeof(session_key);

static char session[] =     \
  "{\"username\": \"creationix\"" \
  ",\"pageViews\": 0"             \
  "}";
static int session_len = sizeof(session);


static uv_buf_t on_alloc(uv_handle_t* handle, size_t suggested_size) {
  uv_buf_t buf;
  buf.base = malloc(suggested_size);
  buf.len = suggested_size;
  return buf;
}

static void on_close(uv_handle_t* handle) {
  printf("%p: Handle closed.\n", handle);
  free(handle);
}

static void after_write(uv_write_t* req, int status) {
  /* printf("%p: after_write\n", req->handle); */
  free(req);
}


static void on_read(uv_stream_t* socket, ssize_t nread, uv_buf_t buf) {
  client_t* client = socket->data;
  if (nread > 0) {
    int i;
    for (i = 0; i < nread; i++) {
      char c = buf.base[i];
      switch (client->state) {
      case START:
        if (c == user_key[0]) {
          client->state = USERNAME;
          client->offset = 1;
        }
        else if (c == session_key[0]) {
          client->state = SESSID;
          client->offset = 1;
        }
        else {
          uv_close((uv_handle_t*)socket, on_close);
          i = nread;
        }
        break;
      case USERNAME:
        if (user_key[client->offset] != c) {
          uv_close((uv_handle_t*)socket, on_close);
          i = nread;
          break;
        }
        if (client->offset == user_key_len - 1) {
          uv_write_t* req = (uv_write_t*)malloc(sizeof(uv_write_t));
          uv_buf_t data[] = {{ .base = user, .len = user_len }};
          uv_write(req, socket, data, 1, after_write);
          client->state = START;
          break;
        }
        client->offset++;
        break;
      case SESSID:
        if (session_key[client->offset] != c) {
          uv_close((uv_handle_t*)socket, on_close);
          i = nread;
          break;
        }
        if (client->offset == session_key_len - 1) {
          uv_write_t* req = (uv_write_t*)malloc(sizeof(uv_write_t));
          uv_buf_t data[] = {{ .base = session, .len = session_len }};
          uv_write(req, socket, data, 1, after_write);
          client->state = START;
          break;
        }
        client->offset++;
        break;
      }
    }
  }
  free(buf.base);
  if (nread < 0) {
    uv_err_t err = uv_last_error(uv_default_loop());
    if (err.code != UV_EOF) {
      fprintf(stderr, "%p: %s: %s\n", socket, uv_err_name(err), uv_strerror(err));
    }
    uv_close((uv_handle_t*)socket, on_close);
  }

}

static void on_connection(uv_stream_t* server, int status) {
  client_t* client = malloc(sizeof(client_t));
  uv_tcp_t* socket = &client->handle;
  uv_tcp_init(uv_default_loop(), socket);
  socket->data = client;
  printf("%p: New Client.\n", socket);
  if (uv_accept(server, (uv_stream_t*)socket)) {
    uv_err_t err = uv_last_error(uv_default_loop());
    fprintf(stderr, "%p: accept: %s\n", socket, uv_strerror(err));
    exit(-1);
  }
  uv_read_start((uv_stream_t*)socket, on_alloc, on_read);
}



int main() {

  uv_tcp_t server;

  uv_tcp_init(uv_default_loop(), &server);
  struct sockaddr_in address = uv_ip4_addr("0.0.0.0", 5555);

  if (uv_tcp_bind((uv_tcp_t*)&server, address)) {
    uv_err_t err = uv_last_error(uv_default_loop());
    fprintf(stderr, "%p: bind: %s\n", &server, uv_strerror(err));
    return -1;
  }

  if (uv_listen((uv_stream_t*)&server, 128, on_connection)) {
    uv_err_t err = uv_last_error(uv_default_loop());
    fprintf(stderr, "%p: listen: %s\n", &server, uv_strerror(err));
    return -1;
  }

  printf("%p: Raw C database listening on port 5555\n", &server);
  /* Block in the main loop */
  uv_run(uv_default_loop());


  return 0;
}
