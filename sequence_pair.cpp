#include <iostream>
#include <vector>
#include <algorithm>
#include <queue> 
#include <tuple>
#include <fstream>
using namespace std;

ofstream debug_out("debug.log");

int N, T, sigma;

// 長方形を表す構造体
struct  Rect{
    int w,h;
    int x,y; 
    bool rotate;
    Rect(int w,int h):w(w),h(h),x(0),y(0),rotate(false){}
};

struct Placement{
    int id;
    int r;//rotate
    char d;
    int b; //基準長方形
};

struct SequencePair {
    vector<Rect> rect; //長方形の情報
    vector<int>  rotate;//それぞれの長方形の回転情報
    vector<int>  P; // 左から右の順序
    vector<int>  Q; // 下から上の順序
    vector<int> x_coords, y_coords; // 各矩形の座標

     SequencePair(vector<Rect> rects, vector<int> p, vector<int> q)
        : rect(rects), P(p), Q(q) {
        x_coords.resize(N, 0);
        y_coords.resize(N, 0);
    }

    void print() const {
        cout << "P: ";
        for (int x : P) cout << x << " ";
        cout << "\nQ: ";
        for (int x : Q) cout << x << " ";
        cout << endl;
    }

    int score(){
    // インデックスの逆引きマップを作成
    unordered_map<int, int> index_in_gamma_minus;
    for (int i = 0; i < N; ++i) {
        index_in_gamma_minus[P[i]] = i;
    }

    // x, y 座標を計算する
    vector<int> x_coords(N, 0), y_coords(N, 0);

    for (int i = 0; i < N; ++i) {
        int rect_id = P[i];
        int max_x = 0, max_y = 0;

        // 矩形 `rect_id` より前にある矩形を探索
        for (int j = 0; j < i; ++j) {
            int prev_id = P[j];

            if (index_in_gamma_minus[prev_id] < index_in_gamma_minus[rect_id]) {
                // 左に配置
                max_x = max(max_x, x_coords[prev_id] + rect[prev_id].w);
            } else {
                // 上に配置
                max_y = max(max_y, y_coords[prev_id] + rect[prev_id].h);
            }
        }

        // 座標を更新
        x_coords[rect_id] = max_x;
        y_coords[rect_id] = max_y;
    }

    // 最大 x 座標と y 座標を計算
    int max_x = 0, max_y = 0;
    for (int i = 0; i < N; ++i) {
        max_x = max(max_x, x_coords[i] + rect[i].w);
        max_y = max(max_y, y_coords[i] + rect[i].h);
    }

    debug_out << "max_x: " << max_x << ", max_y: " << max_y << endl;
    // スコアを計算 (横幅 + 縦幅)
    return max_x + max_y;  
    }

