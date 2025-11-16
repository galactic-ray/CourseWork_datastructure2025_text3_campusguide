// 文件名：3.cpp
// 功能：校园导游（邻接表 + Dijkstra + TSP），改为“坐标驱动”权重：
//  - 每个景点(楼)有 x,y 坐标（米）
//  - 仅需声明哪些楼两两相连（可走的路径）；边长自动用欧氏距离
//  - CSV 单文件持久化：V 行存顶点(id,name,x,y)，E 行存边(u,v)
//  - 启动自动加载 graph_xy.csv；若不存在则尝试加载旧版 graph.csv（向下兼容）
//  - 菜单沿用原逻辑，增加“修改坐标”
#include <bits/stdc++.h>
#ifdef _WIN32
#include <windows.h>
#endif
using namespace std;

struct Edge { int to; double w; };
struct Pt { double x=0, y=0; };
const double INF = 1e100;

/*==================== 图结构：坐标 + 邻接表 ====================*/
struct Graph {
    vector<string> names;              // 顶点名称
    vector<Pt>     coord;              // 顶点坐标
    unordered_map<string,int> id;      // 名称→编号
    vector<vector<Edge>> adj;          // 邻接表（无向边，双向各存一条）

    void clear(){ names.clear(); coord.clear(); id.clear(); adj.clear(); }

    static double dist(const Pt& a, const Pt& b){
        double dx=a.x-b.x, dy=a.y-b.y;
        return std::sqrt(dx*dx+dy*dy);
    }

    int addVertex(const string& name, double x, double y){
        if(id.count(name)) {
            int k=id[name]; coord[k]={x,y}; return k;
        }
        int idx=(int)names.size();
        names.push_back(name);
        coord.push_back({x,y});
        id[name]=idx;
        adj.push_back({});
        return idx;
    }

    void ensureVertex(int v){
        while((int)adj.size()<=v){
            adj.push_back({});
            names.push_back(to_string((int)names.size()));
            coord.push_back({0,0});
        }
    }

    // 添加无向边：权重用坐标计算
    void addUndirectedEdgeById(int u, int v){
        if(u==v) return;
        ensureVertex(max(u,v));
        double w = dist(coord[u], coord[v]);
        adj[u].push_back({v,w});
        adj[v].push_back({u,w});
    }

    // 若坐标变动，重算所有边权
    void recomputeAllEdgeWeights(){
        for(int u=0; u<(int)adj.size(); ++u){
            for(auto& e : adj[u]){
                e.w = dist(coord[u], coord[e.to]);
            }
        }
    }

    void printAdj() const{
        cout << "\n[邻接表（距离单位：米）]" << endl;
        cout.setf(std::ios::fixed); cout<<setprecision(2);
        for(size_t u=0; u<adj.size(); ++u){
            cout << setw(2) << u << " " << names[u]
                 << " @(" << coord[u].x << "," << coord[u].y << ") : ";
            for(size_t k=0; k<adj[u].size(); ++k){
                int v=adj[u][k].to; double w=adj[u][k].w;
                cout << "(" << v << ", " << names[v] << ", " << w << "m)";
                if(k+1<adj[u].size()) cout << "  ";
            }
            cout << "\n";
        }
    }

    pair<double, vector<int>> dijkstra(int src, int dst) const{
        int n=(int)adj.size();
        vector<double> distv(n, INF);
        vector<int> parent(n,-1);
        using P=pair<double,int>;
        priority_queue<P, vector<P>, greater<P>> pq;
        distv[src]=0; pq.push({0,src});
        vector<char> vis(n,0);
        while(!pq.empty()){
            auto [d,u]=pq.top(); pq.pop();
            if(vis[u]) continue; vis[u]=1;
            if(u==dst) break;
            for(const auto& e: adj[u]){
                int v=e.to; double nd=d+e.w;
                if(nd<distv[v]){
                    distv[v]=nd; parent[v]=u; pq.push({nd,v});
                }
            }
        }
        return {distv[dst], parent};
    }

    vector<int> rebuildPath(int src, int dst, const vector<int>& parent) const{
        vector<int> path; if(dst<0) return path;
        for(int v=dst; v!=-1; v=parent[v]) path.push_back(v);
        reverse(path.begin(), path.end());
        if(path.empty() || path.front()!=src) return {}; // 不可达
        return path;
    }

