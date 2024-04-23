#include "widget.h"
#include "./ui_widget.h"



Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(ui->btnNewLine, &QPushButton::clicked, this, &Widget::on_bthNewLine_clicked);
    connect(ui->btnClear, &QPushButton::clicked, this, &Widget::on_btnClear_clicked);

}

Widget::~Widget()
{
    delete ui;
}


void Widget::drawBezierCurve(Line &line) {
    QPainterPath path(line.point[0]);

    QPointF sp, ep, c1, c2;
    for (int i = 0; i < line.point.size() - 1; ++i) {
        // 控制点的 x 坐标为 sp 与 ep 的 x 坐标和的一半
        // 第一个控制点 c1 的 y 坐标为起始点 sp 的 y 坐标
        // 第二个控制点 c2 的 y 坐标为结束点 ep 的 y 坐标
        QPointF sp = line.point[i];
        QPointF ep = line.point[i+1];
        if (i == 0) {
            c1 = QPointF((sp.x() + ep.x()) / 2, sp.y());
            c2 = QPointF((sp.x() + ep.x()) / 2, ep.y());
        } else {
            c1 = line.point[i] + (line.point[i] - c2);
            // QPointF mid = (line.point[i] + line.point[i + 1]) / 2;
            // c2 = mid + (mid - c1);
            c2 = QPointF((sp.x() + ep.x()) / 2, ep.y());
        }
        path.cubicTo(c1, c2, ep);
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(line.color, 2));

    // 绘制 path
    //painter.translate(40, 130);
    painter.drawPath(path);

    // 绘制曲线上的点
    painter.setBrush(Qt::gray);
    for (int i = 0; i < line.point.size(); ++i) {
        painter.drawEllipse(line.point[i], 4, 4);
    }
}

void Widget::paintEvent(QPaintEvent *event)
{
    // QWidget::paintEvent(event);

    // QPainter painter(this);
    // painter.setPen(QPen(Qt::black, 2));


    for (auto &line : lines) {
        if (line.point.empty()) continue;
        drawBezierCurve(line);
    }
    qDebug() << "line number:" << lines.size() << "\n";

}


void Widget::mousePressEvent(QMouseEvent *event)
{

    if (event->button() == Qt::LeftButton) {
        if (lines.empty()) lines.push_back({});
        lines.back().point.push_back(event->pos());
        update(); // 更新绘图
    }
    // if (event->button() == Qt::RightButton) {
    //     line.point.push_back({});
    //     printf("line.point number: %d\n", line.point.size());
    // }
}

void Widget::on_bthNewLine_clicked()
{
    qDebug() << "New Line Clicked\n";
    if (!lines.back().point.empty()) lines.push_back(Line());
    qDebug() << "Now line number:" << lines.size() << "\n";
    update();
}


void Widget::on_btnClear_clicked()
{
    qDebug() << "Clear Clicked\n";
    lines.clear();
    qDebug() << "Now line.point number:" << lines.size() << "\n";
    update();
}

