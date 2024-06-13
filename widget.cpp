#include "widget.h"
#include "./ui_widget.h"
#include "solveTSP.h"
#include "optimizeCP.h"
#include "optimizeCP_global.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <QMenuBar>
#include <QFileDialog>
#include <QTextStream>


double optPara = 10000;

QPointF *draggedControlPoint = nullptr;
QPointF *centerPoint = nullptr;
QPointF *pairControlPoint = nullptr;
bool isDragging = false;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setStyleSheet("background-color: white;");

    setupMenu();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMenuBar(menuBar);
    setLayout(layout);

}

Widget::~Widget()
{
    delete ui;
}

bool intersected_clicked = false;
bool boundingBox_clicked = false;
bool largeAngle_clicked = false;
bool test_clicked = false;
bool Optimize_clicked = false;
bool optCpGlobal_clicked = false;
bool showControlPoints = true;



void Line::sortByTSP() {
    qDebug() << "Sorting TSP";
    int n = point.size();
    if (n <= 2) return;

    std::vector<int> path = /*ans.path*/SolveTSP(point);
    //assert(path.size() == n);
    std::vector<QPointF> res;
    for (int i = 0; i < n; i++) {
        res.push_back(point[path[i]]);
    }

    assert(point.size() == n);
    //qDebug() << "point size: " << point.size() << "\n";
    point = res;

    controlPoint.clear(); cost = 0;
}



/*
Algorithm 1: only consider current part
Step1: Give default control points
Step2: for each part, c1 move on the line defined by sp and c1
       c2 moves free
       optimize by considering
       k1 * length + k2 * max_curvature, only in this part


Algorithm 2: consider the whole curve
Step1: Give default control points
Step2: All control points move freely
       optimize by considering
       k1 * total length + k2 * max_curvature of whole curve
       curvature in endpoint are approximated by
            curvature of lastc1 lastc2 sp c1
            curvature of lastc2 sp c1 c2

Algorithm 3:
Step1: Give default control points
Step2:
We note that, even if both c1 and c2 in part i freely change
it will only affect part i-1, will not affect  part 1,2,...,i-2

Iterate each part
       When interating part i
       - both c1 and c2 can move freely
       - and c1 will affect the c2 in i-1 -th part
The valuating function:
       considering the curve consisting of part 1 to i
       length: total length of i curves
       max curvature:
       max(max(curvature of lastsp lastc1 lastc2 sp),
           max(curvature of lastc1 lastc2 sp c1)
           max(curvature of lastc2 sp c1 c2),
           max(curvature of sp c1 c2 ep)))

 */

void Line::generateControlPoint() {
    // i = 0, 1, 2, ..., n - 1
    int i = controlPoint.size() / 2;
    if (i > point.size() - 1) return;

    QPointF sp = point[i], ep = point[i + 1];
    QPointF c1, c2, dir;
    vector<float> sumLength(point.size());

    if (i == 0) {
        c1 = QPointF((sp.x() + ep.x()) / 2, sp.y());
        c2 = (c1 + ep) / 2;
        controlPoint.push_back(c1);
        controlPoint.push_back(c2);
        return;
    }

    dir = sp - controlPoint.back();
    dir = dir * len(ep - sp) / len(dir) / 3;
    c1 = sp + dir;
    c2 = (c1 + ep) / 2;

    if (Optimize_clicked) {
        auto opt = OptimizeControlPoints(1, optPara, point[i-1], controlPoint[controlPoint.size() - 2], controlPoint[controlPoint.size() - 1],
                                         sp, c1, c2, ep);
        //qDebug() << "before:" << c1 << c2;
        c1 = std::get<0>(opt); c2 = std::get<1>(opt);
        //qDebug() << "after:" << c1 << c2;
        //qDebug() << "optPara:" << optPara;
        //this->maxCurvature = max(this->maxCurvature, std::get<2>(opt));
        auto lc2 = controlPoint.back();
        lc2 = sp * 2 - c1;
        controlPoint.pop_back();
        controlPoint.push_back(lc2);
    } else {
        // auto opt = OptimizeControlPoints(0, optPara, point[i-1], controlPoint[controlPoint.size() - 2], controlPoint[controlPoint.size() - 1],
        //                                  sp, c1, c2, ep);
        //this->maxCurvature = max(this->maxCurvature, std::get<2>(opt));
    }

    controlPoint.push_back(c1);
    controlPoint.push_back(c2);
}

