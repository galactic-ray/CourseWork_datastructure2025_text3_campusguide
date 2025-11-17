#include "graph.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <queue>
#include <algorithm>
#include <cmath>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h>
#define mkdir(dir, mode) _mkdir(dir)
#endif

// Graph类实现
void Graph::clear() {
    names.clear();
    coord.clear();
    id.clear();
    adj.clear();
}

double Graph::dist(const Pt& a, const Pt& b) {
    double dx = a.x - b.x, dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

int Graph::addVertex(const std::string& name, double x, double y) {
    if (id.count(name)) {
        int k = id[name];
        coord[k] = {x, y};
        return k;
    }
    int idx = (int)names.size();
    names.push_back(name);
    coord.push_back({x, y});
    id[name] = idx;
    adj.push_back({});
    return idx;
}

void Graph::ensureVertex(int v) {
    while ((int)adj.size() <= v) {
        adj.push_back({});
        names.push_back(std::to_string((int)names.size()));
        coord.push_back({0, 0});
    }
}

void Graph::addUndirectedEdgeById(int u, int v) {
    if (u == v) return;
    ensureVertex(std::max(u, v));
    double w = dist(coord[u], coord[v]);
    adj[u].push_back({v, w});
    adj[v].push_back({u, w});
}

void Graph::recomputeAllEdgeWeights() {
    for (int u = 0; u < (int)adj.size(); ++u) {
        for (auto& e : adj[u]) {
            e.w = dist(coord[u], coord[e.to]);
        }
    }
}

bool Graph::renameVertex(int idx, const std::string& newName) {
    if (idx < 0 || idx >= (int)names.size()) return false;
    if (newName.empty()) return false;
    auto it = id.find(newName);
    if (it != id.end() && it->second != idx) return false;
    id.erase(names[idx]);
    names[idx] = newName;
    id[newName] = idx;
    return true;
}

bool Graph::removeVertex(int idx) {
    if (idx < 0 || idx >= (int)names.size()) return false;
    names.erase(names.begin() + idx);
    coord.erase(coord.begin() + idx);
    adj.erase(adj.begin() + idx);
    for (auto& edges : adj) {
        edges.erase(std::remove_if(edges.begin(), edges.end(),
                                   [&](const Edge& e) { return e.to == idx; }),
                    edges.end());
        for (auto& e : edges) {
            if (e.to > idx) --e.to;
        }
    }
    id.clear();
    for (size_t i = 0; i < names.size(); ++i) {
        id[names[i]] = static_cast<int>(i);
    }
    return true;
}

std::pair<double, std::vector<int>> Graph::dijkstra(int src, int dst) const {
    int n = (int)adj.size();
    std::vector<double> distv(n, INF);
    std::vector<int> parent(n, -1);
    using P = std::pair<double, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
    distv[src] = 0;
    pq.push({0, src});
    std::vector<char> vis(n, 0);
    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (vis[u]) continue;
        vis[u] = 1;
        if (u == dst) break;
        for (const auto& e : adj[u]) {
            int v = e.to;
            double nd = d + e.w;
            if (nd < distv[v]) {
                distv[v] = nd;
                parent[v] = u;
                pq.push({nd, v});
            }
        }
    }
    return {distv[dst], parent};
}

std::vector<int> Graph::rebuildPath(int src, int dst, const std::vector<int>& parent) const {
    std::vector<int> path;
    if (dst < 0) return path;
    for (int v = dst; v != -1; v = parent[v]) path.push_back(v);
    std::reverse(path.begin(), path.end());
    if (path.empty() || path.front() != src) return {};
    return path;
}

std::vector<std::vector<double>> Graph::allPairsShortest() const {
    int n = (int)adj.size();
    std::vector<std::vector<double>> D(n, std::vector<double>(n, INF));
    for (int s = 0; s < n; ++s) {
        std::vector<double> distv(n, INF);
        distv[s] = 0;
        using P = std::pair<double, int>;
        std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
        pq.push({0, s});
        std::vector<char> vis(n, 0);
        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();
            if (vis[u]) continue;
            vis[u] = 1;
            D[s][u] = d;
            for (const auto& e : adj[u]) {
                int v = e.to;
                double nd = d + e.w;
                if (nd < distv[v]) {
                    distv[v] = nd;
                    pq.push({nd, v});
                }
            }
        }
    }
    return D;
}

