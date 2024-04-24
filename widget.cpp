#include "widget.h"
#include "./ui_widget.h"



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


void Widget::drawBezierCurve(Line &line) {
    if (line.point.empty()) return;


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
        drawBezierCurve(line);
    }
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
    int pointNumber = 7;
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

    int width = this->width();
    int height = this->height();

    // 生成随机的x和y坐标
    int x = QRandomGenerator::global()->bounded(width);
    int y = QRandomGenerator::global()->bounded(height);

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

DP f[1 << 21][21];

double dis(QPoint a, QPoint b) {
    QPoint t = a - b;
    return sqrt(QPointF::dotProduct(t, t));
}
int lowbit(int i) { return i & -i; }
void Widget::sortByTSP(std::vector<QPoint> &point) {
    int n = point.size();
    if (n <= 2) return;


    std::random_shuffle(point.begin(), point.end());

    for (int i = 0 ; i <= (1 << n) - 1; i++) {
        for (int j = 0; j <= n - 1; j++) {
            f[i][j].dis = 1e9;
            f[i][j].path.clear();
        }
    }
    // Dynamic Programming:
    f[0][0].dis = 0;
    for (int i = 1; i <= (1 << n) - 1; i++) {
        if (i == lowbit(i)) {
            int k = log2(i);
            f[i][k].dis = 0;
            f[i][k].path = {k};
            continue;
        }

        std::vector<int> v;
        for (int k = 0; k <= n - 1; k++) {
            if (i >> k & 1) v.push_back(k);
        }

        for (auto ki : v) {
            for (auto kj : v) {
                if (ki == kj) continue;
                int j = i - (1 << ki);
                double new_dis = f[j][kj].dis + dis(point[kj], point[ki]);
                if (new_dis < f[i][ki].dis) {
                    f[i][ki].dis = new_dis;
                    f[i][ki].path = f[j][kj].path;
                    f[i][ki].path.push_back(ki);
                }
            }
        }
    }

    DP ans = {1e9, {}};
    for (int i = 0; i <= n - 1; i++) {
        if (f[(1 << n) - 1][i].dis < ans.dis) {
            ans = f[(1 << n) - 1][i];
        }
    }

    std::vector<int> path = ans.path;
    assert(path.size() == n);
    std::vector<QPoint> res;
    for (int i = 0; i < n; i++) {
        res.push_back(point[path[i]]);
    }

    assert(point.size() == n);
    qDebug() << "point size: " << point.size() << "\n";
    point = res;

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