void Line::generateCurve() {

    int n = point.size();
    int m = controlPoint.size() / 2;
    if (Optimize_clicked) controlPoint.clear(), cost= 0;
    while (controlPoint.size() < (n - 1) * 2) {
        qDebug() << controlPoint.size() <<  (n - 1) * 2;
        generateControlPoint();
    }


    QPainterPath path(point[0]);
    this->maxCurvature = 0;
    for (int i = 0; i < n - 1; i++) {
        path.cubicTo(controlPoint[i * 2], controlPoint[i * 2 + 1], point[i + 1]);
        // 0 01 1 23 2 34
        if (i != 0) {
            auto opt = OptimizeControlPoints(0, optPara, point[i-1], controlPoint[(i-1)*2], controlPoint[(i-1)*2+1],
                                             point[i], controlPoint[i*2], controlPoint[i*2+1], point[i + 1]);
            maxCurvature = max(maxCurvature, std::get<2>(opt));
        }


    }
    curve = path;
    //Optimize_clicked = false;
    //optCpGlobal_clicked = false;
}

QPointF getIntersection(QPointF posA, QPointF posB, QPointF posC, QPointF posD)//返回AB与CD交点，无交点返回（0,0）
{
    QLineF line1(posA, posB);
    QLineF line2(posC, posD);
    QPointF interPos(0,0);
    QLineF::IntersectType type = line1.intersects(line2, &interPos);
    if (type != QLineF::BoundedIntersection)
        interPos = QPointF(1e9, 1e9);
    return interPos;
}




float angle(QPointF A, QPointF O, QPointF B) {
    auto OA = A - O, OB = B - O;
    auto cosAOB =  QPointF::dotProduct(OA, OB) / (len(OA) * len(OB));
    return acos(cosAOB);
}


float crossProduct(QPointF A, QPointF B) {
    return A.x() * B.y() - B.x() * A.y();
}


