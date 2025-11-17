#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include "graph.h"

class GraphWidget : public QWidget {
    Q_OBJECT

public:
    explicit GraphWidget(QWidget* parent = nullptr);
    void setGraph(const Graph* g);
    void setShortestPath(const std::vector<int>& path);
    void setTSPTour(const std::vector<int>& tour, bool cycle);
    void clearPaths();
    int getVertexAt(const QPoint& pos) const;
    void setSelectedVertex(int v);

signals:
    void vertexClicked(int vertexId);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    const Graph* graph;
    std::vector<int> shortestPath;
    std::vector<int> tspTour;
    bool showTSP;
    bool tspCycle;
    int selectedVertex;
    
    QPointF worldToScreen(const Pt& pt) const;
    Pt screenToWorld(const QPointF& pt) const;
    QRectF getBoundingBox() const;
    void drawVertex(QPainter& painter, int v, const QPointF& pos, bool isSelected = false);
    void drawEdge(QPainter& painter, int u, int v, const QPointF& posU, const QPointF& posV, bool highlight = false);
    void drawAxes(QPainter& painter);
};

#endif // GRAPHWIDGET_H

