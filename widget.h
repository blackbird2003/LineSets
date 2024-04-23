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

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event); // 处理鼠标点击事件
    void drawBezierCurve(std::vector<QPoint> &Line);

private slots:
    void on_pushButton_clicked();

private:
    Ui::Widget *ui;

    std::vector<std::vector<QPoint>> Lines; // 记录鼠标点击的位置


};


#endif // WIDGET_H