void Widget::drawBezierCurve(/*Line &line*/ int line_id) {


    auto &line = lines[line_id];
    if (line.point.empty()) return;

    //24 05 06 add invisable points

    // if (largeAngle_clicked) {
    //     const int segnum = 40;
    //     std::vector<QPoint> pt, tmp, inters;
    //     pt = line.point;
    //     tmp = pt;
    //     inters.clear();
    //     //24 05 07 switch to piecewise linearity
    //     QPainterPath path = generatePath(line.point);
    //     QPointF last = path.pointAtPercent(0);
    //     pt.push_back(last.toPoint());

    //     for (int i = 1; i <= segnum; i++) {
    //         auto s1 = last, e1 = path.pointAtPercent(1.f * i / segnum);
    //         bool changed = false;
    //         for (int id = 0; id < line_id; id++) {
    //             auto pre_path = lines[id].path;
    //             for (int j = 1; j <= segnum; j++) {
    //                 auto s2 = pre_path.pointAtPercent(1.f*(j-1)/segnum), e2 = pre_path.pointAtPercent(1.f*j/segnum);
    //                 auto inter = getIntersection(s1, e1, s2, e2);
    //                 inters.push_back(inter.toPoint());
    //                 if (inter.x() > 1e6 || std::min(angle(s1, inter, s2), angle(s1, inter, e2)) > 3.14159 / 6)
    //                     continue;
    //                 changed = true;
    //                 auto u = (s2 - inter);
    //                 auto v1 = QPoint(u.y(), -u.x()), v2 = -v1;
    //                 if (crossProduct(v1, s2 - inter) * crossProduct(s1 - inter, s2 - inter) < 0) {
    //                     std::swap(v1, v2);
    //                 }
    //                 //s1, e1 -> ns1, ne1
    //                 auto ns1 = inter + v1 * (length(s1 - e1) / length(v1));
    //                 auto ne1 = inter + v2 * (length(s1 - e1) / length(v2));
    //                 last = ne1;
    //                 tmp.push_back(last.toPoint());
    //                 pt.push_back(ns1.toPoint()); pt.push_back(ne1.toPoint());
    //                 break;
    //             }
    //             if (changed) break;
    //         }
    //         if (!changed) {
    //             last = e1;
    //             tmp.push_back(last.toPoint());
    //         }

    //     }
    // }

    // if (Optimize_clicked) {

    //}



    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(line.color, 2));

    if (Optimize_clicked || line.controlPoint.size() < (line.point.size() - 1) * 2) {
        line.generateCurve();
    }


    painter.drawPath(line.curve);


    if (test_clicked) {
        // 绘制曲线上的分段点
        painter.setBrush(Qt::blue);
    }


    // 绘制曲线上的可见点
    for (int i = 0; i < line.point.size(); ++i) {
        painter.setBrush(Qt::gray);
        if (i == 0) {
            painter.drawEllipse(line.point[i], 6, 6);
            auto str = std::to_string(line_id);
            painter.setBrush(Qt::red);
            painter.drawText(line.point[i] + QPoint(10, 0), QString(str.c_str()));
        }
        else painter.drawEllipse(line.point[i], 4, 4);
    }

    //绘制Control Point
    if (showControlPoints) {
        for (auto pt:line.controlPoint) {
            painter.setBrush(Qt::green);
            painter.drawEllipse(pt, 4, 4);
        }

        painter.setBrush(Qt::red);
        if(centerPoint) painter.drawEllipse(*centerPoint, 4, 4);


        // 设置画笔为黑色虚线
        QPen pen(Qt::black, 1);
        QVector<qreal> dashes;
        qreal space = 10; // 空白长度
        qreal dashLength = 4; // 虚线长度
        dashes << dashLength << space;
        pen.setDashPattern(dashes);

        painter.setPen(pen);

        // 绘制两个点之间的黑色虚线
        auto p = line.point;
        auto cp = line.controlPoint;
        auto n = p.size();
        for (int i = 0; i < n - 1; i++) {
            // 0 01 1 23 2 45 n-2 2n-4 2n-3 n-1
            painter.drawLine(p[i], cp[i * 2]);
            painter.drawLine(cp[i * 2], cp[i * 2 + 1]);
            painter.drawLine(cp[i * 2 + 1], p[i + 1]);
        }
    }
}

void Widget::paintEvent(QPaintEvent *event)
{
    qDebug() << "paintEvent ing..";

    double maxCurvature = 0, totalLength = 0;
    if (1 || Optimize_clicked || lines.back().controlPoint.size() < (lines.back().point.size() - 1) * 2) {
        for (int i = 0; i < lines.size(); i++) {
            drawBezierCurve(i);
            maxCurvature = max(maxCurvature, lines[i].maxCurvature);
            totalLength += lines[i].curve.length();
        }
    }

    QPainter painter(this);
    // if (intersected_clicked) {
    //     for (auto line1 : lines) {
    //         for (auto line2 : lines) {
    //             if (line1.color == line2.color) continue;
    //             QPainterPath path1 = line1.curve;
    //             QPainterPath path2 = line2.curve;
    //             QPainterPath path_intersected = path1.intersected(path2);
    //             painter.setRenderHint(QPainter::Antialiasing, true);
    //             painter.setPen(QPen(QColor(255, 0, 0), 2));
    //             painter.drawPath(path_intersected);
    //         }
    //     }
    // }

    // if (boundingBox_clicked) {
    //     for (auto &line : lines) {
    //         QRectF boundingBox = line.curve.boundingRect();
    //         QPainterPath path;
    //         path.addRect(boundingBox);
    //         painter.setRenderHint(QPainter::Antialiasing, true);
    //         painter.setPen(QPen(QColor(0, 0, 255), 2));
    //         painter.drawPath(path);
    //     }
    // }

    qDebug() << maxCurvature;
    ui->labelLineNumber->setText("Line number:" + QString(QString::number((int)lines.size())));

    ui->labelTotalLength->setText("Length: " + QString(QString::number(totalLength)));
    ui->labelMaxCurvature->setText("MaxCurvarure: " + QString(QString::number(maxCurvature)));
    ui->labelTotalCost->setText("Cost: " + QString(QString::number(totalLength + optPara * maxCurvature)));
    //qDebug() << "line number:" << lines.size() << "\n";
    Optimize_clicked = 0;

}

