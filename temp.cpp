#include <bits/stdc++.h>
#include <atcoder/all>
using namespace std;
using namespace atcoder;
#define rep(i, n) for (int i = 0; i < (int)(n); i++)
#define all(v) v.begin(), v.end()
using ll = long long;
using Graph = vector<vector<int>>;
using P = pair<int,int>;

ofstream debug_out("debug.log");

uint32_t xor128() {
    static uint32_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
    uint32_t t = (x ^ (x << 11));
    x = y; y = z; z = w;
    return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
}

inline double frand() {
    return xor128() % UINT32_MAX / static_cast<double>(UINT32_MAX);
}

template<typename T, typename U>
inline bool chmin(T& a, U b) { if (a > b) { a = b; return true; } return false; }

// 長方形の構造体
struct Rect {
    int id;               // 長方形の番号
    int w_obs, h_obs;     // 観測された幅と高さ
    double w_est, h_est;  // 推定された幅と高さ 
};

// 配置情報の構造体
struct Placement {
    int id; // id 
    int r;  // 回転フラグ（0または1）
    char d; // 方向（'U'または'L'）
    int b;  // 基準となる長方形の番号（-1または0以上の整数）
};

// 状態
struct State {
    vector<Placement> placements;
    vector<bool> used;
    vector<Rect>* rectangles; // ポインタに変更
    int W, H;
    int score;

    State(vector<Rect>& rects) : rectangles(&rects), used(rects.size(), false), W(0), H(0), score(INT_MAX) {}

    State(const State& other)
        : placements(other.placements),
          used(other.used),
          rectangles(other.rectangles),
          W(other.W),
          H(other.H),
          score(other.score) {}

    State& operator=(const State& other) {
        if (this != &other) {
            placements = other.placements;
            used = other.used;
            rectangles = other.rectangles;
            W = other.W;
            H = other.H;
            score = other.score;
        }
        return *this;
    }

void generateInitialSolution() {
    int n = rectangles->size();
    vector<int> x(n, 0), y(n, 0), w(n), h(n);

    // 最初の長方形を原点に配置
    {
        Rect& rect = (*rectangles)[0];
        bool rotate = false;
        w[0] = rect.w_obs;
        h[0] = rect.h_obs;
        x[0] = 0;
        y[0] = 0;

        Placement placement = {0, rotate ? 1 : 0, 'U', -1};
        placements.push_back(placement);
        used[0] = true;

        W = w[0];
        H = h[0];
    }

    // 2番目以降の長方形を配置
    for (int i = 1; i < n; ++i) {
        Rect& rect = (*rectangles)[i];

        int best_x = 0, best_y = 0, best_b = -1;
        char best_d = 'U';
        bool best_rotate = false;
        int min_area = INT_MAX;

        // 既存の長方形に隣接する候補位置を生成
        for (int j = 0; j < i; ++j) {
            for (bool rotate : {false, true}) {
                int rect_w = rotate ? rect.h_obs : rect.w_obs;
                int rect_h = rotate ? rect.w_obs : rect.h_obs;

                // 右側に配置する候補
                int pos_x_R = x[j] + w[j];
                int pos_y_R = y[j];

                // 上側に配置する候補
                int pos_x_U = x[j];
                int pos_y_U = y[j] + h[j];

                // それぞれについて配置を試みる
                for (auto [pos_x, pos_y, dir] : vector<tuple<int, int, char>>{
                         {pos_x_R, pos_y_R, 'L'}, // 左方向（右側に配置）
                         {pos_x_U, pos_y_U, 'U'}  // 上方向（上側に配置）
                     }) {

                    // 重なりチェック
                    bool overlap = false;
                    for (int k = 0; k < i; ++k) {
                        if (max(pos_x, x[k]) < min(pos_x + rect_w, x[k] + w[k]) &&
                            max(pos_y, y[k]) < min(pos_y + rect_h, y[k] + h[k])) {
                            overlap = true;
                            break;
                        }
                    }
                    if (overlap) continue;

                    // 包絡長方形のサイズを計算
                    int new_W = max(W, pos_x + rect_w);
                    int new_H = max(H, pos_y + rect_h);
                    int area = new_W * new_H;

                    // 最小のエリアを更新
                    if (area < min_area) {
                        min_area = area;
                        best_x = pos_x;
                        best_y = pos_y;
                        best_b = j;
                        best_d = dir;
                        best_rotate = rotate;
                    }
                }
            }
        }

        // 最適な配置が見つからなければ、右上に配置
        if (best_b == -1) {
            // 回転の有無を検討
            for (bool rotate : {false, true}) {
                int rect_w = rotate ? rect.h_obs : rect.w_obs;
                int rect_h = rotate ? rect.w_obs : rect.h_obs;

                int pos_x = W;
                int pos_y = 0;

                // 重なりチェック
                bool overlap = false;
                for (int k = 0; k < i; ++k) {
                    if (max(pos_x, x[k]) < min(pos_x + rect_w, x[k] + w[k]) &&
                        max(pos_y, y[k]) < min(pos_y + rect_h, y[k] + h[k])) {
                        overlap = true;
                        break;
                    }
                }
                if (overlap) continue;

                // 包絡長方形のサイズを計算
                int new_W = pos_x + rect_w;
                int new_H = max(H, pos_y + rect_h);
                int area = new_W * new_H;

                if (area < min_area) {
                    min_area = area;
                    best_x = pos_x;
                    best_y = pos_y;
                    best_b = -1;
                    best_d = 'L';
                    best_rotate = rotate;
                }
            }
        }

        // 配置を記録
        x[i] = best_x;
        y[i] = best_y;
        w[i] = best_rotate ? rect.h_obs : rect.w_obs;
        h[i] = best_rotate ? rect.w_obs : rect.h_obs;

        Placement placement = {i, best_rotate ? 1 : 0, best_d, best_b};
        placements.push_back(placement);
        used[i] = true;

        // 領域を更新
        W = max(W, best_x + w[i]);
        H = max(H, best_y + h[i]);
    }

    // スコアを計算
    computeScore();
}


