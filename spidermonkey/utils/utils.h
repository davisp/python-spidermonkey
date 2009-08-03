/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#ifndef PYSM_UTILS_H
#define PYSM_UTILS_H

#include <Python.h>
#include <frameobject.h>

#include <iostream>

template <class T> class PyXDR {
    public:
        PyXDR(T* p = 0)
        {
            this->data = p;
        }
        
        ~PyXDR()
        {
            Py_XDECREF(this->data);
        }

        PyXDR<T>& operator=(T* p)
        {
            this->data = p;
            return *this;
        }

        T* operator->()
        {
            return this->data;
        }
        
        operator bool () const
        {
            return this->data != NULL;
        }
        
        T* get() const
        {
            return this->data;
        }
        
        void set(T* p)
        {
            this->data = p;
        }
        
        T* release()
        {
            T* p = this->data;
            this->data = NULL;
            return p;
        }

    protected:
        T* data;

};

typedef PyXDR<PyObject> PyObjectXDR;
typedef PyXDR<PyCodeObject> PyCodeXDR;
typedef PyXDR<PyFrameObject> PyFrameXDR;


// JS UTILS

class JSRequest {
    public:
        JSRequest(JSContext* cx);
        ~JSRequest();
    private:
        JSContext* cx;
};

#endif