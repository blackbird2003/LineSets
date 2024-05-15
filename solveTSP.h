#ifndef SOLVETSP_H
#define SOLVETSP_H


#ifdef slots
#undef slots
#endif

#include <Python.h>

#ifndef slots
#define slots Q_SLOTS
#endif

#include <vector>
#include <cmath>
#include <QPoint>
#include <QDebug>
#include <iostream>

using namespace std;

float EuclideanDistance(const QPointF& a, const QPointF& b);
vector<int> SolveTSP(vector<QPointF> &points);


#endif // SOLVETSP_H
