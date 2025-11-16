#include "mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <sstream>
#include <iomanip>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("校园导游咨询系统");
    setMinimumSize(1200, 800);
    
    // 尝试加载已有数据
    if (loadGraphCSV(graph)) {
        showMessage(QString("已自动加载 %1 个景点，%2 条道路")
                    .arg(graph.names.size())
                    .arg(countEdges(graph)));
    }
    
    setupUI();
    updateGraphDisplay();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    
    // 左侧：图形显示区域
    graphWidget = new GraphWidget(this);
    graphWidget->setGraph(&graph);
    connect(graphWidget, &GraphWidget::vertexClicked, this, &MainWindow::onVertexClicked);
    mainLayout->addWidget(graphWidget, 2);
    
    // 右侧：控制面板
    QWidget* controlPanel = new QWidget(this);
    QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);
    
    // 文件操作
    QGroupBox* fileGroup = new QGroupBox("文件操作", this);
    QVBoxLayout* fileLayout = new QVBoxLayout(fileGroup);
    btnLoadSample = new QPushButton("加载示例图", this);
    btnLoad = new QPushButton("从CSV加载", this);
    btnSave = new QPushButton("保存到CSV", this);
    fileLayout->addWidget(btnLoadSample);
    fileLayout->addWidget(btnLoad);
    fileLayout->addWidget(btnSave);
    connect(btnLoadSample, &QPushButton::clicked, this, &MainWindow::onLoadSample);
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::onLoadGraph);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::onSaveGraph);
    
    // 景点操作
    QGroupBox* vertexGroup = new QGroupBox("景点操作", this);
    QVBoxLayout* vertexLayout = new QVBoxLayout(vertexGroup);
    editVertexName = new QLineEdit(this);
    editVertexName->setPlaceholderText("景点名称");
    editVertexX = new QLineEdit(this);
    editVertexX->setPlaceholderText("X坐标(米)");
    editVertexY = new QLineEdit(this);
    editVertexY->setPlaceholderText("Y坐标(米)");
    btnAddVertex = new QPushButton("添加景点", this);
    vertexLayout->addWidget(new QLabel("名称:"));
    vertexLayout->addWidget(editVertexName);
    vertexLayout->addWidget(new QLabel("X坐标:"));
    vertexLayout->addWidget(editVertexX);
    vertexLayout->addWidget(new QLabel("Y坐标:"));
    vertexLayout->addWidget(editVertexY);
    vertexLayout->addWidget(btnAddVertex);
    connect(btnAddVertex, &QPushButton::clicked, this, &MainWindow::onAddVertex);
    
    // 修改景点坐标
    QGroupBox* modifyGroup = new QGroupBox("修改景点坐标", this);
    QVBoxLayout* modifyLayout = new QVBoxLayout(modifyGroup);
    spinModifyVertex = new QSpinBox(this);
    spinModifyVertex->setMinimum(0);
    spinModifyVertex->setMaximum(100);
    editModifyX = new QLineEdit(this);
    editModifyX->setPlaceholderText("新X坐标");
    editModifyY = new QLineEdit(this);
    editModifyY->setPlaceholderText("新Y坐标");
    btnModifyVertex = new QPushButton("修改坐标", this);
    modifyLayout->addWidget(new QLabel("景点编号:"));
    modifyLayout->addWidget(spinModifyVertex);
    modifyLayout->addWidget(new QLabel("新X坐标:"));
    modifyLayout->addWidget(editModifyX);
    modifyLayout->addWidget(new QLabel("新Y坐标:"));
    modifyLayout->addWidget(editModifyY);
    modifyLayout->addWidget(btnModifyVertex);
    connect(btnModifyVertex, &QPushButton::clicked, this, &MainWindow::onModifyVertex);
    
    // 道路操作
    QGroupBox* edgeGroup = new QGroupBox("道路操作", this);
    QVBoxLayout* edgeLayout = new QVBoxLayout(edgeGroup);
    spinEdgeU = new QSpinBox(this);
    spinEdgeU->setMinimum(0);
    spinEdgeU->setMaximum(100);
    spinEdgeV = new QSpinBox(this);
    spinEdgeV->setMinimum(0);
    spinEdgeV->setMaximum(100);
    btnAddEdge = new QPushButton("添加道路", this);
    edgeLayout->addWidget(new QLabel("起点编号:"));
    edgeLayout->addWidget(spinEdgeU);
    edgeLayout->addWidget(new QLabel("终点编号:"));
    edgeLayout->addWidget(spinEdgeV);
    edgeLayout->addWidget(btnAddEdge);
    connect(btnAddEdge, &QPushButton::clicked, this, &MainWindow::onAddEdge);
    
    // 最短路径查询
    QGroupBox* pathGroup = new QGroupBox("最短路径查询", this);
    QVBoxLayout* pathLayout = new QVBoxLayout(pathGroup);
    spinPathStart = new QSpinBox(this);
    spinPathStart->setMinimum(0);
    spinPathStart->setMaximum(100);
    spinPathEnd = new QSpinBox(this);
    spinPathEnd->setMinimum(0);
    spinPathEnd->setMaximum(100);
    btnQueryPath = new QPushButton("查询最短路径", this);
    btnClearPaths = new QPushButton("清除路径显示", this);
    pathLayout->addWidget(new QLabel("起点编号:"));
    pathLayout->addWidget(spinPathStart);
    pathLayout->addWidget(new QLabel("终点编号:"));
    pathLayout->addWidget(spinPathEnd);
    pathLayout->addWidget(btnQueryPath);
    pathLayout->addWidget(btnClearPaths);
    connect(btnQueryPath, &QPushButton::clicked, this, &MainWindow::onQueryShortestPath);
    connect(btnClearPaths, &QPushButton::clicked, this, &MainWindow::onClearPaths);
    
    // TSP遍历
    QGroupBox* tspGroup = new QGroupBox("遍历全部景点", this);
    QVBoxLayout* tspLayout = new QVBoxLayout(tspGroup);
    spinTSPStart = new QSpinBox(this);
    spinTSPStart->setMinimum(0);
    spinTSPStart->setMaximum(100);
    checkTSPCycle = new QCheckBox("是否成环", this);
    btnTSP = new QPushButton("计算最优遍历", this);
    tspLayout->addWidget(new QLabel("起点编号:"));
    tspLayout->addWidget(spinTSPStart);
    tspLayout->addWidget(checkTSPCycle);
    tspLayout->addWidget(btnTSP);
    connect(btnTSP, &QPushButton::clicked, this, &MainWindow::onTSPTour);
    
    // 输出信息
    QGroupBox* outputGroup = new QGroupBox("信息输出", this);
    QVBoxLayout* outputLayout = new QVBoxLayout(outputGroup);
    textOutput = new QTextEdit(this);
    textOutput->setReadOnly(true);
    textOutput->setMaximumHeight(150);
    outputLayout->addWidget(textOutput);
    
    // 组装控制面板
    controlLayout->addWidget(fileGroup);
    controlLayout->addWidget(vertexGroup);
    controlLayout->addWidget(modifyGroup);
    controlLayout->addWidget(edgeGroup);
    controlLayout->addWidget(pathGroup);
    controlLayout->addWidget(tspGroup);
    controlLayout->addWidget(outputGroup);
    controlLayout->addStretch();
    
    mainLayout->addWidget(controlPanel, 1);
    
    updateGraphDisplay();
}

