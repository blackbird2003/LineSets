#ifndef OPTIMIZECP_H
#define OPTIMIZECP_H

#include <Python.h>
#include <iostream>
#include <utility>
#include <QPointF>


std::pair<QPointF, QPointF> OptimizeControlPoints(QPointF P0, QPointF P1, QPointF P2, QPointF P3);

#endif // OPTIMIZECP_H
