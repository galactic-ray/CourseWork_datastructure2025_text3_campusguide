#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <string>
#include <unordered_map>
#include <utility>

struct Edge {
    int to;
    double w;
};

struct Pt {
    double x = 0;
    double y = 0;
};

class Graph {
public:
    static constexpr double INF = 1e100;

    std::vector<std::string> names;              // 顶点名称
    std::vector<Pt> coord;                       // 顶点坐标
    std::unordered_map<std::string, int> id;     // 名称→编号
    std::vector<std::vector<Edge>> adj;          // 邻接表（无向边，双向各存一条）

    void clear();
    static double dist(const Pt& a, const Pt& b);
    int addVertex(const std::string& name, double x, double y);
    void ensureVertex(int v);
    void addUndirectedEdgeById(int u, int v);
    void recomputeAllEdgeWeights();
    std::pair<double, std::vector<int>> dijkstra(int src, int dst) const;
    std::vector<int> rebuildPath(int src, int dst, const std::vector<int>& parent) const;
    std::vector<std::vector<double>> allPairsShortest() const;
};

// TSP相关函数
double tourCost(const std::vector<int>& tour, const std::vector<std::vector<double>>& D, bool cycle);
bool twoOptImprove(std::vector<int>& tour, const std::vector<std::vector<double>>& D, bool cycle);
std::pair<double, std::vector<int>> tspNearest2Opt(const std::vector<std::vector<double>>& D, int start, bool cycle, int maxIter = 2000);
std::pair<double, std::vector<int>> tspHeldKarp(const std::vector<std::vector<double>>& D, int start, bool cycle);
std::pair<double, std::vector<int>> tspAuto(const std::vector<std::vector<double>>& D, int start, bool cycle);

// CSV文件操作
bool saveGraphCSV(const Graph& g, const std::string& file = "");
bool loadGraphCSV(Graph& g);
bool loadGraphCSV_core(Graph& g, const std::string& file, bool isOldFormat);
void loadSample(Graph& g);
size_t countEdges(const Graph& g);

#endif // GRAPH_H