void MainWindow::updateGraphDisplay() {
    graphWidget->setGraph(&graph);
    
    // 更新SpinBox的最大值
    int maxId = std::max(0, (int)graph.names.size() - 1);
    spinEdgeU->setMaximum(maxId);
    spinEdgeV->setMaximum(maxId);
    spinPathStart->setMaximum(maxId);
    spinPathEnd->setMaximum(maxId);
    spinTSPStart->setMaximum(maxId);
    spinModifyVertex->setMaximum(maxId);
}

void MainWindow::showMessage(const QString& msg) {
    textOutput->append(msg);
    textOutput->moveCursor(QTextCursor::End);
}

void MainWindow::onLoadSample() {
    loadSample(graph);
    updateGraphDisplay();
    showMessage(QString("已加载示例图：%1 个景点，%2 条道路")
                .arg(graph.names.size())
                .arg(countEdges(graph)));
    if (saveGraphCSV(graph)) {
        showMessage("已自动保存到 data/graph_xy.csv");
    }
}

void MainWindow::onAddVertex() {
    QString name = editVertexName->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入景点名称");
        return;
    }
    
    bool okX, okY;
    double x = editVertexX->text().toDouble(&okX);
    double y = editVertexY->text().toDouble(&okY);
    
    if (!okX || !okY) {
        QMessageBox::warning(this, "错误", "请输入有效的坐标值");
        return;
    }
    
    int id = graph.addVertex(name.toStdString(), x, y);
    updateGraphDisplay();
    showMessage(QString("已添加景点：%1 (编号 %2) @(%3, %4)")
                .arg(name).arg(id).arg(x, 0, 'f', 2).arg(y, 0, 'f', 2));
    
    editVertexName->clear();
    editVertexX->clear();
    editVertexY->clear();
    
    if (saveGraphCSV(graph)) {
        showMessage("已自动保存");
    }
}

