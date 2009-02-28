
cdef class Runtime:
    cdef JSRuntime* rt

    def __cinit__(self):
        self.rt = JS_NewRuntime(1000000)
        if self.rt == NULL:
            raise JSError("Failed to create JavaScript Runtime.")

    def __dealloc__(self):
        JS_DestroyRuntime(self.rt)

    def create_context(self, root=None):
        cx = Context(self, root)
        return cx
