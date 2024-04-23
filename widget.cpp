#include "widget.h"
#include "./ui_widget.h"



Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(ui->pushButton, &QPushButton::clicked, this, &Widget::on_pushButton_clicked);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::drawBezierCurve(std::vector<QPoint> &Line) {
    QPainterPath path(Line[0]);

    for (int i = 0; i < Line.size() - 1; ++i) {
        // 控制点的 x 坐标为 sp 与 ep 的 x 坐标和的一半
        // 第一个控制点 c1 的 y 坐标为起始点 sp 的 y 坐标
        // 第二个控制点 c2 的 y 坐标为结束点 ep 的 y 坐标
        QPointF sp = Line[i];
        QPointF ep = Line[i+1];
        QPointF c1 = QPointF((sp.x() + ep.x()) / 2, sp.y());
        QPointF c2 = QPointF((sp.x() + ep.x()) / 2, ep.y());

        path.cubicTo(c1, c2, ep);
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(Qt::black, 2));

    // 绘制 path
    //painter.translate(40, 130);
    painter.drawPath(path);

    // 绘制曲线上的点
    painter.setBrush(Qt::gray);
    for (int i = 0; i < Line.size(); ++i) {
        painter.drawEllipse(Line[i], 4, 4);
    }
}

void Widget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 2));


    for (auto &Line : Lines) {
        if (Line.empty()) continue;
        drawBezierCurve(Line);
        // painter.setPen(QPen(Qt::black, 5));
        // for (size_t i = 1; i < Line.size(); ++i) {
        //     painter.setPen(QPen(Qt::black, 5));
        //     painter.drawPoint(Line[i]);
        //     painter.setPen(QPen(Qt::red, 2));
        //     painter.drawLine(Line[i - 1], Line[i]);
        // }
    }
    qDebug() << "Line number:" << Lines.size() << "\n";

}


void Widget::mousePressEvent(QMouseEvent *event)
{

    if (event->button() == Qt::LeftButton) {
        if (Lines.empty()) Lines.push_back({});
        Lines.back().push_back(event->pos());
        update(); // 更新绘图
    }
    // if (event->button() == Qt::RightButton) {
    //     Line.push_back({});
    //     printf("Line number: %d\n", Line.size());
    // }
}

void Widget::on_pushButton_clicked()
{
    qDebug() << "Clicked\n";
    if (!Lines.back().empty()) Lines.push_back({});
     qDebug() << "Now Line number:" << Lines.size() << "\n";
}

