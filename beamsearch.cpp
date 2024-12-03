#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <random>
#include <chrono>
#include <cmath>
#include <cassert>
#include <fstream>

using namespace std;
using namespace std::chrono;

ofstream debug_out("debug.log");

// 長方形の構造体
struct Rectangle {
    int idx; // 長方形の番号
    int w, h; // 観測された幅と高さ
};

// 配置情報の構造体
struct Placement {
    int p;    // 長方形の番号
    int r;    // 回転フラグ（0または1）
    char d;   // 方向（'U'または'L'）
    int b;    // 基準となる長方形の番号（-1または0以上の整数）
};

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

// 状態の構造体
struct State {
    vector<Placement> placements; // 現在の配置
    vector<Rectangle>* rectangles; 
    int W, H;                     // 現在の幅・高さ
    int score;                    // スコア

    bool operator<(const State& other) const {
        return score > other.score; // 小さいスコアが優先
    }
    
    // デフォルトコンストラクタ
    State() : rectangles(nullptr), W(0), H(0), score(1e9) {}

    // 引数付きコンストラクタ
    State(vector<Rectangle>& rects) 
        : rectangles(&rects), W(0), H(0), score(1e9) {}

    // スコアを計算
    int calc_score() {
        vector<int> x(N), y(N), w(N), h(N);
        vector<bool> used(N, false);
        W = 0;
        H = 0;

        for (const auto& plc : placements) {
            int idx = plc.p;
            used[idx] = true;

            int rect_w = (*rectangles)[idx].w;
            int rect_h = (*rectangles)[idx].h;
            if (plc.r == 1) swap(rect_w, rect_h); // 回転

            int pos_x = 0, pos_y = 0;

            if (plc.d == 'U') {
                pos_x = (plc.b == -1) ? 0 : x[plc.b] + w[plc.b];
                int max_y = 0;
                for (int i = 0; i < N; ++i) {
                    if (used[i] && max(pos_x, x[i]) < min(pos_x + rect_w, x[i] + w[i])) {
                        max_y = max(max_y, y[i] + h[i]);
                    }
                }
                pos_y = max_y;
            } else { // 'L'
                pos_y = (plc.b == -1) ? 0 : y[plc.b] + h[plc.b];
                int max_x = 0;
                for (int i = 0; i < N; ++i) {
                    if (used[i] && max(pos_y, y[i]) < min(pos_y + rect_h, y[i] + h[i])) {
                        max_x = max(max_x, x[i] + w[i]);
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

        int unused_score = 0;
        for (int i = 0; i < N; ++i) {
            if (!used[i]) unused_score += (*rectangles)[i].w + (*rectangles)[i].h;
        }

        score = W + H + unused_score;
        return score;
    }
};

vector<Placement> beam_search(State& state, int beam_width, int t) {
    vector<State> now_beam;
    now_beam.push_back(state);

    State best_state = state;

    for (int i = 1; i < N; ++i) {
        vector<State> next_beam;
         double randomness_factor = exp(-5.0 * t / N);
        for (const auto& current_state : now_beam) {
            for (int dir = 0; dir < 2; ++dir) { // 回転の有無
                for (int coord = 0; coord < 2; ++coord) { // 上からか右からか
                    for (int idx = -1; idx < i; ++idx) { // 基準の長方形
                        State next_state = current_state;
                        Placement next_placement = {i, dir, coord ? 'L' : 'U', idx};
                        next_state.placements.push_back(next_placement);
                        next_state.calc_score();
                        next_beam.push_back(next_state);
                    }
                }
            }
        }

        sort(next_beam.begin(), next_beam.end());
        if (next_beam.size() > beam_width) next_beam.resize(beam_width);

        now_beam = next_beam;
    }

    for (const auto& state : now_beam) {
        if (state.score < best_state.score) {
            best_state = state;
        }
    }

    return best_state.placements;
}

int main() {
    start_time = high_resolution_clock::now();

    cin >> N >> T >> sigma;
    rects.resize(N);
    for (int i = 0; i < N; ++i) {
        cin >> rects[i].w >> rects[i].h;
    }

    State initial_state(rects);
    initial_state.placements.push_back({0, 0, 'U', -1});
    initial_state.calc_score();

    vector<Placement> best_placements = beam_search(initial_state, 15, T);

    for (int t = 0; t < T; ++t) {
        cout << best_placements.size() << endl;
        for (const auto& plc : best_placements) {
            cout << plc.p << " " << plc.r << " " << plc.d << " " << plc.b << endl;
        }
        cout.flush();

        int W_obs, H_obs;
        cin >> W_obs >> H_obs;
    }

    return 0;
}


//ビームサーチ途中経過のデバッグ出力