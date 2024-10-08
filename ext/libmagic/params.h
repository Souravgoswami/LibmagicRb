void params(volatile VALUE rb_klass) {
	#ifdef MAGIC_PARAM_INDIR_MAX
	rb_define_const(rb_klass, "MAGIC_PARAM_INDIR_MAX", UINT2NUM(MAGIC_PARAM_INDIR_MAX));
	#endif

	#ifdef MAGIC_PARAM_NAME_MAX
	rb_define_const(rb_klass, "MAGIC_PARAM_NAME_MAX", UINT2NUM(MAGIC_PARAM_NAME_MAX));
	#endif

	#ifdef MAGIC_PARAM_ELF_NOTES_MAX
	rb_define_const(rb_klass, "MAGIC_PARAM_ELF_NOTES_MAX", UINT2NUM(MAGIC_PARAM_ELF_NOTES_MAX));
	#endif

	#ifdef MAGIC_PARAM_ELF_PHNUM_MAX
	rb_define_const(rb_klass, "MAGIC_PARAM_ELF_PHNUM_MAX", UINT2NUM(MAGIC_PARAM_ELF_PHNUM_MAX));
	#endif

	#ifdef MAGIC_PARAM_ELF_SHNUM_MAX
	rb_define_const(rb_klass, "MAGIC_PARAM_ELF_SHNUM_MAX", UINT2NUM(MAGIC_PARAM_ELF_SHNUM_MAX));
	#endif

	#ifdef MAGIC_PARAM_REGEX_MAX
	rb_define_const(rb_klass, "MAGIC_PARAM_REGEX_MAX", UINT2NUM(MAGIC_PARAM_REGEX_MAX));
	#endif

	#ifdef MAGIC_PARAM_BYTES_MAX
	rb_define_const(rb_klass, "MAGIC_PARAM_BYTES_MAX", UINT2NUM(MAGIC_PARAM_BYTES_MAX));
	#endif
}

VALUE lsparams(volatile VALUE obj) {
	VALUE hash = rb_hash_new();

	#ifdef MAGIC_PARAM_INDIR_MAX
	rb_hash_aset(hash, ID2SYM(rb_intern("MAGIC_PARAM_INDIR_MAX")), UINT2NUM(MAGIC_PARAM_INDIR_MAX));
	#endif

	#ifdef MAGIC_PARAM_NAME_MAX
	rb_hash_aset(hash, ID2SYM(rb_intern("MAGIC_PARAM_NAME_MAX")), UINT2NUM(MAGIC_PARAM_NAME_MAX));
	#endif

	#ifdef MAGIC_PARAM_ELF_NOTES_MAX
	rb_hash_aset(hash, ID2SYM(rb_intern("MAGIC_PARAM_ELF_NOTES_MAX")), UINT2NUM(MAGIC_PARAM_ELF_NOTES_MAX));
	#endif

	#ifdef MAGIC_PARAM_ELF_PHNUM_MAX
	rb_hash_aset(hash, ID2SYM(rb_intern("MAGIC_PARAM_ELF_PHNUM_MAX")), UINT2NUM(MAGIC_PARAM_ELF_PHNUM_MAX));
	#endif

	#ifdef MAGIC_PARAM_ELF_SHNUM_MAX
	rb_hash_aset(hash, ID2SYM(rb_intern("MAGIC_PARAM_ELF_SHNUM_MAX")), UINT2NUM(MAGIC_PARAM_ELF_SHNUM_MAX));
	#endif

	#ifdef MAGIC_PARAM_REGEX_MAX
	rb_hash_aset(hash, ID2SYM(rb_intern("MAGIC_PARAM_REGEX_MAX")), UINT2NUM(MAGIC_PARAM_REGEX_MAX));
	#endif

	#ifdef MAGIC_PARAM_BYTES_MAX
	rb_hash_aset(hash, ID2SYM(rb_intern("MAGIC_PARAM_BYTES_MAX")), UINT2NUM(MAGIC_PARAM_BYTES_MAX));
	#endif

	return hash;
}
