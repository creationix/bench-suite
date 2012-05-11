#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "uv.h"

static uv_stream_t server;

#define USER_DATA               \
  "{\"name\":\"Tim Caswell\""   \
  ",\"twitter\":\"creationix\"" \
  ",\"github\":\"creationix\""  \
  ",\"irc\":\"creationix\""     \
  ",\"projects\":[\"node\",\"Luvit\",\"Luvmonkey\",\"candor.io\",\"vfs\",\"architect\",\"wheat\",\"step\"]"\
  ",\"websites\":[\"http://howtonode.org/\",\"http://creationix.com/\",\"http://nodebits.org/\"]"\
  "}"

#define SESSION_DATA              \
  "{\"username\": \"creationix\"" \
  ",\"pageViews\": 0"             \
  "}"

#define ERROR_DATA "Invalid Query"

typedef struct {
  uv_tcp_t handle;
  uv_write_t write_req;
} client_t;

void on_close(uv_handle_t* handle) {
  free(handle);
  /* printf("disconnected\n"); */
}

uv_buf_t on_alloc(uv_handle_t* handle, size_t suggested_size) {
  uv_buf_t buf;
  buf.base = malloc(suggested_size);
  buf.len = suggested_size;
  return buf;
}

void on_read(uv_stream_t* stream, ssize_t nread, uv_buf_t buf) {
  client_t* client = stream->data;

  // Handle data

  free(buf.base);
}

void on_connection(uv_stream_t* server_handle, int status) {
  assert(server_handle == &server);
  printf("connected\n"); 

  client_t* client = malloc(sizeof(client_t));
  uv_tcp_init(uv_default_loop(), &client->handle);
  client->handle.data = client;

  int r = uv_accept(&server, (uv_stream_t*)&client->handle);

  if (r) {
    uv_err_t err = uv_last_error(uv_default_loop());
    fprintf(stderr, "accept: %s\n", uv_strerror(err));
    exit(-1);
  }

  uv_read_start((uv_stream_t*)&client->handle, on_alloc, on_read);

}

void after_write(uv_write_t* req, int status) {
  printf("after_write\n");
  uv_close((uv_handle_t*)req->handle, on_close);
}


int main() {

  uv_tcp_init(uv_default_loop(), (uv_tcp_t*)&server);
  struct sockaddr_in address = uv_ip4_addr("0.0.0.0", 5555);
  int r = uv_tcp_bind((uv_tcp_t*)&server, address);

  if (r) {
    uv_err_t err = uv_last_error(uv_default_loop());
    fprintf(stderr, "bind: %s\n", uv_strerror(err));
    return -1;
  }

  r = uv_listen(&server, 128, on_connection);

  if (r) {
    uv_err_t err = uv_last_error(uv_default_loop());
    fprintf(stderr, "listen: %s\n", uv_strerror(err));
    return -1;
  }

  printf("Raw C database listening on port 5555\n");
  /* Block in the main loop */
  uv_run(uv_default_loop());

  return 0;
}