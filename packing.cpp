#include <iostream>
#include <vector>
#include <algorithm>
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

    // シーケンスペアから操作手順を生成する関数
vector<Placement> generate_operations_from_sequence_pair() {
    vector<Placement> operations;

    // 配置済みの矩形を管理する
    vector<bool> placed(N, false);

    // シーケンスペアの順序に従い操作を生成
    for (int i = 0; i < N; ++i) {
        int rect_id = P[i];
        Rect nextrect = rect[rect_id];
        Placement op;
        op.id = rect_id;
        op.r = rect[rect_id].rotate ? 1 : 0;


        // 配置方向と基準の決定
        if (y_coords[rect_id] == 0) {
            // 下から上に配置
            op.d = 'U';
            op.b = -1; // 基準なし（軸）
        } else if (x_coords[rect_id] == 0) {
            // 右から左に配置
            op.d = 'L';
            op.b = -1; // 基準なし（軸）
        } else {
            // 基準を探す（配置済みの矩形を考慮）
            int base = -1;
            if (op.d == 'U') {
                // 左に最も近い矩形を基準とする
                int max_x = -1;
                for (int j = 0; j < N; ++j) {
                    if (placed[j] && x_coords[j] + rect[j].w <= x_coords[rect_id]) {
                        if (x_coords[j] + rect[j].w > max_x) {
                            max_x = x_coords[j] + rect[j].w;
                            base = j;
                        }
                    }
                }
            } else if (op.d == 'L') {
                // 下に最も近い矩形を基準とする
                int max_y = -1;
                for (int j = 0; j < N; ++j) {
                    if (placed[j] && y_coords[j] + rect[j].h <= y_coords[rect_id]) {
                        if (y_coords[j] + rect[j].h > max_y) {
                            max_y = y_coords[j] + rect[j].h;
                            base = j;
                        }
                    }
                }
            }
            op.b = base;
        }

        // 配置済み矩形としてマーク
        placed[rect_id] = true;
        operations.push_back(op);
    }
    return operations;
}

};

int main() {
    // 入力を読み込む
    // cin >> N >> T >> sigma;
    // N = 4;
    // vector<Rect> rects;
    // for (int i = 0; i < N; ++i) {
    //     int w, h;
    //     cin >> w >> h;
    //     rects.emplace_back(w, h);
    // }
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
    // for (int i = 0; i < N; ++i) {
    //     P[i] = i;
    //     Q[i] = N - 1 - i; // 逆順

    // シーケンスペアを作成
    SequencePair sp(rects, P, Q);

    // // スコアを計算
    // int score = sp.score();
    // sp.print();
    // debug_out << "Score: " << score << endl;

     // サンプルデータを用いてテスト
   
    P = {0, 1, 2, 3};
    Q =  {3, 1, 2, 0};

    SequencePair sp2(rects, P, Q);
    
    sp2.score(); // 座標を計算

    vector<Placement> ops = sp2.generate_operations_from_sequence_pair();

    // 操作手順を出力
    for (const auto &op : ops) {
        debug_out << op.id << " " << op.r << " " << op.d << " " << op.b << endl;
    }

    return 0;
}
 

