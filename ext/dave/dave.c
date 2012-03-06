/*
 * Document-class: Dave
 *
 * = Simple Ruby bindings for {libnodave}[http://libnodave.sourceforge.net/]
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <nodave.h>
#include <ruby.h>

static VALUE rb_cDave;
static VALUE rb_mSimatic;
static VALUE eConnectionError;

void dave_free_di(void *);
void dave_free_dc(void *);
void dave_free_fds(void *);

/*
 * Constructor
 */
static VALUE cDave_initialize(VALUE self, VALUE host, VALUE port) {
  rb_iv_set(self, "@host", host);
  rb_iv_set(self, "@port", port);
  return Qtrue;
}

/*
 * Connect to the PLC
 */
static VALUE cDave_connect(VALUE self) {
  int ret, opt;
  int fd;
  char *hostname;

  struct sockaddr_in addr;
  struct hostent *hp;
  socklen_t addrlen;

  _daveOSserialType *fds;
  daveInterface *di;
  daveConnection *dc;

  VALUE host;
  VALUE port;

  host = rb_iv_get(self, "@host");
  port = rb_iv_get(self, "@port");

  hostname = RSTRING_PTR(host);

  if ((addr.sin_addr.s_addr = inet_addr(hostname)) == -1) {
    hp = gethostbyname2(hostname, AF_INET);
    if (!hp) {
      rb_raise(eConnectionError, "gethostbyname2 failed on: %s", hostname);
      return Qnil;
    }

    memcpy(&addr.sin_addr, hp->h_addr_list[0], sizeof (addr.sin_addr));
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons((uint16_t)NUM2INT(port));

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    rb_raise(eConnectionError, "socket failed: %d", fd);
    return Qnil;
  }

  addrlen = sizeof(addr);
  ret = connect(fd, (struct sockaddr *)&addr, addrlen);
  if (ret < 0) {
    rb_raise(eConnectionError, "connect failed: %d", ret);
    return Qnil;
  }

  opt = 1;
  ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &opt, 4);
  if (ret < 0) {
    rb_raise(eConnectionError, "setsockopt failed: %d", ret);
    return Qnil;
  }

  fds = malloc(sizeof(_daveOSserialType));
  fds->rfd = fds->wfd = fd;

  di = daveNewInterface(*fds, "IF1", 0, daveProtoISOTCP, daveSpeed187k);
  daveSetTimeout(di, 5000000);
  dc = daveNewConnection(di, 2, 0, 2);
  ret = daveConnectPLC(dc);

  if (ret < 0) {
    rb_raise(eConnectionError, "daveConnectPLC failed: %d", ret);
    return Qnil;
  }

  rb_iv_set(self, "@fds", Data_Wrap_Struct(rb_cObject, 0, dave_free_fds, fds));
  rb_iv_set(self, "@di", Data_Wrap_Struct(rb_cObject, 0, dave_free_di, di));
  rb_iv_set(self, "@dc", Data_Wrap_Struct(rb_cObject, 0, dave_free_dc, dc));

  return Qtrue;
}

/*
 * Read
 */
static VALUE cDave_fetch(VALUE self, VALUE db, VALUE size) {
  int ret, i, j;
  char byte;
  char bit[8];
  char *tmp;
  VALUE bs, str;

  daveConnection *dc;
  Data_Get_Struct(rb_iv_get(self, "@dc"), daveConnection, dc);

  ret = daveReadBytes(dc, daveDB, NUM2INT(db), 0, NUM2INT(size), NULL);
  if (ret != 0) {
    rb_raise(eConnectionError, "daveReadBytes failed: %d", ret);
    return Qnil;
  }

  /*
   * This is ugly...
   */
  tmp = malloc(NUM2INT(size) * 8 + 1);
  bzero(tmp, NUM2INT(size) * 8 + 1);
  for (i = 0; i < NUM2INT(size); i++) {
    byte = daveGetU8(dc);
    for (j = 0; j < 8; j++) {
      bit[j] = ((byte >> j) & 0x01);
      tmp[i*8 + j] = bit[j] + 48;
    }
  }

  str = rb_str_new(tmp, strlen(tmp));
  bs = rb_funcall(rb_const_get(rb_mSimatic, rb_intern("Memory")), 
      rb_intern("new"), 2, str, INT2NUM(NUM2INT(size) * 8));

  free(tmp);

  return bs;
}

/*
 * Write
 */
static VALUE cDave_send(VALUE self, VALUE db, VALUE cmd) {
  int ret;

  daveConnection *dc;

  if (RSTRING_PTR(cmd) == NULL) {
    return Qnil;
  }

  Data_Get_Struct(rb_iv_get(self, "@dc"), daveConnection, dc);

  ret = daveWriteBytes(dc, daveDB, NUM2INT(db), 0, RSTRING_LEN(cmd), RSTRING_PTR(cmd));
  if (ret != 0) {
    return Qnil;
  }

  return Qtrue;
}

void dave_free_di(void *di) {
  free(di);
}

void dave_free_dc(void *dc) {
  free(dc);
}

void dave_free_fds(void *p) {
  _daveOSserialType *fds = p;
  close(fds->rfd);
  free(fds);
}

void Init_dave() {
  rb_require("rubygems");
  rb_require("simatic");
  rb_require("socket");

  rb_cDave = rb_define_class("Dave", rb_cObject);
  rb_mSimatic = rb_const_get(rb_cObject, rb_intern("Simatic"));

  eConnectionError = rb_const_get(rb_cObject, rb_intern("SocketError"));

  rb_define_method(rb_cDave, "initialize", cDave_initialize, 2);
  rb_define_method(rb_cDave, "connect", cDave_connect, 0);
  rb_define_method(rb_cDave, "fetch", cDave_fetch, 2);
  rb_define_method(rb_cDave, "send", cDave_send, 2);
}
