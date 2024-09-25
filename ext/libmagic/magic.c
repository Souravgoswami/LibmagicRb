#include <magic.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ruby.h"

/*
* LibmagicRB Header files
*/
#include "modes.h"
#include "params.h"
#include "definitions.h"

#ifndef MAGIC_VERSION
#define MAGIC_VERSION 0
#endif

/*
* Errors
*/
VALUE rb_eFileNotFoundError;
VALUE rb_eFileNotReadableError;
VALUE rb_eInvalidDBError;
VALUE rb_eIsDirError;
VALUE rb_eFileClosedError;

// Garbage collect
void file_free(void *data) {
	if(data) {
		magic_close(*(void **)data);
		*(void **)data = NULL;
	}

	free(data);
}

// Filetype
static rb_data_type_t fileType = {
	.wrap_struct_name = "file",
	.function = {
		.dmark = NULL,
		.dfree = file_free,
	},
	.data = NULL,

	#ifdef RUBY_TYPED_FREE_IMMEDIATELY
	.flags = RUBY_TYPED_FREE_IMMEDIATELY
	#endif
};

#include "validations.h"
#include "func.h"

/*
	Directly check a file with LibmagicRb, without creating any sort of cookies:

		LibmagicRb.check(file: '/tmp/', db: '/usr/share/file/misc/magic.mgc', mode: LibmagicRb::MAGIC_CHECK) # => "sticky, directory"

	[file] The key `file:` is the filename to check. Should be a string

	[db] The key `db:` can be left as nil. Or you can give it the path of the current magic database.

	[mode] The key `mode` can be any of the LibmagicRb.lsmodes().
	To combine modes you can use `|`. For example:
	`mode: LibmagicRb::MAGIC_CHECK | LibmagicRb::MAGIC_SYMLINK | Libmagic_MAGIC_MIME`
	If `mode` key is nil, it will default to `MAGIC_MIME | MAGIC_CHECK | MAGIC_SYMLINK`
*/
static VALUE _check_(VALUE obj, VALUE args) {
	if(!RB_TYPE_P(args, T_HASH)) {
		rb_raise(rb_eArgError, "Expected hash as argument.");
	}

	// Database Path
	VALUE argDBPath = rb_hash_aref(args, ID2SYM(rb_intern("db")));

	char *databasePath = NULL;
	if (!NIL_P(argDBPath)) {
		if (!RB_TYPE_P(argDBPath, T_STRING)) {
			rb_raise(rb_eArgError, "Database name must be an instance of String.");
		}

		databasePath = StringValuePtr(argDBPath);
	}

	// File path
	VALUE argFilePath = rb_hash_aref(args, ID2SYM(rb_intern("file")));
	if (RB_TYPE_P(argFilePath, T_NIL)) {
		rb_raise(rb_eArgError, "Expected `file:\" key as a string, got nil instead");
	} else if (!RB_TYPE_P(argFilePath, T_STRING)) {
		rb_raise(rb_eArgError, "Filename must be an instance of String.");
	}
	char *checkPath = StringValuePtr(argFilePath);

	// Modes
	VALUE argModes = rb_hash_aref(args, ID2SYM(rb_intern("mode")));
	unsigned int modes;
	if(RB_TYPE_P(argModes, T_NIL)) {
		modes = MAGIC_MIME | MAGIC_CHECK | MAGIC_SYMLINK;
	} else if (!RB_TYPE_P(argModes, T_FIXNUM)) {
		rb_raise(rb_eArgError, "Modes must be an instance of Integer. Check LibmagicRb.constants() or LibmagicRb.lsmodes().");
	} else {
		modes = FIX2UINT(argModes);
	}

	// Checks
	struct magic_set *magic = magic_open(modes);

	if (!magic) {
		rb_raise(rb_eRuntimeError, "Failed to initialize magic cookie.");
	}

	// Check if the database is a valid file or not
	// Raises ruby error which will return.
	fileReadable(checkPath);

	if(databasePath) {
		magic_validate_db(magic, databasePath);
	}

	if (magic_load(magic, databasePath) == -1) {
		magic_close(magic);
		rb_raise(rb_eInvalidDBError, "Failed to load magic database: %s", magic_error(magic));
	}

	const char *mt = magic_file(magic, checkPath);

	VALUE retVal = mt ? rb_str_new_cstr(mt) : Qnil;
	magic_close(magic);

	return retVal;
}