void Widget::on_btnNewLine_clicked()
{
    qDebug() << "New Line Clicked\n";
    if (lines.empty() || !lines.back().point.empty()) lines.push_back(Line());
    qDebug() << "Now line number:" << lines.size() << "\n";
    update();
}


void Widget::on_btnClear_clicked()
{
    qDebug() << "Clear Clicked\n";
    lines.clear();
    qDebug() << "Now line number:" << lines.size() << "\n";
    update();
}


void Widget::on_btnRandom_clicked()
{
    //lines.clear();
    int pointNumber = 5;
    Line l;
    for (int i = 0; i < pointNumber; i++) {
        l.point.push_back(generateRandomPoint());
    }
    //while (lines.back().point.empty()) lines.pop_back();
    lines.push_back(l);
    update();
}

QPoint Widget::generateRandomPoint()
{

    int width = this->width() - 100;
    int height = this->height() - 100;

    // 生成随机的x和y坐标
    int x = QRandomGenerator::global()->bounded(width) + 50;
    int y = QRandomGenerator::global()->bounded(height) + 50;

    qDebug() << x << " " << y << Qt::endl;

    return QPoint(x, y);
}

struct DP {
    double dis = 1e9;
    std::vector<int> path;
    bool operator<(const DP &t) const {
        return dis < t.dis;
    }
};




void Widget::on_bthSortCurrent_clicked()
{

    if (!lines.empty()) {
        qDebug() << "Length before sort: " << lines.back().curve.length() << "\n";
        lines.back().sortByTSP();
        qDebug() << "Length after sort: " << lines.back().curve.length() << "\n";
    }
    update();
}


void Widget::on_bthSortAll_clicked()
{
    int cnt = 0;
    for (auto &line : lines) {
        line.sortByTSP();
        qDebug() << "OK " << ++cnt << "\n";
    }
    update();
}


void Widget::on_bthIntersected_clicked()
{
    intersected_clicked = !intersected_clicked;
    qDebug() << "intersected_clicked\n";
    update();
}


void Widget::on_labelLineNumber_linkActivated(const QString &link)
{
    ui->labelLineNumber->setText("Line number:" + QString(QString::number((int)lines.size())));
}


void Widget::on_btnBoundingBox_clicked()
{
    boundingBox_clicked = !boundingBox_clicked;
    qDebug() << "boundingBox_clicked\n";
    update();
}


void Widget::on_bthLargeAngle_clicked()
{
    largeAngle_clicked = !largeAngle_clicked;
    qDebug() << "large angle is" << (largeAngle_clicked ? "on\n" : "off\n");
    update();
}


void Widget::on_bthTest_clicked()
{
    test_clicked = !test_clicked;
    qDebug() << "test_clicked is" << (test_clicked ? "on\n" : "off\n");
    update();
}


void Widget::on_btnOptimize_clicked()
{
    Optimize_clicked = !Optimize_clicked;
    qDebug() << "Optimize_clicked is" << (Optimize_clicked ? "on\n" : "off\n");
    update();
}

void Widget::on_bthCP_clicked()
{
    if (showControlPoints) {
        showControlPoints = false;
        ui->bthCP->setText("Show Control Points");
    } else {
        showControlPoints = true;
        ui->bthCP->setText("Hide Control Points");
    }
    update();
}


void Widget::on_btnOptCpGlobal_clicked()
{
    optCpGlobal_clicked = !optCpGlobal_clicked;
    qDebug() << "optCpGlobal_clicked is" << (optCpGlobal_clicked ? "on\n" : "off\n");
    update();
}

