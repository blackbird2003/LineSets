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

double EuclideanDistance(const QPoint& a, const QPoint& b);
vector<int> SolveTSP(vector<QPoint> &points);


#endif // SOLVETSP_H