    void computeScore() {
        int n = placements.size();
        vector<int> x(rectangles->size()), y(rectangles->size()), w(rectangles->size()), h(rectangles->size());
        used.assign(rectangles->size(), false);
        W = 0;
        H = 0;

        for (int i = 0; i < n; ++i) {
            const Placement& plc = placements[i];
            int idx = plc.id;
            used[idx] = true;
            double rect_w = (*rectangles)[idx].w_est;
            double rect_h = (*rectangles)[idx].h_est;
            if (plc.r == 1) swap(rect_w, rect_h);

            int rect_w_int = max(1, (int)round(rect_w));
            int rect_h_int = max(1, (int)round(rect_h));

            int pos_x = 0, pos_y = 0;
            if (plc.d == 'U') {
                if (plc.b == -1) {
                    pos_x = 0;
                } else {
                    pos_x = x[plc.b] + w[plc.b];
                }
                int max_y = 0;
                for (int j = 0; j < i; ++j) {
                    int other_idx = placements[j].id;
                    if (max(pos_x, x[other_idx]) < min(pos_x + rect_w_int, x[other_idx] + w[other_idx])) {
                        max_y = max(max_y, y[other_idx] + h[other_idx]);
                    }
                }
                pos_y = max_y;
            } else { // 'L'
                if (plc.b == -1) {
                    pos_y = 0;
                } else {
                    pos_y = y[plc.b] + h[plc.b];
                }
                int max_x = 0;
                for (int j = 0; j < i; ++j) {
                    int other_idx = placements[j].id;
                    if (max(pos_y, y[other_idx]) < min(pos_y + rect_h_int, y[other_idx] + h[other_idx])) {
                        max_x = max(max_x, x[other_idx] + w[other_idx]);
                    }
                }
                pos_x = max_x;
            }

            x[idx] = pos_x;
            y[idx] = pos_y;
            w[idx] = rect_w_int;
            h[idx] = rect_h_int;

            W = max(W, pos_x + rect_w_int);
            H = max(H, pos_y + rect_h_int);
        }

        int unused_score = 0;
        for (int i = 0; i < rectangles->size(); ++i) {
            if (!used[i]) {
                unused_score += (int)round((*rectangles)[i].w_est + (*rectangles)[i].h_est);
            }
        }

        score = W + H + unused_score;
    }