// TSP函数实现
double tourCost(const std::vector<int>& tour, const std::vector<std::vector<double>>& D, bool cycle) {
    double c = 0;
    for (size_t i = 0; i + 1 < tour.size(); ++i) c += D[tour[i]][tour[i + 1]];
    if (cycle && tour.size() >= 2) c += D[tour.back()][tour.front()];
    return c;
}

bool twoOptImprove(std::vector<int>& tour, const std::vector<std::vector<double>>& D, bool cycle) {
    int n = (int)tour.size();
    bool improved = false;
    auto edge = [&](int a, int b) { return D[tour[a]][tour[b]]; };
    int endI = cycle ? n - 1 : n - 2;
    for (int i = 1; i < endI; i++) {
        for (int j = i + 1; j < (cycle ? n : n - 1); j++) {
            int i1 = i - 1, i2 = i;
            int j1 = j, j2 = (j + 1 == n ? (cycle ? 0 : -1) : j + 1);
            if (!cycle && j2 == -1) continue;
            double before = edge(i1, i2) + edge(j1, j2);
            double after = edge(i1, j1) + edge(i2, j2);
            if (after + 1e-9 < before) {
                std::reverse(tour.begin() + i, tour.begin() + j + 1);
                improved = true;
            }
        }
    }
    return improved;
}

std::pair<double, std::vector<int>> tspNearest2Opt(const std::vector<std::vector<double>>& D, int start, bool cycle, int maxIter) {
    int n = (int)D.size();
    std::vector<int> unvis;
    unvis.reserve(n - 1);
    for (int i = 0; i < n; i++)
        if (i != start) unvis.push_back(i);
    std::vector<int> tour;
    tour.push_back(start);
    int cur = start;
    while (!unvis.empty()) {
        int bestK = -1;
        double best = Graph::INF;
        for (int k = 0; k < (int)unvis.size(); ++k) {
            int v = unvis[k];
            double w = D[cur][v];
            if (w < best) {
                best = w;
                bestK = k;
            }
        }
        int v = unvis[bestK];
        unvis.erase(unvis.begin() + bestK);
        tour.push_back(v);
        cur = v;
    }
    int it = 0;
    while (it++ < maxIter && twoOptImprove(tour, D, cycle)) {
    }
    double cost = tourCost(tour, D, cycle);
    return {cost, tour};
}

std::pair<double, std::vector<int>> tspHeldKarp(const std::vector<std::vector<double>>& D, int start, bool cycle) {
    int n = (int)D.size();
    std::vector<int> idx;
    for (int i = 0; i < n; i++)
        if (i != start) idx.push_back(i);
    int m = (int)idx.size();
    if (m == 0) return {0.0, std::vector<int>{start}};

    int FULL = 1 << m;
    const double INF2 = 1e100;
    std::vector<std::vector<double>> dp(FULL, std::vector<double>(m, INF2));
    std::vector<std::vector<int>> pre(FULL, std::vector<int>(m, -1));

    for (int i = 0; i < m; i++) dp[1 << i][i] = D[start][idx[i]];

    for (int mask = 1; mask < FULL; ++mask) {
        for (int i = 0; i < m; i++)
            if (mask & (1 << i)) {
                double cur = dp[mask][i];
                if (cur >= INF2) continue;
                for (int j = 0; j < m; j++)
                    if (!(mask & (1 << j))) {
                        int nmask = mask | (1 << j);
                        double nd = cur + D[idx[i]][idx[j]];
                        if (nd < dp[nmask][j]) {
                            dp[nmask][j] = nd;
                            pre[nmask][j] = i;
                        }
                    }
            }
    }

    double best = INF2;
    int bestEnd = -1;
    int mask = FULL - 1;
    for (int i = 0; i < m; i++) {
        double val = dp[mask][i] + (cycle ? D[idx[i]][start] : 0);
        if (val < best) {
            best = val;
            bestEnd = i;
        }
    }
    std::vector<int> tour;
    tour.push_back(start);
    std::vector<int> seq;
    int cur = bestEnd;
    int cmask = mask;
    while (cur != -1) {
        seq.push_back(idx[cur]);
        int p = pre[cmask][cur];
        cmask ^= (1 << cur);
        cur = p;
    }
    std::reverse(seq.begin(), seq.end());
    for (int v : seq) tour.push_back(v);
    return {best, tour};
}