void MainWindow::onAddEdge() {
    if (graph.names.empty()) {
        QMessageBox::warning(this, "错误", "请先添加景点");
        return;
    }
    
    int u = spinEdgeU->value();
    int v = spinEdgeV->value();
    
    if (u < 0 || v < 0 || u >= (int)graph.names.size() || v >= (int)graph.names.size()) {
        QMessageBox::warning(this, "错误", "编号不合法");
        return;
    }
    
    if (u == v) {
        QMessageBox::warning(this, "错误", "起点和终点不能相同");
        return;
    }
    
    graph.addUndirectedEdgeById(u, v);
    double w = Graph::dist(graph.coord[u], graph.coord[v]);
    updateGraphDisplay();
    showMessage(QString("已添加道路：%1 - %2，距离 %3 米")
                .arg(QString::fromStdString(graph.names[u]))
                .arg(QString::fromStdString(graph.names[v]))
                .arg(w, 0, 'f', 2));
    
    if (saveGraphCSV(graph)) {
        showMessage("已自动保存");
    }
}

void MainWindow::onQueryShortestPath() {
    if (graph.names.empty()) {
        QMessageBox::warning(this, "错误", "请先添加景点或加载示例图");
        return;
    }
    
    int s = spinPathStart->value();
    int t = spinPathEnd->value();
    
    if (s < 0 || t < 0 || s >= (int)graph.names.size() || t >= (int)graph.names.size()) {
        QMessageBox::warning(this, "错误", "编号不合法");
        return;
    }
    
    auto res = graph.dijkstra(s, t);
    double d = res.first;
    auto path = graph.rebuildPath(s, t, res.second);
    
    if (path.empty() || d >= Graph::INF / 2) {
        showMessage("不可达或无路径");
        graphWidget->clearPaths();
    } else {
        graphWidget->setShortestPath(path);
        QString pathStr;
        for (size_t i = 0; i < path.size(); ++i) {
            if (i > 0) pathStr += " → ";
            pathStr += QString::fromStdString(graph.names[path[i]]);
        }
        showMessage(QString("最短距离: %1 米\n路径: %2")
                    .arg(d, 0, 'f', 2)
                    .arg(pathStr));
    }
}

