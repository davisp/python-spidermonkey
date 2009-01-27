
cdef class ObjectAdapter:
    cdef Context cx
    cdef ClassAdapter cl_adapter
    cdef ObjectAdapter parent
    cdef JSObject* js_obj
    cdef object py_obj

    def __cinit__(ObjectAdapter self, Context cx, ClassAdapter adapter, ObjectAdapter parent, object obj):
        self.cx = cx
        self.cl_adapter = adapter
        self.parent = parent
        self.js_obj = NULL
        self.py_obj = obj