    vector<vector<double>> allPairsShortest() const{
        int n=(int)adj.size();
        vector<vector<double>> D(n, vector<double>(n, INF));
        for(int s=0; s<n; ++s){
            vector<double> distv(n, INF); distv[s]=0;
            using P=pair<double,int>;
            priority_queue<P, vector<P>, greater<P>> pq; pq.push({0,s});
            vector<char> vis(n,0);
            while(!pq.empty()){
                auto [d,u]=pq.top(); pq.pop();
                if(vis[u]) continue; vis[u]=1; D[s][u]=d;
                for(const auto &e: adj[u]){
                    int v=e.to; double nd=d+e.w;
                    if(nd<distv[v]){ distv[v]=nd; pq.push({nd,v}); }
                }
            }
        }
        return D;
    }
};

/*==================== TSP（沿可走路径的最短总距离） ====================*/
static double tourCost(const vector<int>& tour, const vector<vector<double>>& D, bool cycle){
    double c=0; for(size_t i=0;i+1<tour.size();++i) c += D[tour[i]][tour[i+1]];
    if(cycle && tour.size()>=2) c += D[tour.back()][tour.front()];
    return c;
}
static bool twoOptImprove(vector<int>& tour, const vector<vector<double>>& D, bool cycle){
    int n=(int)tour.size(); bool improved=false;
    auto edge=[&](int a,int b){ return D[tour[a]][tour[b]]; };
    int endI = cycle? n-1 : n-2;
    for(int i=1;i<endI;i++){
        for(int j=i+1;j<(cycle? n : n-1);j++){
            int i1=i-1,i2=i; int j1=j,j2=(j+1==n? (cycle?0:-1): j+1);
            if(!cycle && j2==-1) continue;
            double before=edge(i1,i2)+edge(j1,j2);
            double after =edge(i1,j1)+edge(i2,j2);
            if(after+1e-9<before){ reverse(tour.begin()+i,tour.begin()+j+1); improved=true; }
        }
    }
    return improved;
}
static pair<double, vector<int>> tspNearest2Opt(const vector<vector<double>>& D, int start, bool cycle, int maxIter=2000){
    int n=(int)D.size();
    vector<int> unvis; unvis.reserve(n-1);
    for(int i=0;i<n;i++) if(i!=start) unvis.push_back(i);
    vector<int> tour; tour.push_back(start);
    int cur=start;
    while(!unvis.empty()){
        int bestK=-1; double best=INF;
        for(int k=0;k<(int)unvis.size();++k){
            int v=unvis[k]; double w=D[cur][v];
            if(w<best){ best=w; bestK=k; }
        }
        int v=unvis[bestK]; unvis.erase(unvis.begin()+bestK);
        tour.push_back(v); cur=v;
    }
    int it=0; while(it++<maxIter && twoOptImprove(tour,D,cycle)){}
    double cost=tourCost(tour,D,cycle);
    return {cost,tour};
}
static pair<double, vector<int>> tspHeldKarp(const vector<vector<double>>& D, int start, bool cycle){
    int n=(int)D.size();
    vector<int> idx; for(int i=0;i<n;i++) if(i!=start) idx.push_back(i);
    int m=(int)idx.size();
    if(m==0) return {0.0, vector<int>{start}};

    int FULL=1<<m; const double INF2=1e100;
    vector<vector<double>> dp(FULL, vector<double>(m, INF2));
    vector<vector<int>> pre(FULL, vector<int>(m,-1));

    for(int i=0;i<m;i++) dp[1<<i][i]=D[start][idx[i]];

    for(int mask=1; mask<FULL; ++mask){
        for(int i=0;i<m;i++) if(mask&(1<<i)){
            double cur=dp[mask][i]; if(cur>=INF2) continue;
            for(int j=0;j<m;j++) if(!(mask&(1<<j))){
                int nmask=mask|(1<<j);
                double nd=cur + D[idx[i]][idx[j]];
                if(nd<dp[nmask][j]){ dp[nmask][j]=nd; pre[nmask][j]=i; }
            }
        }
    }

    double best=INF2; int bestEnd=-1; int mask=FULL-1;
    for(int i=0;i<m;i++){
        double val=dp[mask][i] + (cycle? D[idx[i]][start] : 0);
        if(val<best){ best=val; bestEnd=i; }
    }
    vector<int> tour; tour.push_back(start);
    vector<int> seq; int cur=bestEnd; int cmask=mask;
    while(cur!=-1){ seq.push_back(idx[cur]); int p=pre[cmask][cur]; cmask^=(1<<cur); cur=p; }
    reverse(seq.begin(),seq.end());
    for(int v:seq) tour.push_back(v);
    return {best,tour};
}
static pair<double, vector<int>> tspAuto(const vector<vector<double>>& D, int start, bool cycle){
    int n=(int)D.size();
    for(int i=0;i<n;i++) for(int j=0;j<n;j++) if(i!=j && D[i][j]>=INF/2) return {INF,{}};
    if(n<=20) return tspHeldKarp(D,start,cycle);
    return tspNearest2Opt(D,start,cycle);
}