/*
	Intializes a magic cookie that can be used multiple times.
	The benefit of this is to assgign various cookies and change flags of each cookie.

	For example:

		> cookie = LibmagicRb.new(file: '/tmp')
		# => #<LibmagicRb:0x000055810139f738 @closed=false, @db=nil, @file="/tmp", @mode=1106>

		> cookie.check
		# => "inode/directory; charset=binary"

		> cookie.setflags(LibmagicRb::MAGIC_RAW)
		# => 256

		> cookie.check
		# => "sticky, directory"

		> cookie2 = LibmagicRb.new(file: '/usr/share/dict/words')
		# => #<LibmagicRb:0x000055810190a060 @closed=false, @db=nil, @file="/usr/share/dict/words", @mode=1106>

		> cookie2.check
		# => "text/plain; charset=utf-8"

		> cookie.close
		# => #<LibmagicRb:0x000055810139f738 @closed=true, @db=nil, @file="/tmp", @mode=256>

		> cookie.closed?
		# => true

		> cookie2.closed?
		# => false

	Here in this example, we can't use cookie, but cookie2 is a different magic_t wrapper, so we can continue using that.
	Flags/modes applied to cookie, will not affect cookie2 as well. Think of them as totally different containers.
	Of course, you must close cookies when you don't need them. Otherwise it can use memories unless GC is triggered.
*/
VALUE rb_libmagicRb_initialize(VALUE self, VALUE args) {
	// Database Path
	if(!RB_TYPE_P(args, T_HASH)) {
		rb_raise(rb_eArgError, "Expected hash as argument.");
	}

	VALUE argDBPath = rb_hash_aref(args, ID2SYM(rb_intern("db")));

	if (RB_TYPE_P(argDBPath, T_NIL)) {
		rb_ivar_set(self, rb_intern("@db"), Qnil);
	} else if (!RB_TYPE_P(argDBPath, T_STRING)) {
		rb_raise(rb_eArgError, "Database name must be an instance of String.");
	} else {
		rb_ivar_set(self, rb_intern("@db"), argDBPath);
	}

	// File path
	VALUE argFilePath = rb_hash_aref(args, ID2SYM(rb_intern("file")));
	if (RB_TYPE_P(argFilePath, T_NIL)) {
		rb_raise(rb_eArgError, "Expected `file:\" key as a string, got nil instead");
	} else if (!RB_TYPE_P(argFilePath, T_STRING)) {
		rb_raise(rb_eArgError, "Filename must be an instance of String.");
	}
	rb_ivar_set(self, rb_intern("@file"), argFilePath);

	// Modes
	VALUE argModes = rb_hash_aref(args, ID2SYM(rb_intern("mode")));
	unsigned int modes;
	if(RB_TYPE_P(argModes, T_NIL)) {
		modes = MAGIC_MIME | MAGIC_CHECK | MAGIC_SYMLINK;
	} else if (!RB_TYPE_P(argModes, T_FIXNUM)) {
		rb_raise(rb_eArgError, "Modes must be an instance of Integer. Check LibmagicRb.constants() or LibmagicRb.lsmodes().");
	} else {
		modes = FIX2UINT(argModes);
	}
	rb_ivar_set(self, rb_intern("@mode"), UINT2NUM(modes));

	rb_ivar_set(self, rb_intern("@closed"), Qfalse);

	RB_UNWRAP(cookie);
	magic_setflags(*cookie, modes);

	return self;
}

