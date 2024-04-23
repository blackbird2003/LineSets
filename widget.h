#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>



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
    Line() {
        color = QColor(rand() % 256, rand() % 256, rand() % 256);
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
    void drawBezierCurve(Line &line);

private slots:
    void on_bthNewLine_clicked();

    void on_btnClear_clicked();

private:
    Ui::Widget *ui;
    std::vector<Line> lines;
};


#endif // WIDGET_H
