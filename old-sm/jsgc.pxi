
cdef class GC:
    cdef Context cx

    def __cinit__(GC self, Context cx):
        self.cx = cx

    cdef void run(GC self):
        JS_GC(self.cx.cx)
    
    cdef void run_maybe(GC self):
        JS_MaybeGC(self.cx.cx)
    
    cdef void add_root(GC self, void* rp):
        if not JS_AddRoot(self.cx.cx, rp):
            raise JSError("Failed to add a GC root")
    
    cdef void add_named_root(GC self, void* rp, char* name):
        if not JS_AddNamedRoot(self.cx.cx, rp, name):
            raise JSError("Failed to add a named GC root: %s" % name)

    cdef void rem_root(GC self, void* rp):
        if not JS_RemoveRoot(self.cx.cx, rp):
            raise JSError("Failed to remove GC root")