VALUE initAlloc(VALUE self) {
	magic_t *cookie;
	cookie = malloc(sizeof(*cookie));

	if (!cookie) {
		rb_raise(rb_eNoMemError, "Failed to allocate memory for magic cookie.");
	}

	*cookie = magic_open(0);

	return TypedData_Wrap_Struct(self, &fileType, cookie);
}

void Init_main() {
	rb_global_variable(&rb_eFileNotFoundError);
	rb_global_variable(&rb_eFileNotReadableError);
	rb_global_variable(&rb_eInvalidDBError);
	rb_global_variable(&rb_eIsDirError);
	rb_global_variable(&rb_eFileClosedError);

	/*
	* Libmagic Errors
	*/

	/*
		Adds ability to check mime-type of a file using the libmagic (magic(4)). It uses native extensions and it's quite performant.
	*/
	VALUE cLibmagicRb = rb_define_class("LibmagicRb", rb_cObject);

	rb_eFileNotFoundError = rb_define_class_under(cLibmagicRb, "FileNotFound", rb_eRuntimeError);
	rb_eFileNotReadableError = rb_define_class_under(cLibmagicRb, "FileUnreadable", rb_eRuntimeError);
	rb_eInvalidDBError = rb_define_class_under(cLibmagicRb, "InvalidDBError", rb_eRuntimeError);
	rb_eIsDirError = rb_define_class_under(cLibmagicRb, "IsDirError", rb_eRuntimeError);
	rb_eFileClosedError = rb_define_class_under(cLibmagicRb, "FileClosedError", rb_eRuntimeError);

	/*
	* Constants
	*/
	modes(cLibmagicRb);
	params(cLibmagicRb);


	#if MAGIC_VERSION > 525
		char version[6];
		sprintf(version, "%0.2f", magic_version() / 100.0);

		/*
			LibmagicRb::MAGIC_VERSION returns the magic version of the library.
			For older libmagic version, this can be undefined, so this method will return "0" instead.
		*/
		rb_define_const(cLibmagicRb, "MAGIC_VERSION", rb_str_new_cstr(version));
	#else
		rb_define_const(cLibmagicRb, "MAGIC_VERSION", rb_str_new_cstr("0"));
	#endif

	/*
	* Singleton Methods
	*/
	rb_define_singleton_method(cLibmagicRb, "check", _check_, 1);
	rb_define_singleton_method(cLibmagicRb, "lsmodes", lsmodes, 0);
	rb_define_singleton_method(cLibmagicRb, "lsparams", lsparams, 0);

	/*
	* Instance Methods
	*/
	rb_define_alloc_func(cLibmagicRb, initAlloc);

	// LibmagicRb.new()
	rb_define_method(cLibmagicRb, "initialize", rb_libmagicRb_initialize, 1);

	// Attributes
	rb_define_attr(cLibmagicRb, "closed", 1, 0);
	rb_define_attr(cLibmagicRb, "mode", 1, 0);
	rb_define_attr(cLibmagicRb, "file", 1, 1);
	rb_define_attr(cLibmagicRb, "db", 1, 1);

	// Close database
	rb_define_method(cLibmagicRb, "close", _closeGlobal_, 0);
	rb_define_alias(cLibmagicRb, "closed?", "closed");

	// Load database
	rb_define_method(cLibmagicRb, "load", _loadGlobal_, 1);

	// Check for file mimetype
	rb_define_method(cLibmagicRb, "check", _checkGlobal_, 0);

	// Get and set params
	rb_define_method(cLibmagicRb, "getparam", _getParamGlobal_, 1);
	rb_define_method(cLibmagicRb, "setparam", _setParamGlobal_, 2);

	// Set modes dynamically
	rb_define_method(cLibmagicRb, "setflags", _setflagsGlobal_, 1);
	rb_define_method(cLibmagicRb, "mode=", _setflagsGlobal_, 1);

	// Miscellaneous
	rb_define_method(cLibmagicRb, "magic_buffer", _bufferGlobal_, 1);
	rb_define_method(cLibmagicRb, "magic_list", _listGlobal_, 0);
}
