#include <Python.h>
#include <vector>
#include <QPointF>
#include <stdexcept>

std::vector<QPointF> OptimizeControlPoints_global(std::vector<QPointF> p, std::vector<QPointF> cp) {
    if (cp.size() != 2 * p.size() - 2) {
        throw std::invalid_argument("cp must have size of 2n - 2");
    }

    PyGILState_STATE gilState = PyGILState_Ensure();

    // 设置Python模块搜索路径
    PyObject* sysPath = PySys_GetObject("path");
    PyList_Append(sysPath, PyUnicode_FromString("D:\\Documents\\QtProject\\LineSets"));

    // 导入模块
    PyObject* pName = PyUnicode_FromString("optimize_control_points_global");
    PyObject* pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule == nullptr) {
        PyErr_Print();
        PyGILState_Release(gilState);
        throw std::runtime_error("Failed to load \"optimize_control_points_global\"");
    }

    // 获取函数
    PyObject* pFunc = PyObject_GetAttrString(pModule, "optimize_control_points_global");
    if (!pFunc || !PyCallable_Check(pFunc)) {
        PyErr_Print();
        Py_DECREF(pModule);
        PyGILState_Release(gilState);
        throw std::runtime_error("Cannot find function optimize_control_points_global");
    }

    // 构建参数
    PyObject* pyP = PyList_New(p.size());
    for (size_t i = 0; i < p.size(); ++i) {
        PyList_SetItem(pyP, i, Py_BuildValue("(dd)", p[i].x(), p[i].y()));
    }
    PyObject* pyCP = PyList_New(cp.size());
    for (size_t i = 0; i < cp.size(); ++i) {
        PyList_SetItem(pyCP, i, Py_BuildValue("(dd)", cp[i].x(), cp[i].y()));
    }

    PyObject* pArgs = PyTuple_Pack(2, pyP, pyCP);

    // 调用函数
    PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);
    Py_DECREF(pFunc);
    Py_DECREF(pModule);

    if (pValue == nullptr) {
        PyErr_Print();
        PyGILState_Release(gilState);
        throw std::runtime_error("Call to optimize_control_points_global failed");
    }

    // 处理返回值
    std::vector<QPointF> result;
    if (PyList_Check(pValue)) {
        for (Py_ssize_t i = 0; i < PyList_Size(pValue); ++i) {
            PyObject* item = PyList_GetItem(pValue, i);
            double x = PyFloat_AsDouble(PyTuple_GetItem(item, 0));
            double y = PyFloat_AsDouble(PyTuple_GetItem(item, 1));
            result.push_back(QPointF(x, y));
        }
    }

    Py_DECREF(pValue);
    PyGILState_Release(gilState);

    return result;
}