/*==================== CSV：单文件（含坐标） ====================*/
/*
  文件：graph_xy.csv
  表头：kind,id,name,x,y,u,v
  - 顶点 V 行：  V,<id>,<name>,<x>,<y>,,
  - 边   E 行：  E,,,,,<u>,<v>
  兼容旧版 graph.csv（V 行无坐标；E 行可能带 w），加载时自动处理。
*/
static inline string csvEscape(const string& s){
    bool need=false;
    for(char c:s){ if(c=='"'||c==','||c=='\n'||c=='\r'){ need=true; break; } }
    if(!need) return s;
    string out="\"";
    for(char c:s){ out += (c=='"')? "\"\"" : string(1,c); }
    out += "\"";
    return out;
}
static vector<string> parseCsvLine(const string& line){
    vector<string> f; string cur; bool inq=false;
    for(size_t i=0;i<line.size();++i){
        char c=line[i];
        if(inq){
            if(c=='"'){ if(i+1<line.size() && line[i+1]=='"'){ cur.push_back('"'); ++i; } else inq=false; }
            else cur.push_back(c);
        }else{
            if(c==','){ f.push_back(cur); cur.clear(); }
            else if(c=='"'){ inq=true; }
            else if(c=='\r'){ }
            else cur.push_back(c);
        }
    }
    f.push_back(cur);
    // 去BOM
    if(!f.empty() && !f[0].empty() && (unsigned char)f[0][0]==0xEF){
        if(f[0].size()>=3 && (unsigned char)f[0][1]==0xBB && (unsigned char)f[0][2]==0xBF){
            f[0]=f[0].substr(3);
        }
    }
    return f;
}

static const string FILE_NEW = "graph_xy.csv";
static const string FILE_OLD = "graph.csv";

static size_t countEdges(const Graph& g){
    size_t s=0; for(const auto& a:g.adj) s+=a.size(); return s/2;
}

static bool saveGraphCSV(const Graph& g, const string& file=FILE_NEW){
    ofstream out(file, ios::trunc);
    if(!out) return false;
    out << "kind,id,name,x,y,u,v\n";
    out.setf(std::ios::fixed); out<<setprecision(3);
    // 顶点
    for(size_t i=0;i<g.names.size();++i){
        out << "V," << i << "," << csvEscape(g.names[i]) << ","
            << g.coord[i].x << "," << g.coord[i].y << ",,\n";
    }
    // 边：只写 u<v
    for(size_t u=0; u<g.adj.size(); ++u){
        for(const auto& e : g.adj[u]){
            if((int)u < e.to){
                out << "E,,,,," << u << "," << e.to << "\n";
            }
        }
    }
    return true;
}

static bool loadGraphCSV_core(Graph& g, const string& file, bool isOldFormat){
    ifstream in(file);
    if(!in) return false;
    g.clear();
    string line;
    if(!getline(in,line)) return false; // 跳过表头

    struct ERow{ int u,v; };
    vector<ERow> edges;

    while(getline(in,line)){
        if(line.empty()) continue;
        auto f=parseCsvLine(line);
        if(f.empty()) continue;
        string kind=f[0];
        if(kind=="V"){
            if(isOldFormat){
                // 旧版：V,u,,,name
                // idx=1=id, idx=4=name；坐标用0,0
                if(f.size()<5) continue;
                int idv = stoi(f[1]);
                string name = f[4];
                if(idv<0) continue;
                g.ensureVertex(idv);
                g.names[idv]=name;
                g.coord[idv]={0,0};
                g.id[name]=idv;
            }else{
                // 新版：V,id,name,x,y,,
                if(f.size()<5) continue;
                int idv = stoi(f[1]);
                string name = f[2];
                double x=0,y=0;
                try{
                    x = f[3].empty()? 0.0 : stod(f[3]);
                    y = f[4].empty()? 0.0 : stod(f[4]);
                }catch(...){ x=0;y=0; }
                if(idv<0) continue;
                g.ensureVertex(idv);
                g.names[idv]=name;
                g.coord[idv]={x,y};
                g.id[name]=idv;
            }
        }else if(kind=="E"){
            if(isOldFormat){
                // 旧版：E,u,v,w,,
                if(f.size()<3) continue;
                int u=-1,v=-1;
                try{ u=stoi(f[1]); v=stoi(f[2]); }catch(...){ continue; }
                if(u>=0 && v>=0) edges.push_back({u,v});
            }else{
                // 新版：E,,,,,u,v
                if(f.size()<7) continue;
                int u=-1,v=-1;
                try{ u=stoi(f[5]); v=stoi(f[6]); }catch(...){ continue; }
                if(u>=0 && v>=0) edges.push_back({u,v});
            }
        }
    }
    // 添边（按坐标算权重）
    for(const auto& e : edges){
        if(e.u<(int)g.names.size() && e.v<(int)g.names.size())
            g.addUndirectedEdgeById(e.u, e.v);
    }
    return true;
}

