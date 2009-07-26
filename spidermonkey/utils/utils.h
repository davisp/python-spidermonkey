/*
 * Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
 *
 * This file is part of the python-spidermonkey package released
 * under the MIT license.
 *
 */

#ifndef PYSM_CONVERT_H
#define PYSM_CONVERT_H

#include <Python.h>

class PyObjXDR {
    public:
        explicit PyObjXDR(PyObject* p = 0);
        ~PyObjXDR();

        void reset(PyObject* p = 0);
    
        PyObject* get() const;
    
    protected:
        PyObjXDR() {}
    
    private:
        PyObjXDR(const PyObjXDR&);
        const PyObjXDR& operator=(const PyObjXDR&);
        
        PyObject*   data;
};

#endif