    void print() {
        cout << placements.size() <<  endl;
        for (const auto& plc : placements) {
            cout << plc.id << " " << plc.r << " " << plc.d << " " << plc.b << endl;
        }
        cout.flush();
    }

    bool operator<(const State& other) const {
        return score < other.score;
    }
};

// 推定値の更新関数
void updateEstimates(vector<Rect>& rectangles, int W_obs, int H_obs, int W_pred, int H_pred, double sigma) {
    double K_w = sigma * sigma / (sigma * sigma + sigma * sigma);
    double delta_W = W_obs - W_pred;

    for (auto& rect : rectangles) {
        rect.w_est += K_w * delta_W / rectangles.size();
        rect.w_est = max(1.0, min(1e9, rect.w_est));
    }

    double K_h = sigma * sigma / (sigma * sigma + sigma * sigma);
    double delta_H = H_obs - H_pred;

    for (auto& rect : rectangles) {
        rect.h_est += K_h * delta_H / rectangles.size();
        rect.h_est = max(1.0, min(1e9, rect.h_est));
    }
}

bool isValidPlacement(const State& state) {
    int n = state.placements.size();

    // 長方形の位置とサイズを計算
    vector<int> x(n), y(n), w(n), h(n);
    for (int i = 0; i < n; ++i) {
        const Placement& plc = state.placements[i];
        int idx = plc.id;
        double rect_w = (*state.rectangles)[idx].w_est;
        double rect_h = (*state.rectangles)[idx].h_est;
        if (plc.r == 1) swap(rect_w, rect_h);

        int rect_w_int = max(1, (int)round(rect_w));
        int rect_h_int = max(1, (int)round(rect_h));

        if (plc.b == -1) {
            // 基準長方形が存在しない場合（最初の長方形）
            x[i] = 0;
            y[i] = 0;
        } else {
            // 基準長方形の検証
            const Placement& base_plc = state.placements[plc.b];
            int base_idx = base_plc.id;

            // **基準のIDが現在のIDより小さいことを確認**
            if (base_idx >= idx) {
                return false; // 基準IDが現在のIDより大きい場合は不正
            }

            // 配置位置を計算
            if (plc.d == 'U') {
                x[i] = x[base_idx];
                y[i] = y[base_idx] + h[base_idx];
            } else if (plc.d == 'L') {
                x[i] = x[base_idx] + w[base_idx];
                y[i] = y[base_idx];
            } else {
                return false; // 不正な方向指定
            }
        }

        w[i] = rect_w_int;
        h[i] = rect_h_int;

        // 配置が負の座標にならないことを確認
        if (x[i] < 0 || y[i] < 0) {
            return false;
        }
    }

    // 長方形が重なっていないことを確認
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (max(x[i], x[j]) < min(x[i] + w[i], x[j] + w[j]) &&
                max(y[i], y[j]) < min(y[i] + h[i], y[j] + h[j])) {
                return false; // 重なりがある場合は無効
            }
        }
    }

    return true; // 配置が有効
}

void generateNeighbor(State& state) {
    if (state.placements.empty()) return;

    int idx = xor128() % state.placements.size();
    int op = xor128() % 3;

    State next_state = state;
    if (op == 0) {
        // 回転を変更
        next_state.placements[idx].r ^= 1;
        if(isValidPlacement(next_state)){
            state.placements[idx].r ^= 1;
        }
    } else if (op == 1) {
        // 方向を変更
        next_state.placements[idx].d = (state.placements[idx].d == 'U') ? 'L' : 'U';
        if(isValidPlacement(next_state)){
            state.placements[idx].d = (state.placements[idx].d == 'U') ? 'L' : 'U';
        }
    } else {
        // 基準長方形を変更
        int base_idx = xor128() % state.placements.size();
        next_state.placements[idx].b = -1;
        if(isValidPlacement(next_state)){
            state.placements[idx].b = -1;
        }
    }

    // 配置の整合性チェック
    if (!isValidPlacement(state)) {
        debug_out << "Invalid neighbor generated. Skipping..." << endl;
    }
}

