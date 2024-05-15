#include <Python.h>
#include <QPointF>
#include <utility>
#include <QDebug>

std::pair<double, double> findMaxCurvature(QPointF P0, QPointF P1, QPointF P2, QPointF P3) {
    Py_Initialize();

    // 获取GIL
    PyGILState_STATE gilState = PyGILState_Ensure();

    // 设置Python模块搜索路径
    PyObject* sysPath = PySys_GetObject("path");
    PyList_Append(sysPath, PyUnicode_FromString("D:\\Documents\\QtProject\\LineSets"));

    // 导入模块
    PyObject* pName = PyUnicode_FromString("findMaxCurvature");
    PyObject* pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule == nullptr) {
        PyErr_Print();
        PyGILState_Release(gilState);
        qDebug() << ("Failed to load \"curvature\"\n");
    }

    // 获取函数
    PyObject* pFunc = PyObject_GetAttrString(pModule, "find_max_curvature");
    if (!pFunc || !PyCallable_Check(pFunc)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(pModule);
        PyGILState_Release(gilState);
        throw std::runtime_error("Cannot find function find_max_curvature");
    }

    // 构建参数
    PyObject* pArgs = PyTuple_Pack(4,
                                   Py_BuildValue("[dd]", P0.x(), P0.y()),
                                   Py_BuildValue("[dd]", P1.x(), P1.y()),
                                   Py_BuildValue("[dd]", P2.x(), P2.y()),
                                   Py_BuildValue("[dd]", P3.x(), P3.y()));

    // 调用函数
    PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);
    Py_DECREF(pFunc);
    Py_DECREF(pModule);

    if (pValue == nullptr) {
        PyErr_Print();
        PyGILState_Release(gilState);
        throw std::runtime_error("Call to find_max_curvature failed");
    }

    std::pair<double, double> result;
    result.first = PyFloat_AsDouble(PyTuple_GetItem(pValue, 0));
    result.second = PyFloat_AsDouble(PyTuple_GetItem(pValue, 1));

    Py_DECREF(pValue);

    // 释放GIL
    PyGILState_Release(gilState);
    Py_Finalize();
    return result;
}