static bool loadGraphCSV(Graph& g){
    // 优先新文件，其次兼容旧文件
    if(loadGraphCSV_core(g, FILE_NEW, false)) return true;
    if(loadGraphCSV_core(g, FILE_OLD, true))  return true;
    return false;
}

/*==================== 示例图（附坐标） ====================*/
// 坐标单位米：简单布点，和原示例点名一致
static void loadSample(Graph& g){
    g.clear();
    vector<pair<string, Pt>> vs = {
        {"东门",{  0,  0}}, {"图书馆",{ 60,  5}}, {"教学楼A",{ 65, 40}},
        {"教学楼B",{ 95, 45}}, {"实验楼",{120, 50}}, {"食堂",{120, 10}},
        {"体育馆",{150, 20}}, {"宿舍区",{130,-30}}, {"行政楼",{ 90,  5}},
        {"医务室",{100,-25}}, {"西门",{200,  0}}, {"南门",{120,-60}}
    };
    vector<int> vid(vs.size());
    for(size_t i=0;i<vs.size();++i)
        vid[i]=g.addVertex(vs[i].first, vs[i].second.x, vs[i].second.y);

    auto E=[&](int a,int b){ g.addUndirectedEdgeById(vid[a],vid[b]); };
    E(0,1); E(0,2); E(1,2); E(1,8); E(8,9); E(8,3); E(2,3);
    E(3,4); E(4,5); E(5,7); E(7,6); E(6,10); E(10,11); E(11,5);
    E(1,5); E(8,5); E(2,5); E(0,11); E(7,10); E(9,11);
}

/*==================== 交互与菜单 ====================*/
static void printPath(const Graph& g, const vector<int>& path){
    for(size_t i=0;i<path.size();++i){
        cout << g.names[path[i]];
        if(i+1<path.size()) cout << " → ";
    }
    cout << "\n";
}

