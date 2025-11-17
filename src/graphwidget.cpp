#include "graphwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>
#include <algorithm>

GraphWidget::GraphWidget(QWidget* parent) : QWidget(parent), graph(nullptr), showTSP(false), tspCycle(false), selectedVertex(-1) {
    setMinimumSize(800, 600);
    setMouseTracking(true);
}

void GraphWidget::setGraph(const Graph* g) {
    graph = g;
    update();
}

void GraphWidget::setShortestPath(const std::vector<int>& path) {
    shortestPath = path;
    showTSP = false;
    update();
}

void GraphWidget::setTSPTour(const std::vector<int>& tour, bool cycle) {
    tspTour = tour;
    showTSP = true;
    tspCycle = cycle;
    update();
}

void GraphWidget::clearPaths() {
    shortestPath.clear();
    tspTour.clear();
    showTSP = false;
    update();
}

int GraphWidget::getVertexAt(const QPoint& pos) const {
    if (!graph || graph->names.empty()) return -1;
    
    QPointF screenPos = pos;
    double minDist = 1e10;
    int closest = -1;
    
    for (size_t i = 0; i < graph->names.size(); ++i) {
        QPointF vPos = worldToScreen(graph->coord[i]);
        double dist = std::sqrt(std::pow(screenPos.x() - vPos.x(), 2) + std::pow(screenPos.y() - vPos.y(), 2));
        if (dist < 20 && dist < minDist) {
            minDist = dist;
            closest = i;
        }
    }
    return closest;
}

void GraphWidget::setSelectedVertex(int v) {
    selectedVertex = v;
    update();
}

QRectF GraphWidget::getBoundingBox() const {
    if (!graph || graph->coord.empty()) return QRectF(0, 0, 200, 200);
    
    double minX = graph->coord[0].x, maxX = graph->coord[0].x;
    double minY = graph->coord[0].y, maxY = graph->coord[0].y;
    
    for (const auto& pt : graph->coord) {
        minX = std::min(minX, pt.x);
        maxX = std::max(maxX, pt.x);
        minY = std::min(minY, pt.y);
        maxY = std::max(maxY, pt.y);
    }
    // 包含原点，便于显示坐标轴
    minX = std::min(minX, 0.0);
    maxX = std::max(maxX, 0.0);
    minY = std::min(minY, 0.0);
    maxY = std::max(maxY, 0.0);
    
    double padding = 20;
    return QRectF(minX - padding, minY - padding, maxX - minX + 2 * padding, maxY - minY + 2 * padding);
}

QPointF GraphWidget::worldToScreen(const Pt& pt) const {
    QRectF bbox = getBoundingBox();
    double scaleX = (width() - 80) / (bbox.width() > 0 ? bbox.width() : 1);
    double scaleY = (height() - 80) / (bbox.height() > 0 ? bbox.height() : 1);
    double scale = std::min(scaleX, scaleY);
    double offsetX = 40 - bbox.left() * scale;
    double offsetY = 40 + bbox.bottom() * scale;
    return QPointF(pt.x * scale + offsetX, offsetY - pt.y * scale);
}

Pt GraphWidget::screenToWorld(const QPointF& pt) const {
    QRectF bbox = getBoundingBox();
    double scaleX = (width() - 80) / (bbox.width() > 0 ? bbox.width() : 1);
    double scaleY = (height() - 80) / (bbox.height() > 0 ? bbox.height() : 1);
    double scale = std::min(scaleX, scaleY);
    double offsetX = 40 - bbox.left() * scale;
    double offsetY = 40 + bbox.bottom() * scale;
    return Pt{(pt.x() - offsetX) / scale, (offsetY - pt.y()) / scale};
}

void GraphWidget::drawVertex(QPainter& painter, int v, const QPointF& pos, bool isSelected) {
    QColor color = isSelected ? QColor(255, 100, 100) : QColor(70, 130, 180);
    QColor borderColor = isSelected ? QColor(200, 0, 0) : QColor(25, 25, 112);
    
    painter.setPen(QPen(borderColor, 2));
    painter.setBrush(QBrush(color));
    painter.drawEllipse(pos, 15, 15);
    
    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPointSize(9);
    font.setBold(true);
    painter.setFont(font);
    QRectF textRect(pos.x() - 30, pos.y() - 35, 60, 20);
    QString text = QString::fromUtf8(graph->names[v].c_str());
    painter.drawText(textRect, Qt::AlignCenter, text);
}

