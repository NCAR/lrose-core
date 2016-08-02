
#include <Python.h>



static PyObject * sand_addtwo(PyObject *self,PyObject *args){

    int inValue,outValue;

    if(!PyArg_ParseTuple(args,"i",&inValue)){
	return NULL;
    }

    outValue = inValue + 2;

    return Py_BuildValue("i",outValue);
}


static PyMethodDef SandMethods[] = {
    {"addtwo",sandd_addtwo,METH_VARARGS,
     "Add two."},
    {NULL,NULL,0,NULL}
};

/PyMODINIT_FUNC initsand(void){
    (void) Py_InitModule("sand","SandMethods");
}
