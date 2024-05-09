#include "solveTSP.h"


// 计算两点之间的欧几里得距离
double EuclideanDistance(const QPoint& a, const QPoint& b) {
    return sqrt(pow(a.x() - b.x(), 2) + pow(a.y() - b.y(), 2));
}

vector<int> SolveTSP(vector<QPoint> &points) {
    int n = points.size();
    // 创建距离矩阵
    vector<vector<double>> dis(n + 1, vector<double>(n + 1, 0.0));

    // 填充距离矩阵
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= n; ++j) {
            if (i == j) {
                dis[i][j] = 0;
            } else {
                dis[i][j] = EuclideanDistance(points[i - 1], points[j - 1]);
            }
        }
    }

    // 初始化Python环境
    Py_Initialize();

    PyObject* sysPath = PySys_GetObject("path");
    PyList_Append(sysPath, PyUnicode_FromString("D:\\Documents\\QtProject\\LineSets"));

    // 导入模块
    PyObject* pyModuleName = PyUnicode_FromString("tsp_solver");
    PyObject* pyModule = PyImport_Import(pyModuleName);
    Py_DECREF(pyModuleName);

    if (!pyModule) {
        PyErr_Print();
        qDebug() << "Error importing Python module\n";

    }

    // 获取函数
    PyObject* pyFunc = PyObject_GetAttrString(pyModule, "solve_float_matrix");
    if (!pyFunc || !PyCallable_Check(pyFunc)) {
        if (PyErr_Occurred())
            PyErr_Print();
        throw runtime_error("Cannot find function solve_float_matrix");
    }

    // 构建参数
    PyObject* pyList = PyList_New(n + 1);
    for (int i = 0; i <= n; ++i) {
        PyObject* pySubList = PyList_New(n + 1);
        for (int j = 0; j <= n; ++j) {
            PyList_SetItem(pySubList, j, PyFloat_FromDouble(dis[i][j]));
        }
        PyList_SetItem(pyList, i, pySubList);
    }

    // 调用函数
    PyObject* pyResult = PyObject_CallObject(pyFunc, PyTuple_Pack(1, pyList));
    Py_DECREF(pyList);
    Py_DECREF(pyFunc);
    Py_DECREF(pyModule);

    if (!pyResult) {
        PyErr_Print();
        throw runtime_error("Call to solve_float_matrix failed");
    }

    vector<int> result;
    if (PyList_Check(pyResult)) {
        for (Py_ssize_t i = 0; i < PyList_Size(pyResult); i++) {
            PyObject* pyItem = PyList_GetItem(pyResult, i);
            result.push_back((int)PyLong_AsLong(pyItem));
        }
    }

    Py_DECREF(pyResult);
    Py_Finalize();

    vector<int> res;
    for (auto i : result) {
        if (i != 0) res.push_back(i - 1);
    }
    return res;
}

// int main() {
//     // 示例用法
//     vector<QPoint> points = {{0, 0}, {1, 1}, {1, 0}, {0, 1}};
//     vector<int> tour = SolveTSP(points);
//     for (int i : tour) {
//         cout << i << " ";
//     }
//     cout << endl;
//     return 0;
// }

