#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <cassert>
#include "solveTSP.h"
#include "findMaxCurvature.h"


#include <vector>




QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

#define len(v) sqrt(QPointF::dotProduct(v, v) + 0.0001)
// float len(QPointF v) {
//     return
// }



float calcCost(QPointF &c1, QPointF &c2, QPointF sp, QPointF ep);
void SimulateAnnealing(QPointF &c1, QPointF &c2, QPointF sp, QPointF ep, QPointF dir);

class Line {
public:
    std::vector<QPointF> point;
    std::vector<QPointF> controlPoint;
    QColor color;
    QPainterPath curve;

    void sortByTSP();
    void generateControlPoint();
    void generateCurve();

    Line() {
        color = QColor(128 + rand() % 128, 128 + rand() % 128, 128 + rand() % 128);
    }
    Line(vector<QPointF> &v) {
        point = v;
        color = QColor(128 + rand() % 128, 128 + rand() % 128, 128 + rand() % 128);
    }
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event); // 处理鼠标点击事件
    void drawBezierCurve(int line_id);

    QPoint generateRandomPoint();
    void sortByTSP(std::vector<QPoint> &point);

private slots:
    void on_btnNewLine_clicked();

    void on_btnClear_clicked();

    void on_btnRandom_clicked();

    void on_bthSortCurrent_clicked();

    void on_bthSortAll_clicked();

    void on_bthIntersected_clicked();

    void on_labelLineNumber_linkActivated(const QString &link);

    void on_btnBoundingBox_clicked();

    void on_bthLargeAngle_clicked();

    void on_bthTest_clicked();



    void on_bthSA_clicked();

private:
    Ui::Widget *ui;
    std::vector<Line> lines;
};


#endif // WIDGET_H
