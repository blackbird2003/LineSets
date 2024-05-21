#include "widget.h"
#include "./ui_widget.h"
#include "solveTSP.h"
#include "optimizeCP.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setStyleSheet("background-color: grey;");

}

Widget::~Widget()
{
    delete ui;
}

bool intersected_clicked = false;
bool boundingBox_clicked = false;
bool largeAngle_clicked = false;
bool test_clicked = false;
bool SA_clicked = false;

float CalcCost(QPointF c1, QPointF c2, QPointF sp, QPointF ep) {
    //cost define as length + max curvature
    double length, maxCurvature;
    QPainterPath path(sp); path.cubicTo(c1, c2, ep);
    length = path.length();
    maxCurvature = findMaxCurvature(sp, c1, c2, ep).first;
    if (rand() < RAND_MAX * 0.0001) qDebug() << length << " " << maxCurvature << "\n";
    return 0.0001 * length;
}

#define rd() ((rand() * 2 - RAND_MAX) / RAND_MAX * 100)

const float initT = 100;
const float bar = 0.01;
const float cold = 0.96;

void SimulateAnnealing(QPointF &c1, QPointF &c2, QPointF sp, QPointF ep, QPointF dir) {
    double T = initT;
    QPointF u1 = c1, u2 = c2;
    while (T > bar) {
        // c1 can only change length of c1 - sp, while c2 can change freely
        auto v1 = u1 + dir * T * rd();
        auto v2 = u2;
        v2.setX(u2.x() + T * rd());
        v2.setY(u2.y() + T * rd());
        auto costv = CalcCost(v1, v2, sp, ep), costu = CalcCost(u1, u2, sp, ep);
        double delta = costv - costu;
        if (delta < 0)
            u1 = v1, u2 = v2;
        else if (rand() < RAND_MAX * exp(-delta / T)) {
            //qDebug() << costu << " -> " << costv << " " <<exp(-delta / T) ;
             u1 = v1, u2 = v2;
        }

        T -= 0.01;
        //qDebug() << T << " " << costv << " " << costu << "\n";
    }
    c1 = u1, c2 = u2;
}

void Line::sortByTSP() {
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
}



/*

New Algorithm:

Step1: Give default control points
Step2: Iterate each part
       When interating part i
       - both c1 and c2 can move freely
       - and c1 will affect the c2 in i-1 -th part
The valuating function:
       considering the curve consisting of part 1 to i
       length: total length of i curves
       max curvature:
       max(max(curvature of lastsp lastc1 lastc2 lastep)
           max(curvature of lastc2 sp c1 c2),
           max(curvature of sp c1 c2 ep)))

 */

void Line::generateControlPoint() {
    controlPoint.clear();
    QPointF sp, ep, c1, c2, dir;
    vector<float> sumLength(point.size());
    for (int i = 0; i < point.size() - 1; i++) {
        sp = point[i], ep = point[i + 1];
        if (i == 0) {
            c1 = QPointF((sp.x() + ep.x()) / 2, sp.y());
        } else {
            dir = sp - c2;
            dir = dir * len(ep - sp) / len(dir) / 3;
            c1 = sp + dir;
        }
        c2 = (c1 + ep) / 2;
        if (SA_clicked && i != 0) {
            // qDebug() << "before:" << c1 << " " << c2 << findMaxCurvature(sp, c1, c2, ep).first;
            // SimulateAnnealing(c1, c2, sp, ep, dir);
            // qDebug() << "after:" << c1 << " " << c2 << findMaxCurvature(sp, c1, c2, ep).first;
            auto opt = OptimizeControlPoints(point[i-1], controlPoint[controlPoint.size() - 2], controlPoint[controlPoint.size() - 1],
                                            sp, c1, c2, ep);
            qDebug() << "before:" << c1 << c2;
            c1 = opt.first; c2 = opt.second;
            qDebug() << "after:" << c1 << c2;
            auto lc2 = controlPoint.back();
            lc2 = sp * 2 - c1;
            controlPoint.pop_back();
            controlPoint.push_back(lc2);
        }
        controlPoint.push_back(c1);
        controlPoint.push_back(c2);
    }
    //qDebug() << "CP generate finished\n";
    SA_clicked = false;
}

void Line::generateCurve() {

    if (controlPoint.empty()) generateControlPoint();

    int n = point.size();
    assert(controlPoint.size() == (n - 1) * 2);

    QPainterPath path(point[0]);
    for (int i = 0; i < n - 1; i++) {
        path.cubicTo(controlPoint[i * 2], controlPoint[i * 2 + 1], point[i + 1]);
    }
    curve = path;
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

    auto line = lines[line_id];
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

    // if (SA_clicked) {

    //}



    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(line.color, 2));

    line.generateCurve();
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
    for (auto pt:line.controlPoint) {
        painter.setBrush(Qt::green);
        painter.drawEllipse(pt, 4, 4);
    }
}

void Widget::paintEvent(QPaintEvent *event)
{
    for (int i = 0; i < lines.size(); i++) {
        drawBezierCurve(i);
    }

    QPainter painter(this);
    if (intersected_clicked) {
        for (auto line1 : lines) {
            for (auto line2 : lines) {
                if (line1.color == line2.color) continue;
                QPainterPath path1 = line1.curve;
                QPainterPath path2 = line2.curve;
                QPainterPath path_intersected = path1.intersected(path2);
                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setPen(QPen(QColor(255, 0, 0), 2));
                painter.drawPath(path_intersected);
            }
        }
    }

    if (boundingBox_clicked) {
        for (auto &line : lines) {
            QRectF boundingBox = line.curve.boundingRect();
            QPainterPath path;
            path.addRect(boundingBox);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setPen(QPen(QColor(0, 0, 255), 2));
            painter.drawPath(path);
        }
    }

    ui->labelLineNumber->setText("Line number:" + QString(QString::number((int)lines.size())));
    //qDebug() << "line number:" << lines.size() << "\n";

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


void Widget::on_bthSA_clicked()
{
    SA_clicked = !SA_clicked;
    qDebug() << "SA_clicked is" << (SA_clicked ? "on\n" : "off\n");
    update();
}

