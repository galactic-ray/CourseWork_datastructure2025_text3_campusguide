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
    
    double padding = 20;
    return QRectF(minX - padding, minY - padding, maxX - minX + 2 * padding, maxY - minY + 2 * padding);
}

QPointF GraphWidget::worldToScreen(const Pt& pt) const {
    QRectF bbox = getBoundingBox();
    double scaleX = (width() - 80) / (bbox.width() > 0 ? bbox.width() : 1);
    double scaleY = (height() - 80) / (bbox.height() > 0 ? bbox.height() : 1);
    double scale = std::min(scaleX, scaleY);
    QPointF offset(40 - bbox.left() * scale, 40 - bbox.top() * scale);
    return QPointF(pt.x * scale + offset.x(), pt.y * scale + offset.y());
}

Pt GraphWidget::screenToWorld(const QPointF& pt) const {
    QRectF bbox = getBoundingBox();
    double scaleX = (width() - 80) / (bbox.width() > 0 ? bbox.width() : 1);
    double scaleY = (height() - 80) / (bbox.height() > 0 ? bbox.height() : 1);
    double scale = std::min(scaleX, scaleY);
    QPointF offset(40 - bbox.left() * scale, 40 - bbox.top() * scale);
    return Pt{(pt.x() - offset.x()) / scale, (pt.y() - offset.y()) / scale};
}

void GraphWidget::drawVertex(QPainter& painter, int v, const QPointF& pos, bool isSelected) {
    QColor color = isSelected ? QColor(255, 100, 100) : QColor(70, 130, 180);
    QColor borderColor = isSelected ? QColor(200, 0, 0) : QColor(25, 25, 112);
    
    painter.setPen(QPen(borderColor, 2));
    painter.setBrush(QBrush(color));
    painter.drawEllipse(pos, 15, 15);
    
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    QRectF textRect(pos.x() - 30, pos.y() - 35, 60, 20);
    QString text = QString::fromStdString(graph->names[v]);
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
        drawVertex(painter, i, pos, i == selectedVertex);
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

