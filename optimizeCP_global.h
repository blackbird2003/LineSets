#ifndef OPTIMIZECP_GLOBAL_H
#define OPTIMIZECP_GLOBAL_H

#include <Python.h>
#include <iostream>
#include <utility>
#include <QPointF>
#include <vector>
using namespace std;

vector<QPointF> OptimizeControlPoints_global(vector<QPointF> p, vector<QPointF> cp);

#endif // OPTIMIZECP_GLOBAL_H
