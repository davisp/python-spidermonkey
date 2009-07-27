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

template<class T> class PyPtr {
    public:
        PyPtr(T* p = 0)
        {
            this->data = p;
        }
        
        ~PyPtr() {}

        T*
        get() const
        {
            return this->data;
        }
        
        void
        reset()
        {
            this->data = NULL;
        }

        T*
        operator->()
        {
            return this->data;
        }
        
        PyPtr<T>&
        operator=(T* p)
        {
            this->data = p;
            return *this;
        }
        
        PyPtr<T>&
        operator=(PyPtr<T>& p)
        {
            this->data = p.data;
            p.reset();
            return *this;
        }
    
        operator bool () const
        {
            return this->data != NULL;
        }
        
    protected:
        T*   data;
};

template <class T> class PyXDR : public PyPtr<T> {
    public:
        PyXDR(T* p = 0)
        {
            this->data = p;
        }
        
        ~PyXDR()
        {
            Py_XDECREF(this->data);
        }
        
        PyXDR<T>&
        operator=(T* p)
        {
            this->data = p;
            return *this;
        }
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