#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <numpy/arrayobject.h>
#include <stdio.h>
#include "szimulacio.h"


PyObject* run(PyObject self[], PyObject args[]) {

    // numpy array objektum definiálása
    PyArrayObject *tnO, *fr_startO, *initO, *hibakO, *kifizetesekO, *statisztikakO;
    PyArg_ParseTuple(args, "OOOOOO", &tnO, &fr_startO, &initO, &hibakO, &kifizetesekO, &statisztikakO);
    
    // a numpy objektumból kihámozom az adatok pointereit
    unsigned char * tn = (unsigned char *) PyArray_DATA(tnO);
    unsigned char * fr_start = (unsigned char *) PyArray_DATA(fr_startO);
    unsigned short * init = (unsigned short *) PyArray_DATA(initO);
    float * hibak =  (float *) PyArray_DATA(hibakO);
    unsigned char * kifizetesek = (unsigned char *) PyArray_DATA(kifizetesekO);
    float * statisztikak = (float *) PyArray_DATA(statisztikakO);

    // szimuláció futtatása
    szimulacio(tn, fr_start, init, hibak, kifizetesek, statisztikak);
    
Py_RETURN_NONE;
}


// modul beállítások

PyMethodDef fuggvenyek[] = {
    {"run", run, METH_VARARGS, "Szimulacio futtatasa."},  // nev, referencia, _, doc
    {NULL, NULL, 0, NULL}
};

struct PyModuleDef cmodul = { PyModuleDef_HEAD_INIT, " ", " ", -1, fuggvenyek };

PyMODINIT_FUNC PyInit_cmodul(void) {
    import_array();
return (PyModule_Create(&cmodul));
}

