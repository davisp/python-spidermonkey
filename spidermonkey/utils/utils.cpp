
#include <spidermonkey.h>

PyObjXDR::PyObjXDR(PyObject* p)
{
    this->data = p;
}

PyObjXDR::~PyObjXDR()
{
    Py_XDECREF(this->data);
}

PyObject*
PyObjXDR::get()
{
    return this->data
}