std::pair<double, std::vector<int>> tspAuto(const std::vector<std::vector<double>>& D, int start, bool cycle) {
    int n = (int)D.size();
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (i != j && D[i][j] >= Graph::INF / 2) return {Graph::INF, {}};
    if (n <= 20) return tspHeldKarp(D, start, cycle);
    return tspNearest2Opt(D, start, cycle);
}

// CSV操作函数
static inline std::string csvEscape(const std::string& s) {
    bool need = false;
    for (char c : s) {
        if (c == '"' || c == ',' || c == '\n' || c == '\r') {
            need = true;
            break;
        }
    }
    if (!need) return s;
    std::string out = "\"";
    for (char c : s) {
        out += (c == '"') ? "\"\"" : std::string(1, c);
    }
    out += "\"";
    return out;
}

static std::vector<std::string> parseCsvLine(const std::string& line) {
    std::vector<std::string> f;
    std::string cur;
    bool inq = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inq) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    cur.push_back('"');
                    ++i;
                } else
                    inq = false;
            } else
                cur.push_back(c);
        } else {
            if (c == ',') {
                f.push_back(cur);
                cur.clear();
            } else if (c == '"')
                inq = true;
            else if (c == '\r') {
            } else
                cur.push_back(c);
        }
    }
    f.push_back(cur);
    // 去BOM
    if (!f.empty() && !f[0].empty() && (unsigned char)f[0][0] == 0xEF) {
        if (f[0].size() >= 3 && (unsigned char)f[0][1] == 0xBB && (unsigned char)f[0][2] == 0xBF) {
            f[0] = f[0].substr(3);
        }
    }
    return f;
}

// 获取数据文件路径的辅助函数
static std::string getDataPath(const std::string& filename) {
    // 尝试从data目录加载
    std::string dataPath = "data/" + filename;
    std::ifstream test(dataPath);
    if (test.good()) {
        test.close();
        return dataPath;
    }
    // 如果data目录不存在，尝试当前目录（向后兼容）
    return filename;
}

static const std::string FILE_NEW = "graph_xy.csv";
static const std::string FILE_OLD = "graph.csv";

size_t countEdges(const Graph& g) {
    size_t s = 0;
    for (const auto& a : g.adj) s += a.size();
    return s / 2;
}

bool saveGraphCSV(const Graph& g, const std::string& file) {
    std::string filepath = file;
    if (filepath.empty()) {
        // 默认保存到data目录，如果目录不存在则创建
        struct stat info;
        if (stat("data", &info) != 0) {
            // data目录不存在，创建它
            mkdir("data", 0755);
        }
        filepath = "data/graph_xy.csv";
    }
    std::ofstream out(filepath, std::ios::trunc);
    if (!out) return false;
    out << "kind,id,name,x,y,u,v\n";
    out.setf(std::ios::fixed);
    out << std::setprecision(3);
    // 顶点
    for (size_t i = 0; i < g.names.size(); ++i) {
        out << "V," << i << "," << csvEscape(g.names[i]) << ","
            << g.coord[i].x << "," << g.coord[i].y << ",,\n";
    }
    // 边：只写 u<v
    for (size_t u = 0; u < g.adj.size(); ++u) {
        for (const auto& e : g.adj[u]) {
            if ((int)u < e.to) {
                out << "E,,,,," << u << "," << e.to << "\n";
            }
        }
    }
    return true;
}