static void menu(){
    Graph g;
    ios::sync_with_stdio(false);
    cin.tie(&cout);

    // 启动尝试自动加载
    if(loadGraphCSV(g)){
        cout << "[自动加载] 已读取 "
             << g.names.size() << " 个景点，"
             << countEdges(g) << " 条道路。\n";
    }else{
        cout << "[提示] 未发现 " << FILE_NEW << "（或旧版 " << FILE_OLD << "），当前为空图。\n";
    }

    while(true){
        cout << "\n===== 校园导游（带坐标）=====\n";
        cout << "1. 加载示例校园图（含坐标）\n";
        cout << "2. 添加景点（名称 + x + y，单位米）\n";
        cout << "3. 添加道路（无向，自动按坐标计算距离）\n";
        cout << "4. 查询两点最短距离（Dijkstra）\n";
        cout << "5. 遍历全部景点（总距离最短：自动精确/启发式）\n";
        cout << "6. 打印邻接表\n";
        cout << "7. 保存到 CSV\n";
        cout << "8. 从 CSV 加载\n";
        cout << "9. 修改某景点坐标（并重算所有边）\n";
        cout << "0. 退出\n";
        cout << "选择: " << flush;

        int op; if(!(cin>>op)) return;
        if(op==0) break;

        if(op==1){
            loadSample(g);
            cout << "示例图已加载（" << g.names.size() << " 个景点，"
                 << countEdges(g) << " 条道路）。\n";
            if(saveGraphCSV(g)) cout << "[已保存] " << FILE_NEW << "\n";
        }
        else if(op==2){
            cout << "输入 名称: "; string name; cin>>ws; getline(cin,name);
            cout << "输入 坐标x(米) y(米): "; double x,y; cin>>x>>y;
            int idv = g.addVertex(name,x,y);
            cout.setf(std::ios::fixed); cout<<setprecision(2);
            cout << "已添加/更新：" << name << " (编号 " << idv
                 << ") @" << "("<<x<<","<<y<<")\n";
            if(saveGraphCSV(g)) cout << "[已保存] " << FILE_NEW << "\n";
        }
        else if(op==3){
            if(g.names.empty()){ cout << "请先添加景点或加载示例图。\n"; continue; }
            cout << "输入 两端编号 u v："; int u,v; cin>>u>>v;
            if(u<0||v<0||u>=(int)g.names.size()||v>=(int)g.names.size()||u==v){
                cout << "编号不合法。\n"; continue;
            }
            g.addUndirectedEdgeById(u,v);
            double w = Graph::dist(g.coord[u], g.coord[v]);
            cout.setf(std::ios::fixed); cout<<setprecision(2);
            cout << "已添加无向边：" << g.names[u] << " - " << g.names[v]
                 << "，距离 " << w << " m\n";
            if(saveGraphCSV(g)) cout << "[已保存] " << FILE_NEW << "\n";
        }
        else if(op==4){
            if(g.names.empty()){ cout << "请先添加景点或加载示例图。\n"; continue; }
            cout << "起点编号 终点编号："; int s,t; cin>>s>>t;
            if(s<0||t<0||s>=(int)g.names.size()||t>=(int)g.names.size()){
                cout<<"编号不合法。\n"; continue;
            }
            auto res = g.dijkstra(s,t);
            double d = res.first; auto path = g.rebuildPath(s,t,res.second);
            if(path.empty() || d>=INF/2){ cout << "不可达或无路径。\n"; }
            else{
                cout.setf(std::ios::fixed); cout<<setprecision(2);
                cout << "最短距离: " << d << " m\n路径: ";
                printPath(g,path);
            }
        }
        else if(op==5){
            if(g.names.empty()){ cout << "请先添加景点或加载示例图。\n"; continue; }
            int n=(int)g.names.size();
            cout << "输入起点编号 (0-"<<n-1<<")："; int s; cin>>s;
            if(s<0||s>=n){ cout<<"编号不合法。\n"; continue; }
            cout << "是否成环(Y/N)："; char c; cin>>c; bool cycle=(c=='Y'||c=='y');
            auto D=g.allPairsShortest();
            auto [cost,tour] = tspAuto(D,s,cycle);
            if(cost>=INF/2){ cout << "图不连通，无法覆盖全部景点。\n"; continue; }
            cout.setf(std::ios::fixed); cout<<setprecision(2);
            cout << (cycle? "总距离(回路): " : "总距离: ") << cost << " m\n";
            cout << (cycle? "回路: " : "路径: ");
            for(size_t i=0;i<tour.size();++i){
                cout<<g.names[tour[i]]; if(i+1<tour.size()) cout<<" → ";
            }
            if(cycle) cout << " → " << g.names[tour.front()];
            cout << "\n";
        }
        else if(op==6){
            g.printAdj();
        }
        else if(op==7){
            if(saveGraphCSV(g)) cout << "[已保存] " << FILE_NEW << "\n";
            else cout << "[失败] 无法写入 " << FILE_NEW << "\n";
        }
        else if(op==8){
            if(loadGraphCSV(g)){
                cout << "[已加载] 顶点 " << g.names.size()
                     << "，道路 " << countEdges(g) << "。\n";
            }else{
                cout << "[失败] 未成功读取 CSV 文件。\n";
            }
        }
        else if(op==9){
            if(g.names.empty()){ cout << "请先添加景点或加载示例图。\n"; continue; }
            cout << "输入要修改的编号："; int u; cin>>u;
            if(u<0||u>=(int)g.names.size()){ cout<<"编号不合法。\n"; continue; }
            cout << "新坐标 x y (米)："; double x,y; cin>>x>>y;
            g.coord[u]={x,y};
            g.recomputeAllEdgeWeights();
            cout.setf(std::ios::fixed); cout<<setprecision(2);
            cout << "已更新坐标，相关边权已重算。\n";
            if(saveGraphCSV(g)) cout << "[已保存] " << FILE_NEW << "\n";
        }
        else{
            cout << "无此选项。\n";
        }
    }
}

int main(){
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    menu();
    return 0;
}
