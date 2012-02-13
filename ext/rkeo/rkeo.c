/*
 * Document-class: Rkeo
 *
 * = Simple Ruby bindings for {rkeo}[http://code.google.com/p/rkeo/]
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>
#include <rk512.h>
#include <r3964.h>
#include <ruby.h>

static VALUE rb_cRkeo;
static VALUE rb_mSimatic;
static VALUE eConnectionError;
static VALUE eArgumentError;
static VALUE eMemoryError;

/*
 * Constructor
 */
static VALUE cRkeo_initialize(VALUE self, VALUE port, VALUE baud) {
	int ret;
	char *errorMessage = malloc(2048);
	char *fileName = RSTRING_PTR(port);
	THandle *handle;

	VALUE fd;

	handle = createConnection(fileName, NUM2INT(baud), 'E', 1, true, 0, 0, 0, 0, 0, errorMessage);
	if (!handle) {
		rb_raise(eConnectionError, "createConnection failed: %s", errorMessage);
		return Qnil;
	}

	ret = fcntl(handle->desc, F_SETFL, FNDELAY);
	if (ret < 0) {
		rb_raise(eConnectionError, "fcntl failed");
		return Qnil;
	}

	fd = rb_funcall(rb_const_get(rb_cObject, rb_intern("IO")), rb_intern("for_fd"), 1, INT2NUM(handle->desc));

	rb_iv_set(self, "@fd", fd);
	rb_iv_set(self, "@handle", Data_Wrap_Struct(rb_cObject, 0, 0, handle));

	return Qtrue;
}

/*
 * Read
 */
static VALUE cRkeo_fetch(VALUE self, VALUE db, VALUE size, VALUE version) {
	int ret, i, j;
	char *data;
	char bit[8];
	char *tmp;

	VALUE bs;
	VALUE str;

	THandle *handle;
	printf("%s\n", RSTRING_PTR(version));

	data = malloc(NUM2INT(size));
	tmp = malloc(NUM2INT(size) * 8 + 1);

	Data_Get_Struct(rb_iv_get(self, "@handle"), THandle, handle);

	if (!strcmp(RSTRING_PTR(version), "bytes")) {
		ret = fetchRk512Dbb(handle, NUM2INT(db), 0, NUM2INT(size), data);
		if (ret == -1) {
			rb_raise(eConnectionError, "fetchRk512Dbb failed: %s", handle->errorMessage);
			return Qnil;
		}
	} else if (!strcmp(RSTRING_PTR(version), "words")) {
		ret = fetchRk512Dbw(handle, NUM2INT(db), 0, NUM2INT(size), data);
		if (ret == -1) {
			rb_raise(eConnectionError, "fetchRk512Dbw failed: %s", handle->errorMessage);
			return Qnil;
		}
	} else if (!strcmp(RSTRING_PTR(version), "doubles")) {
		ret = fetchRk512Dbd(handle, NUM2INT(db), 0, NUM2INT(size), data);
		if (ret == -1) {
			rb_raise(eConnectionError, "fetchRk512Dbd failed: %s", handle->errorMessage);
			return Qnil;
		}
	} else if (!strcmp(RSTRING_PTR(version), "floats")) {
		ret = fetchRk512DbReal(handle, NUM2INT(db), 0, NUM2INT(size), data);
		if (ret == -1) {
			rb_raise(eConnectionError, "fetchRk512DbReal failed: %s", handle->errorMessage);
			return Qnil;
		}
	} else {
		rb_raise(eArgumentError, "invalid version");
		return Qnil;
	}

	for (i = 0; i < NUM2INT(size); i++) {
		for (j = 0; j < 8; j++) {
			bit[j] = ((data[i] >> j) & 0x01);
			tmp[i*8 + j] = bit[j] + 48;
		}
	}

	str = rb_str_new(tmp, NUM2INT(size));
	bs = rb_funcall(rb_const_get(rb_cObject, rb_intern("Memory")), rb_intern("new"), 1, str);
	
	free(data);
	
	return bs;
}

/*
 * Писане
 */
static VALUE cRkeo_send(VALUE self, VALUE db, VALUE cmd, VALUE version) {
	int ret;

	VALUE size;

	THandle *handle;

	if (RSTRING_PTR(cmd) == NULL) {
		rb_raise(eArgumentError, "cmd not specified");
		return Qnil;
	}

	if (RSTRING_PTR(version) == NULL) {
		rb_raise(eArgumentError, "version not specified");
		return Qnil;
	}

	Data_Get_Struct(rb_iv_get(self, "@handle"), THandle, handle);
	size = INT2NUM(RSTRING_LEN(cmd));

	if (!strcmp(RSTRING_PTR(version), "bytes")) {
		ret = sendRk512Dbb(handle, NUM2INT(db), 0, NUM2INT(size), RSTRING_PTR(cmd));
		if (ret == -1) {
			rb_raise(eConnectionError, "sendRk512Dbb failed: %s", handle->errorMessage);
			return Qnil;
		}
	} else if (!strcmp(RSTRING_PTR(version), "words")) {
		ret = sendRk512Dbw(handle, NUM2INT(db), 0, NUM2INT(size), RSTRING_PTR(cmd));
		if (ret == -1) {
			rb_raise(eConnectionError, "sendRk512Dbw failed: %s", handle->errorMessage);
			return Qnil;
		}
	} else if (!strcmp(RSTRING_PTR(version), "doubles")) {
		ret = sendRk512Dbd(handle, NUM2INT(db), 0, NUM2INT(size), RSTRING_PTR(cmd));
		if (ret == -1) {
			rb_raise(eConnectionError, "sendRk512Dbd failed: %s", handle->errorMessage);
			return Qnil;
		}
	} else if (!strcmp(RSTRING_PTR(version), "floats")) {
		ret = sendRk512DbReal(handle, NUM2INT(db), 0, NUM2INT(size), RSTRING_PTR(cmd));
		if (ret == -1) {
			rb_raise(eConnectionError, "sendRk512DbReal failed: %s", handle->errorMessage);
			return Qnil;
		}
	} else {
		rb_raise(eArgumentError, "invalid version");
		return Qnil;
	}

	return Qtrue;
}

/*
 * Should use garbage collector...
 */
static VALUE cRkeo_dispose(VALUE self) {
	THandle *handle;

	Data_Get_Struct(rb_iv_get(self, "@handle"), THandle, handle);
	free(handle->errorMessage);
	destroyConnection(handle);

	return Qtrue;
}

void Init_rkeo() {
	rb_require("rubygems");
	rb_require("simatic");
	rb_require("socket");

	rb_cRkeo = rb_define_class("Rkeo", rb_cObject);
	rb_mSimatic = rb_const_get(rb_cObject, rb_intern("Simatic"));

	eConnectionError = rb_const_get(rb_cObject, rb_intern("SocketError"));
	eArgumentError = rb_const_get(rb_cObject, rb_intern("ArgumentError"));
	eMemoryError = rb_const_get(rb_cObject, rb_intern("NoMemoryError"));

	rb_define_method(rb_cRkeo, "initialize", cRkeo_initialize, 2);
	rb_define_method(rb_cRkeo, "fetch", cRkeo_fetch, 3);
	rb_define_method(rb_cRkeo, "send", cRkeo_send, 3);
	rb_define_method(rb_cRkeo, "dispose", cRkeo_dispose, 0);
}