 vector<Placement> generate_operations_from_sequence_pair() {
    vector<Placement> operations;

    // 各長方形の回転後の幅と高さを計算
    vector<int> widths(N), heights(N);
    for (int i = 0; i < N; ++i) {
        if (rect[i].rotate) {
            widths[i] = rect[i].h;
            heights[i] = rect[i].w;
        } else {
            widths[i] = rect[i].w;
            heights[i] = rect[i].h;
        }
    }

    // HCG（水平制約グラフ）と VCG（垂直制約グラフ）の構築
    unordered_map<int, int> pos_in_P, pos_in_Q;
    for (int i = 0; i < N; ++i) {
        pos_in_P[P[i]] = i;
        pos_in_Q[Q[i]] = i;
    }
    vector<vector<int>> adj_H(N), adj_V(N);
    for (int i = 0; i < N; ++i) {
        int rect_i = P[i];
        for (int j = 0; j < N; ++j) {
            if (i == j) continue;
            int rect_j = P[j];
            if (pos_in_P[rect_i] < pos_in_P[rect_j] && pos_in_Q[rect_i] < pos_in_Q[rect_j]) {
                adj_H[rect_i].push_back(rect_j);
            } else if (pos_in_P[rect_i] < pos_in_P[rect_j] && pos_in_Q[rect_i] > pos_in_Q[rect_j]) {
                adj_V[rect_i].push_back(rect_j);
            }
        }
    }

    // HCG に基づく x 座標の計算と基準の記録
vector<int> in_degree_H(N, 0), base_x(N, -1);
for (int i = 0; i < N; ++i)
    for (int j : adj_H[i]) in_degree_H[j]++;
queue<int> q;
for (int i = 0; i < N; ++i)
    if (in_degree_H[i] == 0) q.push(i);
while (!q.empty()) {
    int u = q.front(); q.pop();
    for (int v : adj_H[u]) {
        if (x_coords[v] < x_coords[u] + widths[u]) {
            x_coords[v] = x_coords[u] + widths[u];
            base_x[v] = u; // 基準長方形を記録
        }
        if (--in_degree_H[v] == 0) q.push(v);
    }
}

// VCG に基づく y 座標の計算と基準の記録
vector<int> in_degree_V(N, 0), base_y(N, -1);
for (int i = 0; i < N; ++i)
    for (int j : adj_V[i]) in_degree_V[j]++;
for (int i = 0; i < N; ++i)
    if (in_degree_V[i] == 0) q.push(i);
while (!q.empty()) {
    int u = q.front(); q.pop();
    for (int v : adj_V[u]) {
        if (y_coords[v] < y_coords[u] + heights[u]) {
            y_coords[v] = y_coords[u] + heights[u];
            base_y[v] = u; // 基準長方形を記録
        }
        if (--in_degree_V[v] == 0) q.push(v);
    }
}

// デバッグ情報の出力
for (int i = 0; i < N; i++) {
    debug_out << "Rect ID " << i << ": "
              << "x=" << x_coords[i] << " (Base=" << base_x[i] << "), "
              << "y=" << y_coords[i] << " (Base=" << base_y[i] << ")" << endl;
}
    for (int idx = 0; idx < N; ++idx) {
        int rect_id = P[idx];
        Placement op;
        op.id = rect_id;
        op.r = rect[rect_id].rotate ? 1 : 0;

        int x = x_coords[rect_id];
        int y = y_coords[rect_id];

        if(y_coords[base_x[rect_id]] == (base_x[rect_id] == -1 ? 0 : y_coords[rect_id])){
            op.d = 'L';
            op.b = base_x[rect_id]; 
        }
        else {
            op.d = 'U';
            op.b = base_y[rect_id];
        }
        

        operations.push_back(op);
    }
    for(int i=0;i<N;i++){
        debug_out << operations[i].id << " " << operations[i].r << " " << operations[i].d << " " << operations[i].b << endl;
    }

    return operations;
}



};

void debug(){
    N = 4;
    vector<Rect> rects;
    for(int i = 0; i < 1; ++i){
        rects.emplace_back(100, 200);
        rects.emplace_back(150, 300);
        rects.emplace_back(120, 250);
        rects.emplace_back(180, 160);
    }
    // // P, Q の例を生成 (順番は単純な順列として仮定)
    vector<int> P(N), Q(N);
       
    P = {0, 1, 2, 3};
    Q =  {3, 1, 2, 0};

     // シーケンスペアを作成
    SequencePair sp(rects, P, Q);
    vector<Placement> ops = sp.generate_operations_from_sequence_pair();


    // 操作手順を出力
    ofstream out("operations.txt");
    for (const auto &op : ops) {
        out << op.id << " " << op.r << " " << op.d << " " << op.b << endl;
    }
     for(int i=0;i<4;i++){
        out << sp.x_coords[i] << " " << sp.y_coords[i] << endl;
    }
    out.close();

}
int main() {
    //cin >> N >> T >> sigma;
    debug();
    

    vector<int> P(N), Q(N);

    return 0;
}
 

