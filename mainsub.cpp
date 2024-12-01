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

int N, T, sigma;
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
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
    vector<Rect>* rectangles; 
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
        vector<int> x(n), y(n), w(n), h(n);
        vector<int>dir(n);
        //vector<P>limit(n);
         auto doubling = [&](int nx, int ny, int nw, int nh, int count) -> bool {
                        for (int k = 0; k < count; ++k) {
                            if(dir[k]==1){
                                swap(nw,nh);
                            }
                            if (max(nx, x[k]) < min(nx + nw, x[k] + w[k]) &&
                                max(ny, y[k]) < min(ny + nh, y[k] + h[k])) {
                                // 重なりがある場合
                                debug_out << "id = "<< k << "doubleing" << endl;
                                return true;
                            }
                        }
                // 重なりがない場合
                return false;
            };

        int score = 0;
        for(int i=0;i<n;i++){
            score += (*rectangles)[i].w_obs + (*rectangles)[i].h_obs;
        }
        Rect& rectz = (*rectangles)[0];
        bool  rotate = false ;
        w[0] = rectz.w_obs;
        h[0] = rectz.h_obs;
        x[0] = 0;
        y[0] = 0;
        dir[0] = 0;
        Placement placement = {0, rotate ? 1 : 0, 'U', -1};
        placements.push_back(placement);
        used[0] = true;
        // first : 下から上 second : 右から左
        //limit[0] = make_pair(0,0);
        W = w[0];
        H = h[0];
        
        int xlim = W;
        int ylim = H;
        score += W + H - rectz.w_obs - rectz.h_obs;
        struct select
        {
            int dir;
            int coord;
            int index;
            int x;
            int y;
            select(int dir,int coord,int index,int x,int y):dir(dir),coord(coord),index(index),x(x),y(y){}
            select() : dir(0), coord(0), index(0), x(0), y(0) {}
        };
        
        select best_sel;// first: dir second: coord third: index
        int best_score = 1e9; 

        for(int i = 1;i<n;i++){
            Rect &rect = (*rectangles)[i];
            // 既存の長方形を基準にする
            for(int j = 0;j<i;j++){
                //回転
                for(int dir = 0;dir <2;dir++){
                    int rect_w = dir ? rect.h_obs : rect.w_obs;
                    int rect_h = dir ? rect.w_obs : rect.h_obs;
                    //x,y軸方向 0:U  1:L
                    for (int coord = 0; coord < 2; ++coord) {
                        if(i %2 == 0&& coord == 0)continue;
                        int pos_x , pos_y;
                    if (coord == 0) { // 'U' 方向への配置
                        pos_x = x[j]+w[j];
                        for(int k = 0;k<i;k++){
                            pos_y = y[k]+h[k];
                            if(doubling(pos_x,pos_y,rect_w,rect_h,i)){
                                debug_out << i << ' ' << j << ' ' << dir <<' ' << "doubling" << endl;
                                debug_out << "pos_x: " << pos_x << " pos_y: " << pos_y << " rect_w: " << rect_w << " rect_h: " << rect_h << endl;
                                continue;
                            }
                            else break;
                        }
                    } else { // 'L' 方向への配置
                        pos_y =  y[j]+h[j];
                        for(int k = 0;k<i;k++){
                            pos_x = x[k]+w[k];
                            if(doubling(pos_x,pos_y,rect_w,rect_h,i)){
                                debug_out << i << ' ' << j << ' ' << dir <<' ' << "doubling" << endl;
                                debug_out << "pos_x: " << pos_x << " pos_y: " << pos_y << " rect_w: " << rect_w << " rect_h: " << rect_h << endl;
                                continue;
                            }
                            else break;
                        }
                    }

                    // if (doubling(pos_x, pos_y, rect_w, rect_h, j)) {
                    //     debug_out << i << ' ' << j << ' ' << dir <<' ' << "doubling" << endl;
                    //     debug_out << "pos_x: " << pos_x << " pos_y: " << pos_y << " rect_w: " << rect_w << " rect_h: " << rect_h << endl;
                    //     continue;  // 重なりがある場合はスキップ
                    // }

                    int next_xlim = max(xlim, pos_x + rect_w);
                    int next_ylim = max(ylim, pos_y + rect_h);
                    int next_score = score + max(xlim, pos_x + rect_w) + max(ylim, pos_y + rect_h) - xlim - ylim - rect.w_obs - rect.h_obs;
                    debug_out << i << ' '  << j << ' '<< dir << "next_score: " << next_score << endl;
                    if (next_score < best_score) {
                        best_score = next_score;
                        best_sel = {dir, coord, j,pos_x,pos_y};
                    }
                }
                }
            }
            Placement placement = {i, best_sel.dir ? 1:0, best_sel.coord? 'L':'U', best_sel.index};
            score += max(xlim,x[i]+w[i])+max(ylim,y[i]+h[i])-xlim-ylim-rect.w_obs-rect.h_obs;
            dir[i] = best_sel.dir;
            x[i] = best_sel.x;
            y[i] = best_sel.y;
            h[i] = rect.h_obs;
            w[i] = rect.w_obs;
            xlim = max(xlim,x[i]+w[i]);
            ylim = max(ylim,y[i]+h[i]);
            //debug_out << i << " x;" << x[i] << " y:" << y[i] << " w:" << w[i] << " h:" << h[i] << ' ' << dir[i] << ' ' << (get<1>(best_sel) == 0 ? 'U' : 'L') << ' ' << get<2>(best_sel) << endl;
            placements.push_back(placement);
            //limit[i] = make_pair(y[i],x[i]);
            // if (get<1>(best_sel) == 0) { // 下から上
            //     limit[get<2>(best_sel)].second += h[i];
            // }
            // else
            // { // 右から左
            //     limit[get<2>(best_sel)].first += w[i];
            // }
            //debug_out << "limit[" << get<2>(best_sel) << "] = (" << limit[get<2>(best_sel)].first << ", " << limit[get<2>(best_sel)].second << ")" << endl;

        }
          //debugPlacements(x, y, w, h, placements);
    }

    void debugPlacements(const vector<int>& x, const vector<int>& y, const vector<int>& w, const vector<int>& h, const vector<Placement>& placements) {
    debug_out << "=== Debug Placements ===" << endl;
    debug_out << "Index | X      | Y      | Width  | Height | Rotate | Direction | Base" << endl;
    debug_out << "-------------------------------------------------------------" << endl;
    for (size_t i = 0; i < placements.size(); i++) {
        int index = placements[i].id;
        int rotate = placements[i].r;
        char direction = placements[i].d;
        int base = placements[i].b;
        debug_out << index << "     | " 
             << x[index] << "      | " 
             << y[index] << "      | " 
             << w[index] << "       | " 
             << h[index] << "       | " 
             << rotate << "       | " 
             << direction << "          | " 
             << base << endl;
    }
    debug_out << "=========================" << endl;
}

    void Score() {
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

    //@brief 結果を出力
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
  
}

