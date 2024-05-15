#include "optimizeCP.h"

#include <QDebug>


std::pair<QPointF, QPointF> OptimizeControlPoints(QPointF P0, QPointF P1, QPointF P2, QPointF P3) {
    Py_Initialize();
    PyGILState_STATE gilState = PyGILState_Ensure();

    // 设置Python模块搜索路径
    PyObject* sysPath = PySys_GetObject("path");
    PyList_Append(sysPath, PyUnicode_FromString("D:\\Documents\\QtProject\\LineSets"));

    // 导入模块
    PyObject* pName = PyUnicode_FromString("optimize_control_points");
    PyObject* pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule == nullptr) {
        PyErr_Print();
        PyGILState_Release(gilState);
        throw std::runtime_error("Failed to load \"optimize_control_points\"");
    }

    // 获取函数
    PyObject* pFunc = PyObject_GetAttrString(pModule, "optimize_control_points");
    if (!pFunc || !PyCallable_Check(pFunc)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(pModule);
        PyGILState_Release(gilState);
        throw std::runtime_error("Cannot find function optimize_control_points");
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
        throw std::runtime_error("Call to optimize_control_points failed");
    }

    // 处理返回值
    double p1x = PyFloat_AsDouble(PyTuple_GetItem(pValue, 0));
    double p1y = PyFloat_AsDouble(PyTuple_GetItem(pValue, 1));
    double p2x = PyFloat_AsDouble(PyTuple_GetItem(pValue, 2));
    double p2y = PyFloat_AsDouble(PyTuple_GetItem(pValue, 3));

    Py_DECREF(pValue);
    PyGILState_Release(gilState);
    qDebug() << P0 << P1 << P2 << P3;
    auto np1 = QPointF(p1x, p1y), np2 = QPointF(p2x, p2y);
    qDebug() << P0 << np1 << np2 << P3;
    return std::make_pair(np1, np2);
}