void improveSolution(State& state) {
    vector<tuple<int, int, int, int>> free_spaces; // 空きスペース (x, y, width, height)

    // 初期空きスペースは配置領域全体
    free_spaces.push_back({0, 0, state.W, state.H});

    // 配置済みの長方形を元に空きスペースを計算
    for (const auto& plc : state.placements) {
        int idx = plc.id;
        double rect_w = (*state.rectangles)[idx].w_est;
        double rect_h = (*state.rectangles)[idx].h_est;
        if (plc.r == 1) swap(rect_w, rect_h);

        int rect_x = (plc.b == -1) ? 0 : plc.d == 'U' ? plc.b : plc.b + rect_w;
        int rect_y = (plc.b == -1) ? 0 : plc.d == 'U' ? plc.b + rect_h : plc.b;

        int rect_w_int = max(1, (int)round(rect_w));
        int rect_h_int = max(1, (int)round(rect_h));

        // 空きスペースから配置済み領域を引いて更新
        vector<tuple<int, int, int, int>> new_spaces;
        for (const auto& space : free_spaces) {
            int sx, sy, sw, sh;
            tie(sx, sy, sw, sh) = space;

            // 長方形と重なる領域を除去
            if (sx >= rect_x + rect_w_int || sx + sw <= rect_x || sy >= rect_y + rect_h_int || sy + sh <= rect_y) {
                // 重ならない場合、そのまま追加
                new_spaces.push_back(space);
            } else {
                // 重なる場合、分割して追加
                if (sy < rect_y) { // 上部分
                    new_spaces.push_back({sx, sy, sw, rect_y - sy});
                }
                if (sx < rect_x) { // 左部分
                    new_spaces.push_back({sx, sy, rect_x - sx, sh});
                }
                if (sx + sw > rect_x + rect_w_int) { // 右部分
                    new_spaces.push_back({rect_x + rect_w_int, sy, (sx + sw) - (rect_x + rect_w_int), sh});
                }
                if (sy + sh > rect_y + rect_h_int) { // 下部分
                    new_spaces.push_back({sx, rect_y + rect_h_int, sw, (sy + sh) - (rect_y + rect_h_int)});
                }
            }
        }
        free_spaces = new_spaces;
    }

    // 未配置の長方形を小さい順に並べ替え
    vector<int> unplaced_indices;
    for (int i = 0; i < state.rectangles->size(); ++i) {
        if (!state.used[i]) unplaced_indices.push_back(i);
    }
    sort(unplaced_indices.begin(), unplaced_indices.end(), [&](int a, int b) {
        return (*state.rectangles)[a].w_obs * (*state.rectangles)[a].h_obs <
               (*state.rectangles)[b].w_obs * (*state.rectangles)[b].h_obs;
    });

    // 空きスペースに未配置の長方形を埋め込む
    for (int rect_idx : unplaced_indices) {
        const Rect& rect = (*state.rectangles)[rect_idx];
        bool placed = false;

        for (auto it = free_spaces.begin(); it != free_spaces.end(); ++it) {
            int sx, sy, sw, sh;
            tie(sx, sy, sw, sh) = *it;

            for (bool rotate : {false, true}) {
                int rect_w = rotate ? rect.h_obs : rect.w_obs;
                int rect_h = rotate ? rect.w_obs : rect.h_obs;

                if (rect_w <= sw && rect_h <= sh) {
                    // 配置可能なら配置
                    state.placements.push_back({rect_idx, rotate ? 1 : 0, 'U', -1});
                    state.used[rect_idx] = true;

                    // 空きスペースを更新
                    vector<tuple<int, int, int, int>> new_spaces;
                    for (const auto& space : free_spaces) {
                        int fsx, fsy, fsw, fsh;
                        tie(fsx, fsy, fsw, fsh) = space;

                        if (fsx >= sx + rect_w || fsx + fsw <= sx || fsy >= sy + rect_h || fsy + fsh <= sy) {
                            new_spaces.push_back(space);
                        } else {
                            if (fsy < sy) { // 上部分
                                new_spaces.push_back({fsx, fsy, fsw, sy - fsy});
                            }
                            if (fsx < sx) { // 左部分
                                new_spaces.push_back({fsx, fsy, sx - fsx, fsh});
                            }
                            if (fsx + fsw > sx + rect_w) { // 右部分
                                new_spaces.push_back({sx + rect_w, fsy, (fsx + fsw) - (sx + rect_w), fsh});
                            }
                            if (fsy + fsh > sy + rect_h) { // 下部分
                                new_spaces.push_back({fsx, sy + rect_h, fsw, (fsy + fsh) - (sy + rect_h)});
                            }
                        }
                    }
                    free_spaces = new_spaces;
                    placed = true;
                    break;
                }
            }
            if (placed) break;
        }
    }

    state.computeScore(); // スコア再計算
}