void MainWindow::onTSPTour() {
    if (graph.names.empty()) {
        QMessageBox::warning(this, "错误", "请先添加景点或加载示例图");
        return;
    }
    
    int n = (int)graph.names.size();
    int s = spinTSPStart->value();
    
    if (s < 0 || s >= n) {
        QMessageBox::warning(this, "错误", "编号不合法");
        return;
    }
    
    bool cycle = checkTSPCycle->isChecked();
    auto D = graph.allPairsShortest();
    auto [cost, tour] = tspAuto(D, s, cycle);
    
    if (cost >= Graph::INF / 2) {
        showMessage("图不连通，无法覆盖全部景点");
        graphWidget->clearPaths();
    } else {
        graphWidget->setTSPTour(tour, cycle);
        QString tourStr;
        for (size_t i = 0; i < tour.size(); ++i) {
            if (i > 0) tourStr += " → ";
            tourStr += QString::fromStdString(graph.names[tour[i]]);
        }
        if (cycle && tour.size() >= 2) {
            tourStr += " → " + QString::fromStdString(graph.names[tour.front()]);
        }
        showMessage(QString("%1: %2 米\n路径: %3")
                    .arg(cycle ? "总距离(回路)" : "总距离")
                    .arg(cost, 0, 'f', 2)
                    .arg(tourStr));
    }
}

void MainWindow::onSaveGraph() {
    QString fileName = QFileDialog::getSaveFileName(this, "保存图形", "graph_xy.csv", "CSV Files (*.csv)");
    if (!fileName.isEmpty()) {
        if (saveGraphCSV(graph, fileName.toStdString())) {
            showMessage(QString("已保存到: %1").arg(fileName));
        } else {
            QMessageBox::critical(this, "错误", "保存失败");
        }
    }
}

void MainWindow::onLoadGraph() {
    QString fileName = QFileDialog::getOpenFileName(this, "加载图形", "", "CSV Files (*.csv)");
    if (!fileName.isEmpty()) {
        Graph newGraph;
        if (loadGraphCSV_core(newGraph, fileName.toStdString(), false) || 
            loadGraphCSV_core(newGraph, fileName.toStdString(), true)) {
            graph = newGraph;
            updateGraphDisplay();
            graphWidget->clearPaths();
            showMessage(QString("已加载: %1 个景点，%2 条道路")
                        .arg(graph.names.size())
                        .arg(countEdges(graph)));
        } else {
            QMessageBox::critical(this, "错误", "加载失败");
        }
    }
}

void MainWindow::onModifyVertex() {
    if (graph.names.empty()) {
        QMessageBox::warning(this, "错误", "请先添加景点或加载示例图");
        return;
    }
    
    int u = spinModifyVertex->value();
    if (u < 0 || u >= (int)graph.names.size()) {
        QMessageBox::warning(this, "错误", "编号不合法");
        return;
    }
    
    bool okX, okY;
    double x = editModifyX->text().toDouble(&okX);
    double y = editModifyY->text().toDouble(&okY);
    
    if (!okX || !okY) {
        QMessageBox::warning(this, "错误", "请输入有效的坐标值");
        return;
    }
    
    graph.coord[u] = {x, y};
    graph.recomputeAllEdgeWeights();
    updateGraphDisplay();
    showMessage(QString("已更新景点 %1 的坐标为 (%2, %3)，相关边权已重算")
                .arg(QString::fromStdString(graph.names[u]))
                .arg(x, 0, 'f', 2)
                .arg(y, 0, 'f', 2));
    
    editModifyX->clear();
    editModifyY->clear();
    
    if (saveGraphCSV(graph)) {
        showMessage("已自动保存");
    }
}

void MainWindow::onVertexClicked(int vertexId) {
    graphWidget->setSelectedVertex(vertexId);
    showMessage(QString("点击了景点: %1 (编号 %2)")
                .arg(QString::fromStdString(graph.names[vertexId]))
                .arg(vertexId));
    spinPathStart->setValue(vertexId);
    spinPathEnd->setValue(vertexId);
    spinTSPStart->setValue(vertexId);
    spinModifyVertex->setValue(vertexId);
}

void MainWindow::onClearPaths() {
    graphWidget->clearPaths();
    showMessage("已清除路径显示");
}