void GraphWidget::drawEdge(QPainter& painter, int u, int v, const QPointF& posU, const QPointF& posV, bool highlight) {
    QPen pen;
    if (highlight) {
        pen = QPen(QColor(255, 0, 0), 3);
    } else {
        pen = QPen(QColor(150, 150, 150), 1);
    }
    painter.setPen(pen);
    painter.drawLine(posU, posV);
    
    // 显示距离
    if (graph && u < (int)graph->adj.size()) {
        for (const auto& e : graph->adj[u]) {
            if (e.to == v) {
                QPointF mid = (posU + posV) / 2;
                painter.setPen(Qt::darkGray);
                painter.setFont(QFont("Arial", 8));
                painter.drawText(mid, QString::number(e.w, 'f', 1) + "m");
                break;
            }
        }
    }
}

void GraphWidget::drawAxes(QPainter& painter) {
    if (!graph || graph->names.empty()) return;
    QRectF bbox = getBoundingBox();
    painter.save();
    QPen axisPen(QColor(160, 160, 160), 1, Qt::DashLine);
    painter.setPen(axisPen);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    
    // X轴
    if (bbox.top() <= 0 && bbox.bottom() >= 0) {
        QPointF start = worldToScreen(Pt{bbox.left(), 0});
        QPointF end = worldToScreen(Pt{bbox.right(), 0});
        painter.drawLine(start, end);
        painter.drawText(end + QPointF(-10, -5), "X");
    }
    // Y轴
    if (bbox.left() <= 0 && bbox.right() >= 0) {
        QPointF start = worldToScreen(Pt{0, bbox.top()});
        QPointF end = worldToScreen(Pt{0, bbox.bottom()});
        painter.drawLine(start, end);
        painter.drawText(start + QPointF(5, -400), "Y");
    }
    painter.restore();
}

void GraphWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 背景
    painter.fillRect(rect(), QColor(245, 245, 250));
    
    if (!graph || graph->names.empty()) {
        painter.setPen(Qt::gray);
        painter.setFont(QFont("Arial", 14));
        painter.drawText(rect(), Qt::AlignCenter, "请加载或创建校园图");
        return;
    }
    
    drawAxes(painter);
    
    // 绘制所有边
    for (size_t u = 0; u < graph->adj.size(); ++u) {
        QPointF posU = worldToScreen(graph->coord[u]);
        for (const auto& e : graph->adj[u]) {
            int v = e.to;
            if ((int)u < v) {  // 只画一次
                QPointF posV = worldToScreen(graph->coord[v]);
                bool highlight = false;
                
                // 检查是否在最短路径中
                if (!shortestPath.empty()) {
                    for (size_t i = 0; i + 1 < shortestPath.size(); ++i) {
                        if ((shortestPath[i] == (int)u && shortestPath[i + 1] == v) ||
                            (shortestPath[i] == v && shortestPath[i + 1] == (int)u)) {
                            highlight = true;
                            break;
                        }
                    }
                }
                
                // 检查是否在TSP路径中
                if (showTSP && !tspTour.empty()) {
                    for (size_t i = 0; i + 1 < tspTour.size(); ++i) {
                        if ((tspTour[i] == (int)u && tspTour[i + 1] == v) ||
                            (tspTour[i] == v && tspTour[i + 1] == (int)u)) {
                            highlight = true;
                            break;
                        }
                    }
                    if (tspCycle && tspTour.size() >= 2) {
                        int last = tspTour.back();
                        int first = tspTour.front();
                        if ((last == (int)u && first == v) || (last == v && first == (int)u)) {
                            highlight = true;
                        }
                    }
                }
                
                drawEdge(painter, u, v, posU, posV, highlight);
            }
        }
    }
    
    // 绘制所有顶点
    for (size_t i = 0; i < graph->names.size(); ++i) {
        QPointF pos = worldToScreen(graph->coord[i]);
        drawVertex(painter, static_cast<int>(i), pos, static_cast<int>(i) == selectedVertex);
    }
}

void GraphWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        int v = getVertexAt(event->pos());
        if (v >= 0) {
            emit vertexClicked(v);
        }
    }
}

