#include "widget.h"
#include "./ui_widget.h"
#include "solveTSP.h"

double dis(QPoint a, QPoint b) {
    QPoint t = a - b;
    return sqrt(QPointF::dotProduct(t, t) + 0.0001);
}


bool intersected_clicked = false;
bool boundingBox_clicked = false;
bool largeAngle_clicked = false;
bool test_clicked = false;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    // connect(ui->btnNewLine, &QPushButton::clicked, this, &Widget::on_bthNewLine_clicked);
    // connect(ui->btnClear, &QPushButton::clicked, this, &Widget::on_btnClear_clicked);
    // connect(ui->btnRandom, &QPushButton::clicked, this, &Widget::on_btnRandom_clicked);
    setStyleSheet("background-color: grey;");

}

Widget::~Widget()
{
    delete ui;
}





QPainterPath generatePath(std::vector<QPoint> point) {
    if (point.empty()) return QPainterPath();

    QPainterPath path(point[0]);
    QPointF sp, ep, c1, c2;
    for (int i = 0; i < point.size() - 1; ++i) {
        QPointF sp = point[i];
        QPointF ep = point[i+1];
        //QPainterPath p1(line.point[i]), p2(line.point[i]);
        if (i == 0) {
            c1 = QPointF((sp.x() + ep.x()) / 2, /*(sp.y() + ep.y()) / 2*/ ep.y());
            //c2 = QPointF((sp.x() + ep.x()) / 2, ep.y());
            //p1.cubicTo(c1, c2, ep); p2 = p1;
        } else {
            auto veclength = dis(c1.toPoint(), point[i]);
            auto midlength = dis(point[i], point[i + 1]) / 2;
            c1 = point[i] + (point[i] - c1) * midlength / veclength;
            // QPointF mid = (line.point[i] + line.point[i + 1]) / 2;
            // c2 = mid + (mid - c1);

            c2 = QPointF((sp.x() + ep.x()) / 2, ep.y());
            // p1.cubicTo(c1, c2, ep);
            // c2 = QPointF((sp.x() + ep.x()) / 2, sp.y());
            // p2.cubicTo(c1, c2, ep);
        }
        //path.cubicTo(c1, c2, ep);
        path.quadTo(c1, ep);
    }
    return path;
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



float length(QPointF a) {
    return sqrt(QPointF::dotProduct(a, a));
}
float angle(QPointF A, QPointF O, QPointF B) {
    auto OA = A - O, OB = B - O;
    auto cosAOB =  QPointF::dotProduct(OA, OB) / (length(OA) * length(OB));
    return acos(cosAOB);
}


float crossProduct(QPointF A, QPointF B) {
    return A.x() * B.y() - B.x() * A.y();
}

const int segnum = 40;
std::vector<QPoint> pt, tmp, inters;
void Widget::drawBezierCurve(/*Line &line*/ int line_id) {
    auto line = lines[line_id];
    if (line.point.empty()) return;

    //24 05 06 add invisable points
    pt = line.point;
    tmp = pt;
    inters.clear();
    if (largeAngle_clicked) {
        //24 05 07 switch to piecewise linearity
        QPainterPath path = generatePath(line.point);
        QPointF last = path.pointAtPercent(0);
        pt.push_back(last.toPoint());

        for (int i = 1; i <= segnum; i++) {
            auto s1 = last, e1 = path.pointAtPercent(1.f * i / segnum);
            bool changed = false;
            for (int id = 0; id < line_id; id++) {
                auto pre_path = lines[id].path;
                for (int j = 1; j <= segnum; j++) {
                    auto s2 = pre_path.pointAtPercent(1.f*(j-1)/segnum), e2 = pre_path.pointAtPercent(1.f*j/segnum);
                    auto inter = getIntersection(s1, e1, s2, e2);
                    inters.push_back(inter.toPoint());
                    if (inter.x() > 1e6 || std::min(angle(s1, inter, s2), angle(s1, inter, e2)) > 3.14159 / 4)
                        continue;
                    changed = true;
                    auto u = (s2 - inter);
                    auto v1 = QPoint(u.y(), -u.x()), v2 = -v1;
                    if (crossProduct(v1, s2 - inter) * crossProduct(s1 - inter, s2 - inter) < 0) {
                        std::swap(v1, v2);
                    }
                    //s1, e1 -> ns1, ne1
                    auto ns1 = inter + v1 * (length(s1 - e1) / length(v1));
                    auto ne1 = inter + v2 * (length(s1 - e1) / length(v2));
                    last = ne1;
                    tmp.push_back(last.toPoint());
                    pt.push_back(ns1.toPoint()); pt.push_back(ne1.toPoint());
                    break;
                }
                if (changed) break;
            }
            if (!changed) {
                last = e1;
                tmp.push_back(last.toPoint());
            }

        }

        // for (int i = 1; i < pt.size(); i++) {
        //     auto s1 = pt[i - 1], e1 = pt[i];
        //     bool flag = false;
        //     for (auto [s2, e2] : drawn_segment) {
        //         auto inter = getIntersection(s1, e1, s2, e2);
        //         if (inter.x() > 1e6) continue;
        //         else {
        //             if (std::min(angle(s1, inter, s2), angle(s1, inter, e2)) < 3.14159 / 4) {
        //                 auto u = (s2 - inter);
        //                 auto v1 = QPoint(u.y(), -u.x()), v2 = -v1;

        //                 if (crossProduct(v1, s2 - inter) * crossProduct(s1 - inter, s2 - inter) < 0) {
        //                     std::swap(v1, v2);
        //                 }
        //                 if (test_clicked) {
        //                     auto p1 = inter + v1 * (length(s1 - e1) / 12 /length(v1));
        //                     auto p2 = inter + v2 * (length(s1 - e1)/ 12 / length(v2));
        //                     pt.insert(pt.begin() + i, p1); drawn_segment.push_back({pt[i - 1], pt[i]});
        //                     pt.insert(pt.begin() + i + 1, p2); drawn_segment.push_back({pt[i], pt[i + 1]});
        //                     drawn_segment.push_back({pt[i + 1], pt[i + 2]});
        //                     i += 2;
        //                 } else {
        //                     auto p1 = inter + v1 * (length(s1 - inter) / 2 /length(v1));
        //                     auto p2 = inter + v2 * (length(e1 - inter)/ 2 / length(v2));
        //                     pt.insert(pt.begin() + i, p1); drawn_segment.push_back({pt[i - 1], pt[i]});
        //                     pt.insert(pt.begin() + i + 1, inter); drawn_segment.push_back({pt[i], pt[i + 1]});
        //                     pt.insert(pt.begin() + i + 2, p2); drawn_segment.push_back({pt[i + 1], pt[i + 2]});
        //                     drawn_segment.push_back({pt[i + 2], pt[i + 3]});
        //                     i += 3;
        //                 }
        //                 flag = true;
        //                 qDebug() << "OK\n";
        //                 break;
        //             }
        //         }
        //     }
        //     if (!flag) drawn_segment.push_back({pt[i - 1], pt[i]});
        // }
    }




    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(line.color, 2));

    // 绘制 path
    //painter.translate(40, 130);

    QPainterPath path;
    if (largeAngle_clicked) {
        sortByTSP(pt);
        path = generatePath(/*line.point*/ pt);
        lines[line_id].path = path;
    } else {
        path = generatePath(line.point);
        lines[line_id].path = path;
    }
    painter.drawPath(path);





    if (test_clicked) {
        // 绘制曲线上的分段点
        painter.setBrush(Qt::blue);
        for (auto p : tmp) {
            painter.drawEllipse(p, 4, 4);
        }

        // 绘制曲线上的交叉点
        painter.setBrush(Qt::green);
        for (auto p : inters) {
            painter.drawEllipse(p, 4, 4);
        }

        // 绘制pt上的所有点
        painter.setBrush(Qt::red);
        for (auto p : pt) {
            painter.drawEllipse(p, 4, 4);
        }
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
}

void Widget::paintEvent(QPaintEvent *event)
{
    // QWidget::paintEvent(event);

    // QPainter painter(this);
    // painter.setPen(QPen(Qt::black, 2));


    for (int i = 0; i < lines.size(); i++) {
        drawBezierCurve(i);
    }

    QPainter painter(this);
    if (intersected_clicked) {
        for (auto line1 : lines) {
            for (auto line2 : lines) {
                if (line1.color == line2.color) continue;
                QPainterPath path1 = generatePath(line1.point);
                QPainterPath path2 = generatePath(line2.point);

                QPainterPath path_intersected = path1.intersected(path2);


                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setPen(QPen(QColor(255, 0, 0), 2));

                // 绘制 path
                //painter.translate(40, 130);
                painter.drawPath(path_intersected);
            }
        }
    }

    if (boundingBox_clicked) {
        for (auto &line : lines) {
            QRectF boundingBox = generatePath(line.point).boundingRect();
            QPainterPath path;
            path.addRect(boundingBox);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setPen(QPen(QColor(0, 0, 255), 2));

            // 绘制 path
            //painter.translate(40, 130);
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


const int maxn = 21;
DP f[1 << maxn][maxn];


int lowbit(int i) { return i & -i; }
void Widget::sortByTSP(std::vector<QPoint> &point) {
    int n = point.size();
    if (n <= 2) return;

    // std::random_shuffle(point.begin(), point.end());

    // for (int i = 0 ; i <= (1 << n) - 1; i++) {
    //     for (int j = 0; j <= n - 1; j++) {
    //         f[i][j].dis = 1e9;
    //         f[i][j].path.clear();
    //     }
    // }
    // // Dynamic Programming:
    // f[0][0].dis = 0;
    // for (int i = 1; i <= (1 << n) - 1; i++) {
    //     if (i == lowbit(i)) {
    //         int k = log2(i);
    //         f[i][k].dis = 0;
    //         f[i][k].path = {k};
    //         continue;
    //     }

    //     std::vector<int> v;
    //     for (int k = 0; k <= n - 1; k++) {
    //         if (i >> k & 1) v.push_back(k);
    //     }

    //     for (auto ki : v) {
    //         for (auto kj : v) {
    //             if (ki == kj) continue;
    //             int j = i - (1 << ki);
    //             double new_dis = f[j][kj].dis + dis(point[kj], point[ki]);
    //             if (new_dis < f[i][ki].dis) {
    //                 f[i][ki].dis = new_dis;
    //                 f[i][ki].path = f[j][kj].path;
    //                 f[i][ki].path.push_back(ki);
    //             }
    //         }
    //     }
    // }

    // DP ans = {1e9, {}};
    // for (int i = 0; i <= n - 1; i++) {
    //     if (f[(1 << n) - 1][i].dis < ans.dis) {
    //         ans = f[(1 << n) - 1][i];
    //     }
    // }

    std::vector<int> path = /*ans.path*/SolveTSP(point);
    //assert(path.size() == n);
    std::vector<QPoint> res;
    for (int i = 0; i < n; i++) {
        res.push_back(point[path[i]]);
    }


    assert(point.size() == n);
    //qDebug() << "point size: " << point.size() << "\n";
    point = res;

    QPainterPath path1 = generatePath(point);
    auto rpoint = point;
    std::reverse(rpoint.begin(), rpoint.end());
    QPainterPath path2 = generatePath(point);

    if (path2.length() < path1.length()) point = rpoint;

    // Bruteforce:

    // std::vector<int> p;
    // for (int i = 0; i < n; i++) p.push_back(i);
    // float mindis = 1e9;
    // std::vector<QPoint> res;
    // do {
    //     double dis = 0;
    //     for (int i = 1; i < n; i++) {
    //         QPointF d = (point[p[i]] - point[p[i - 1]]).toPointF();
    //         dis += sqrt(QPointF::dotProduct(d, d));
    //     }
    //     if (dis < mindis) {
    //         mindis = dis;
    //         res.clear();
    //         for (auto i : p) res.push_back(point[i]);
    //     }
    // } while (std::next_permutation(p.begin(), p.end()));
    // point = res;


}

void Widget::on_bthSortCurrent_clicked()
{

    if (!lines.empty()) {
        qDebug() << "Length before sort: " << lines.back().length() << "\n";
        sortByTSP(lines.back().point);
        qDebug() << "Length after sort: " << lines.back().length() << "\n";
    }
    update();
}


void Widget::on_bthSortAll_clicked()
{
    int cnt = 0;
    for (auto &line : lines) {
        sortByTSP(line.point);
        update();
        qDebug() << "OK " << ++cnt << "\n";
    }
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
    qDebug() << "largeAngle_clicked\n";
    update();
}


void Widget::on_bthTest_clicked()
{
    test_clicked = !test_clicked;
    qDebug() << "test_clicked is" << (test_clicked ? "on\n" : "off\n");
    update();
}

