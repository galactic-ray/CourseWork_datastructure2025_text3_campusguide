#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QFileDialog>
#include "graph.h"
#include "graphwidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadSample();
    void onAddVertex();
    void onAddEdge();
    void onQueryShortestPath();
    void onTSPTour();
    void onSaveGraph();
    void onLoadGraph();
    void onModifyVertex();
    void onVertexClicked(int vertexId);
    void onClearPaths();

private:
    void setupUI();
    void updateGraphDisplay();
    void showMessage(const QString& msg);
    
    Graph graph;
    GraphWidget* graphWidget;
    
    // UI组件
    QPushButton* btnLoadSample;
    QPushButton* btnAddVertex;
    QPushButton* btnAddEdge;
    QPushButton* btnQueryPath;
    QPushButton* btnTSP;
    QPushButton* btnSave;
    QPushButton* btnLoad;
    QPushButton* btnModifyVertex;
    QPushButton* btnClearPaths;
    
    QLineEdit* editVertexName;
    QLineEdit* editVertexX;
    QLineEdit* editVertexY;
    QSpinBox* spinEdgeU;
    QSpinBox* spinEdgeV;
    QSpinBox* spinPathStart;
    QSpinBox* spinPathEnd;
    QSpinBox* spinTSPStart;
    QCheckBox* checkTSPCycle;
    QSpinBox* spinModifyVertex;
    QLineEdit* editModifyX;
    QLineEdit* editModifyY;
    
    QTextEdit* textOutput;
};

#endif // MAINWINDOW_H