void solve() {
    int N, T, sigma;
    cin >> N >> T >> sigma;

    vector<Rect> rects(N);
    for (int i = 0; i < N; ++i) {
        cin >> rects[i].w_obs >> rects[i].h_obs;
        rects[i].w_est = rects[i].w_obs;
        rects[i].h_est = rects[i].h_obs;
        rects[i].id = i;
    }

    const double TOTAL_TIME_LIMIT = 2.68;
    const double INITIAL_TEMP = 1000.0;
    const double FINAL_TEMP = 1e-3;

    auto global_start_time = chrono::steady_clock::now();

    State state(rects);
    state.generateInitialSolution();
    State best_state = state;

    debug_out << "Initial solution score: " << state.score << endl;

    for (int t = 0; t < T; t++) {
        auto turn_start_time = chrono::steady_clock::now();
        double time_per_turn = TOTAL_TIME_LIMIT / T;

        debug_out << chrono::duration<double>(turn_start_time - global_start_time).count() 
                  << " seconds elapsed since the start." << endl;
        debug_out << "Turn " << t + 1 << "/" << T << " started. Time limit per turn: " 
                  << time_per_turn << " seconds." << endl;

        int iterations = 0;
        while (true) {
            auto now = chrono::steady_clock::now();
            double elapsed_time = chrono::duration<double>(now - turn_start_time).count();

            if (elapsed_time > time_per_turn) break;

            State next_state = state;
            //generateNeighbor(next_state);
            improveSolution(next_state);
            next_state.computeScore();

            double temp = INITIAL_TEMP * pow(FINAL_TEMP / INITIAL_TEMP, elapsed_time / time_per_turn);
            double delta = next_state.score - state.score;

            if (delta < 0 || exp(-delta / temp) > frand()) {
                state = next_state;
                if (state < best_state) {
                    best_state = state;
                    debug_out << "New best score: " << best_state.score 
                              << " (Iteration: " << iterations << ")" << endl;
                }
            }

            ++iterations;

            if (iterations % 100 == 0) {
                debug_out << "Iteration " << iterations << ", Current score: " 
                          << state.score << ", Best score: " << best_state.score 
                          << ", Temperature: " << temp << endl;
            }
            //if(iterations >= 10000) break;
        }

        debug_out << "Turn " << t + 1 << " finished. Best score for this turn: " 
                  << best_state.score << endl;

        best_state.print();

        int W_obs, H_obs;
        cin >> W_obs >> H_obs;

        debug_out << "Observed W: " << W_obs << ", Observed H: " << H_obs 
                  << ", Predicted W: " << best_state.W 
                  << ", Predicted H: " << best_state.H << endl;

        //updateEstimates(rects, W_obs, H_obs, best_state.W, best_state.H, sigma);
        debug_out << "Estimates updated for next turn." << endl;
    }

    debug_out << "Optimization complete. Final best score: " << best_state.score << endl;
}

int main() {
    solve();
    return 0;
}