void Widget::on_lineEdit_textChanged(const QString &arg1)
{
    optPara = arg1.toDouble();
}


void Widget::on_btnUndo_clicked()
{
    if (!lines.empty()) {
        auto &l = lines.back();
        if (!l.point.empty()) {
            l.point.pop_back();
            while (l.controlPoint.size() > (l.point.size() - 1) * 2)
                l.controlPoint.pop_back();
            l.generateCurve();
        }
    }
    update();
}





void Widget::mousePressEvent(QMouseEvent *event)
{
    QPointF clickedPoint = event->pos();
    isDragging = false;

    // 检查是否点击在控制点上
    for (auto &line : lines) {
        auto &cp = line.controlPoint;
        for (int i = 0; i < cp.size(); i++) {
            auto pt = &cp[i];
            if (QLineF(clickedPoint, *pt).length() < 5) {
                draggedControlPoint = pt;
                if (i == 0) {
                    pairControlPoint = nullptr;
                    centerPoint = &line.point[0];
                } else if (i == cp.size() - 1) {
                    pairControlPoint = nullptr;
                    centerPoint = &line.point.back();
                } else if (i % 2 == 0) {
                    pairControlPoint = pt - 1;
                    centerPoint = &line.point[(i + 1) / 2];
                    // 12:1 34:2 56:3 ..
                } else {
                    pairControlPoint = pt + 1;
                    centerPoint = &line.point[(i + 1) / 2];
                }
                isDragging = true;
                break;
            }
        }
    }

    // 如果没有点击在控制点上，添加新点
    if (!isDragging) {
        if (lines.empty()) {
            lines.push_back(Line());
        }
        lines.back().point.push_back(clickedPoint);
        update();
    }
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging && draggedControlPoint) {
        *draggedControlPoint = event->pos();

        if (pairControlPoint) {
            auto old_len = len(*pairControlPoint - *centerPoint);
            auto dir = *centerPoint - *draggedControlPoint;
            dir = dir * old_len / len(dir);
            *pairControlPoint = *centerPoint + dir;
        }

        for (auto &line : lines) {
            line.generateCurve();
        }
        update();
    }
}

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    draggedControlPoint = centerPoint = pairControlPoint = nullptr;
    isDragging = false;
    update();
}

void Widget::setupMenu()
{
    menuBar = new QMenuBar(this);
    QMenu *fileMenu = new QMenu("File", this);

    importAction = new QAction("Load", this);
    connect(importAction, &QAction::triggered, this, &Widget::importFromFile);
    fileMenu->addAction(importAction);

    exportAction = new QAction("Save", this);
    connect(exportAction, &QAction::triggered, this, &Widget::exportToFile);
    fileMenu->addAction(exportAction);

    menuBar->addMenu(fileMenu);
}

void Widget::exportToFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, "保存文件", "", "Text Files (*.txt)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);

            for (int i = 0; i < lines.size(); ++i) {
                out << i << "\n";
                for (const auto &pt : lines[i].controlPoint) {
                    out << pt.x() << " " << pt.y() << "\n";
                }
            }

            file.close();
        }
    }
}

void Widget::importFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "打开文件", "", "Text Files (*.txt)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);

            lines.clear();
            Line line;
            int lineIndex = -1;
            while (!in.atEnd()) {
                QString lineText = in.readLine();
                if (lineText.trimmed().isEmpty()) {
                    continue;
                }
                bool ok;
                if (lineText.toInt(&ok)) {
                    if (lineIndex != -1) {
                        lines.push_back(line);
                        line.controlPoint.clear();
                        line.point.clear();
                    }
                    lineIndex = lineText.toInt();
                } else {
                    QStringList coords = lineText.split(' ');
                    if (coords.size() == 2) {
                        QPointF pt(coords[0].toFloat(), coords[1].toFloat());
                        line.point.push_back(pt);
                    }
                }
            }
            if (lineIndex != -1) {
                lines.push_back(line);
            }

            file.close();
            update();
        }
    }
}



