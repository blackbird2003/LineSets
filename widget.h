#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QRandomGenerator>



#include <vector>


QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE




class Line {
public:
    std::vector<QPoint> point; // 记录鼠标点击的位置
    QColor color;
    QPainterPath path;
    double length() {
        if (point.size() < 2) return 0;
        double res = 0;
        for (int i = 1; i < point.size(); i++) {
            QPoint t = point[i] - point[i - 1];
            res += sqrt(QPointF::dotProduct(t, t));
        }
        return res;
    }
    Line() {
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

private:
    Ui::Widget *ui;
    std::vector<Line> lines;
};


#endif // WIDGET_H