bool isValidPlacement(const State& state) {
    int n = state.placements.size();

    // 長方形の位置とサイズを計算
    vector<int> x(n), y(n), w(n), h(n);
    for (int i = 0; i < n; ++i) {
        const Placement& plc = state.placements[i];
        int idx = plc.id;
        // 長方形の縦横
        double rect_w = (*state.rectangles)[idx].w_est;
        double rect_h = (*state.rectangles)[idx].h_est;
        //回転する場合交換
        if (plc.r == 1) swap(rect_w, rect_h);

        //返ってくるときは四捨五入した後の値
        // int rect_w_int = max(1, (int)round(rect_w));
        // int rect_h_int = max(1, (int)round(rect_h));

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

        // 配置が負の座標にならないことを確認
        if (x[i] < 0 || y[i] < 0) {
            return false;
        }
    }

    // 長方形の重なり確認
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


void improveSolution(State& state) {
  
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
            uniform_int_distribution<int> dist_b(-1, placements[idx].id - 1);
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
                    if (plc.id == i) {
                        found = true;
                        break;
                    }
                }
                if (!found) unused_indices.push_back(i);
            }
            if (!unused_indices.empty()) {
                uniform_int_distribution<int> dist_unused(0, unused_indices.size() - 1);
                int new_idx = unused_indices[dist_unused(rng)];
                int b_value = (placements.empty()) ? -1 : placements.back().id;
                placements.push_back({new_idx, 0, 'U', b_value});
                sort(placements.begin(), placements.end(), [](const Placement& a, const Placement& b) {
                    return a.id < b.id;
                });
            }
        } else if (placements.size() > 1) {
            // 削除
            placements.erase(placements.begin() + idx);
        }
    }
}

void solve() {
    auto global_start_time = chrono::steady_clock::now();
    cin >> N >> T >> sigma;

    vector<Rect> rects(N);
    for (int i = 0; i < N; ++i) {
        cin >> rects[i].w_obs >> rects[i].h_obs;
        rects[i].w_est = rects[i].w_obs;
        rects[i].h_est = rects[i].h_obs;
        rects[i].id = i;
    }

    // 初期解の生成
    State state(rects);
    state.generateInitialSolution();
    State best_state = state;

    int best_score = state.score;

    const double TOTAL_TIME_LIMIT = 2.68;
    const double TIME_LIMIT = TOTAL_TIME_LIMIT / T; // 制限時間（秒） per T iteration
    const double START_TEMP = 1000.0;
    const double END_TEMP = 1e-3;

        auto turn_start_time = chrono::steady_clock::now();
        double time_per_turn = TOTAL_TIME_LIMIT / T;

        const int MAX_ITERATIONS = 100; // 1秒当たりの最大反復回数
        int iteration = 0;

        while (true) {
            auto now = chrono::steady_clock::now();
            double elapsed_time = chrono::duration<double>(now - turn_start_time).count();

            if (elapsed_time > time_per_turn) break;
            double t_ratio = elapsed_time / TIME_LIMIT;
            double temp = START_TEMP * pow(END_TEMP / START_TEMP, t_ratio);

            // 近傍解を生成
            State new_state = state;
            //generate_neighbor(new_state.placements);

            // スコア計算
            new_state.Score();
            int new_score = new_state.score;

            // スコアが改善された場合、または確率的遷移条件を満たした場合に更新
            if (new_score < state.score || exp((state.score - new_score) / temp) > frand()) {
                state = new_state;
                if (new_score < best_score) {
                    best_score = new_score;
                    best_state = new_state;
                }
            }

            iteration++;
            if(iteration >= MAX_ITERATIONS) break;
        }
    for (int t = 0; t < T; ++t) {
        // t回目の配置を出力
        best_state.print();

        // 計測値を受け取る
        int W_obs, H_obs;
        cin >> W_obs >> H_obs;

        // 推定値を更新
        //updateEstimates(rects, W_obs, H_obs, best_state.W, best_state.H, sigma);

        // 状態をリセット
        state = best_state;
        state.Score();
    }
}


int main() {
    solve();
    return 0;
}