bool loadGraphCSV_core(Graph& g, const std::string& file, bool isOldFormat) {
    std::ifstream in(file);
    if (!in) return false;
    g.clear();
    std::string line;
    if (!getline(in, line)) return false;  // 跳过表头

    struct ERow {
        int u, v;
    };
    std::vector<ERow> edges;

    while (getline(in, line)) {
        if (line.empty()) continue;
        auto f = parseCsvLine(line);
        if (f.empty()) continue;
        std::string kind = f[0];
        if (kind == "V") {
            if (isOldFormat) {
                // 旧版：V,u,,,name
                if (f.size() < 5) continue;
                int idv = std::stoi(f[1]);
                std::string name = f[4];
                if (idv < 0) continue;
                g.ensureVertex(idv);
                g.names[idv] = name;
                g.coord[idv] = {0, 0};
                g.id[name] = idv;
            } else {
                // 新版：V,id,name,x,y,,
                if (f.size() < 5) continue;
                int idv = std::stoi(f[1]);
                std::string name = f[2];
                double x = 0, y = 0;
                try {
                    x = f[3].empty() ? 0.0 : std::stod(f[3]);
                    y = f[4].empty() ? 0.0 : std::stod(f[4]);
                } catch (...) {
                    x = 0;
                    y = 0;
                }
                if (idv < 0) continue;
                g.ensureVertex(idv);
                g.names[idv] = name;
                g.coord[idv] = {x, y};
                g.id[name] = idv;
            }
        } else if (kind == "E") {
            if (isOldFormat) {
                // 旧版：E,u,v,w,,
                if (f.size() < 3) continue;
                int u = -1, v = -1;
                try {
                    u = std::stoi(f[1]);
                    v = std::stoi(f[2]);
                } catch (...) {
                    continue;
                }
                if (u >= 0 && v >= 0) edges.push_back({u, v});
            } else {
                // 新版：E,,,,,u,v
                if (f.size() < 7) continue;
                int u = -1, v = -1;
                try {
                    u = std::stoi(f[5]);
                    v = std::stoi(f[6]);
                } catch (...) {
                    continue;
                }
                if (u >= 0 && v >= 0) edges.push_back({u, v});
            }
        }
    }
    // 添边（按坐标算权重）
    for (const auto& e : edges) {
        if (e.u < (int)g.names.size() && e.v < (int)g.names.size())
            g.addUndirectedEdgeById(e.u, e.v);
    }
    return true;
}

bool loadGraphCSV(Graph& g) {
    // 优先新文件，其次兼容旧文件
    // 先尝试data目录，再尝试当前目录
    if (loadGraphCSV_core(g, getDataPath(FILE_NEW), false)) return true;
    if (loadGraphCSV_core(g, FILE_NEW, false)) return true;
    if (loadGraphCSV_core(g, getDataPath(FILE_OLD), true)) return true;
    if (loadGraphCSV_core(g, FILE_OLD, true)) return true;
    return false;
}

void loadSample(Graph& g) {
    g.clear();
    std::vector<std::pair<std::string, Pt>> vs = {
        {"东门", {0, 0}}, {"图书馆", {60, 5}}, {"教学楼A", {65, 40}},
        {"教学楼B", {95, 45}}, {"实验楼", {120, 50}}, {"食堂", {120, 10}},
        {"体育馆", {150, 20}}, {"宿舍区", {130, -30}}, {"行政楼", {90, 5}},
        {"医务室", {100, -25}}, {"西门", {200, 0}}, {"南门", {120, -60}}};
    std::vector<int> vid(vs.size());
    for (size_t i = 0; i < vs.size(); ++i)
        vid[i] = g.addVertex(vs[i].first, vs[i].second.x, vs[i].second.y);

    auto E = [&](int a, int b) { g.addUndirectedEdgeById(vid[a], vid[b]); };
    E(0, 1);
    E(0, 2);
    E(1, 2);
    E(1, 8);
    E(8, 9);
    E(8, 3);
    E(2, 3);
    E(3, 4);
    E(4, 5);
    E(5, 7);
    E(7, 6);
    E(6, 10);
    E(10, 11);
    E(11, 5);
    E(1, 5);
    E(8, 5);
    E(2, 5);
    E(0, 11);
    E(7, 10);
    E(9, 11);
}

