#ifndef FINDMAXCURVATURE_H
#define FINDMAXCURVATURE_H

#include <Python.h>
#include <QPoint>
#include <utility>
#include <QDebug>

std::pair<double, double> findMaxCurvature(QPointF P0, QPointF P1, QPointF P2, QPointF P3);

#endif // FINDMAXCURVATURE_H
