#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <random>
#include <chrono>
#include <cmath>
#include <cassert>

using namespace std;
using namespace std::chrono;

// 長方形の構造体
struct Rectangle {
    int idx; // 長方形の番号
    int w, h; // 観測された幅と高さ
    int w_real, h_real; // 実際の幅と高さ
};

// 配置情報の構造体
struct Placement {
    int p;    // 長方形の番号
    int r;    // 回転フラグ（0または1）
    char d;   // 方向（'U'または'L'）
    int b;    // 基準となる長方形の番号（-1または0以上の整数）
};

// 焼きなまし法のパラメータ
const double TIME_LIMIT = 1.9; // 制限時間（秒）
const double START_TEMP = 1000;
const double END_TEMP = 1;

// グローバル変数
int N, T, sigma;
vector<Rectangle> rects;
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

// 時間計測用
high_resolution_clock::time_point start_time;

// 時間計測関数
double elapsed_time() {
    return duration_cast<duration<double>>(high_resolution_clock::now() - start_time).count();
}

// コスト関数の計算
int calc_score(const vector<Placement>& placements, int& W, int& H) {
    // 長方形の位置とサイズを計算
    vector<int> x(N), y(N), w(N), h(N);
    vector<bool> used(N, false);
    W = 0;
    H = 0;

    for (const auto& plc : placements) {
        int idx = plc.p;
        used[idx] = true;

        int rect_w = rects[idx].w;
        int rect_h = rects[idx].h;
        if (plc.r == 1) swap(rect_w, rect_h); // 回転

        int pos_x = 0;
        int pos_y = 0;

        if (plc.d == 'U') {
            // 左端の位置を決定
            if (plc.b == -1) {
                pos_x = 0;
            } else {
                pos_x = x[plc.b] + w[plc.b];
            }

            // 上に移動して重ならない位置を探す
            int max_y = 0;
            for (int i = 0; i < N; ++i) {
                if (used[i] && i != idx) {
                    // x方向で重なるか確認
                    if (max(pos_x, x[i]) < min(pos_x + rect_w, x[i] + w[i])) {
                        max_y = max(max_y, y[i] + h[i]);
                    }
                }
            }
            pos_y = max_y;
        } else { // 'L'
            // 上端の位置を決定
            if (plc.b == -1) {
                pos_y = 0;
            } else {
                pos_y = y[plc.b] + h[plc.b];
            }

            // 左に移動して重ならない位置を探す
            int max_x = 0;
            for (int i = 0; i < N; ++i) {
                if (used[i] && i != idx) {
                    // y方向で重なるか確認
                    if (max(pos_y, y[i]) < min(pos_y + rect_h, y[i] + h[i])) {
                        max_x = max(max_x, x[i] + w[i]);
                    }
                }
            }
            pos_x = max_x;
        }

        x[idx] = pos_x;
        y[idx] = pos_y;
        w[idx] = rect_w;
        h[idx] = rect_h;

        W = max(W, x[idx] + w[idx]);
        H = max(H, y[idx] + h[idx]);
    }

    // 未使用の長方形のスコアを計算
    int unused_score = 0;
    for (int i = 0; i < N; ++i) {
        if (!used[i]) {
            unused_score += rects[i].w + rects[i].h;
        }
    }

    int total_score = W + H + unused_score;
    return total_score;
}

// 近傍解の生成
void generate_neighbor(vector<Placement>& placements) {
    if (placements.empty()) return; // placementsが空の場合は何もしない

    uniform_int_distribution<int> dist_idx(0, placements.size() - 1);
    int idx = dist_idx(rng);

    // 操作を選択
    uniform_int_distribution<int> dist_op(0, 3);
    int op = dist_op(rng);

    if (op == 0) {
        // 回転を変更
        placements[idx].r ^= 1;
    } else if (op == 1) {
        // 方向を変更
        placements[idx].d = (placements[idx].d == 'U') ? 'L' : 'U';
    } else if (op == 2) {
        // 基準を変更
        if (idx == 0) {
            placements[idx].b = -1;
        } else {
            uniform_int_distribution<int> dist_b(-1, placements[idx].p - 1);
            placements[idx].b = dist_b(rng);
        }
    } else {
        // 長方形を追加または削除
        if (placements.size() < N && rng() % 2 == 0) {
            // 追加
            // まだ配置していない長方形を探す
            vector<int> unused_indices;
            for (int i = 0; i < N; ++i) {
                bool found = false;
                for (const auto& plc : placements) {
                    if (plc.p == i) {
                        found = true;
                        break;
                    }
                }
                if (!found) unused_indices.push_back(i);
            }
            if (!unused_indices.empty()) {
                uniform_int_distribution<int> dist_unused(0, unused_indices.size() - 1);
                int new_idx = unused_indices[dist_unused(rng)];
                int b_value = (placements.empty()) ? -1 : placements.back().p;
                placements.push_back({new_idx, 0, 'U', b_value});
                sort(placements.begin(), placements.end(), [](const Placement& a, const Placement& b) {
                    return a.p < b.p;
                });
            }
        } else if (placements.size() > 1) {
            // 削除
            placements.erase(placements.begin() + idx);
        }
    }
}

int main() {
    start_time = high_resolution_clock::now();

    // 入力
    cin >> N >> T >> sigma;
    rects.resize(N);
    for (int i = 0; i < N; ++i) {
        rects[i].idx = i;
        cin >> rects[i].w >> rects[i].h;
    }

    // 初期解の生成（すべての長方形を配置）
    vector<Placement> best_placements;
    for (int i = 0; i < N; ++i) {
        best_placements.push_back({i, 0, 'U', -1});
    }

    int best_W, best_H;
    int best_score = calc_score(best_placements, best_W, best_H);

        // T回の操作を実行
    for (int t = 0; t < T; ++t) {
    // 焼きなまし法の開始
    while (elapsed_time() < TIME_LIMIT/T * t) {
        double t = elapsed_time() / TIME_LIMIT;
        double temp = START_TEMP + (END_TEMP - START_TEMP) * t;

        // 近傍解を生成
        vector<Placement> new_placements = best_placements;
        generate_neighbor(new_placements);

        int new_W, new_H;
        int new_score = calc_score(new_placements, new_W, new_H);

        // スコアが改善されたら更新
        if (new_score < best_score || exp((best_score - new_score) / temp) > (double)rng() / rng.max()) {
            best_placements = new_placements;
            best_score = new_score;
        }
    }


        // 配置を出力
        cout << best_placements.size() << endl;
        for (const auto& plc : best_placements) {
            cout << plc.p << " " << plc.r << " " << plc.d << " " << plc.b << endl;
        }
        cout.flush();

        // 計測値を受け取る（無視）
        int W_obs, H_obs;
        cin >> W_obs >> H_obs;
    }

    return 0;
}
