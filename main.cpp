#include <bits/stdc++.h>
#include <atcoder/all>
using namespace std;
using namespace atcoder;
#define rep(i, n) for (int i = 0; i < (int)(n); i++)
#define all(v) v.begin(), v.end()
using ll = long long;
using Graph = vector<vector<int>>;
using P = pair<int,int>;
template<typename T,typename U>inline bool chmax(T&a,U b)
{if(a<b){a=b;return 1;}return 0;}
template<typename T,typename U>inline bool chmin(T&a,U b)
{if(a>b){a=b;return 1;}return 0;}

class Timer {
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point getNow() {
        return std::chrono::system_clock::now();
    }

   public:
    void start() {
        start_time = getNow();
    }
    float getSec() {
        float count =
            std::chrono::duration_cast<std::chrono::microseconds>(getNow() - start_time).count();
        return count / 1e6;
    }
};

uint32_t xor128() {
    static uint32_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
    uint32_t t;
    t = (x ^ (x << 11));
    x = y;
    y = z;
    z = w;
    return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
}
inline float frand() {
    return xor128() % UINT32_MAX / static_cast<float>(UINT32_MAX);
}
inline int exprand(int x) {
    return (int)pow(x, frand());
}
template <class RandomAccessIterator>
void xor128_shuffle(RandomAccessIterator first, RandomAccessIterator last) {
    typename iterator_traits<RandomAccessIterator>::difference_type i, n;
    n = (last - first);
    for (i = n - 1; i > 0; --i) swap(first[i], first[xor128() % (i + 1)]);
}

static constexpr double GLOBAL_TIME_LIMIT = 1.97;
Timer global_timer;

struct Rect {
    int id;       // 長方形の番号
    int w_obs;    // 観測された幅
    int h_obs;    // 観測された高さ
    double w_est; // 推定される幅
    double h_est; // 推定される高さ
};

struct Placement {
    int p;    // 長方形の番号
    int r;    // 回転フラグ (0 or 1)
    char d;   // 配置方向 ('U' or 'L')
    int b;    // 基準となる長方形の番号 (-1 or 0以上)
};

struct State {
    vector<Placement> placements;      // 配置された長方形の情報
    vector<bool> used;                 // 長方形が使用されているか
    vector<Rect>& rectangles;          // 長方形の情報への参照
    int W, H;                          // 全体の幅、高さ
    int score;                         // 現在のスコア

    State(vector<Rect>& rects) : rectangles(rects), used(rects.size(), false), W(0), H(0), score(INT_MAX) {}

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
            W = other.W;
            H = other.H;
            score = other.score;
        }
        return *this;
    }

    void computeScore() {
        int n = placements.size();
        vector<int> x(rectangles.size()), y(rectangles.size()), w(rectangles.size()), h(rectangles.size());
        used.assign(rectangles.size(), false);
        W = 0;
        H = 0;

        for (int i = 0; i < n; ++i) {
            const Placement& plc = placements[i];
            int idx = plc.p;
            used[idx] = true;
            double rect_w = rectangles[idx].w_est;
            double rect_h = rectangles[idx].h_est;
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
                    int other_idx = placements[j].p;
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
                    int other_idx = placements[j].p;
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
        for (int i = 0; i < rectangles.size(); ++i) {
            if (!used[i]) {
                unused_score += (int)round(rectangles[i].w_est + rectangles[i].h_est);
            }
        }

        score = W + H + unused_score;
    }

    void print() {
        cout << placements.size() << endl;
        for (const auto& plc : placements) {
            cout << plc.p << " " << plc.r << " " << plc.d << " " << plc.b << endl;
        }
        cout.flush();
    }

    bool operator<(const State& other) const {
        return score < other.score;
    }
};

void input(int& N, int& T, int& sigma, vector<Rect>& rectangles) {
    cin >> N >> T >> sigma;
    rectangles.resize(N);
    for (int i = 0; i < N; ++i) {
        int w_obs, h_obs;
        cin >> w_obs >> h_obs;
        rectangles[i] = {i, w_obs, h_obs, (double)w_obs, (double)h_obs};
    }
}

class Transition {
   protected:
    const string name;
    State& state;

   public:
    Transition(const string& name, State& state) : name(name), state(state) {
    }
    virtual ~Transition() = default;
    virtual void apply() = 0;
    virtual void rollback() = 0;
    virtual string get_name() {
        return name;
    }
};

class TransitionSelector {
    vector<int> table; // 重み付きのインデックスを保持するテーブル
    vector<unique_ptr<Transition>> transitions; // Transitionオブジェクトのユニークポインタのベクター
    int select_index; // 選択されたTransitionのインデックス

   public:
    // Transitionオブジェクトとその重みを追加するメソッド
    void add(unique_ptr<Transition>&& transition, int weight) {
        int idx = transitions.size(); // 現在のtransitionsのサイズをインデックスとして使用
        for (int i = 0; i < weight; i++) {
            table.push_back(idx); // 重みの数だけインデックスをtableに追加
        }
        transitions.emplace_back(move(transition)); // transitionをtransitionsに追加
    }

    // ランダムにTransitionを選択するメソッド
    inline void select_transition() {
        select_index = table[xor128() % table.size()]; // ランダムにインデックスを選択
    }

    // 選択されたTransitionを適用するメソッド
    inline void apply() {
        transitions[select_index]->apply(); // 選択されたTransitionのapplyメソッドを呼び出す
    }

    // 選択されたTransitionをロールバックするメソッド
    inline void rollback() {
        transitions[select_index]->rollback(); // 選択されたTransitionのrollbackメソッドを呼び出す
    }

    // 選択されたTransitionの名前を出力するメソッド
    inline void dump() {
        cerr << "select: " << transitions[select_index]->get_name() << endl; // 選択されたTransitionの名前を標準エラー出力に出力
    }
};

class Transition1 : public Transition {
   private:
   public:
    Transition1(const string& name, State& state) : Transition(name, state) {
    }
    virtual void apply() override {
        state.score++;
    }
    virtual void rollback() override {
        state.score--;
    }
};

void input() {
}

void init(State& state) {
}

double calc_temp(double sec, double time_limit) {
    static const double START_TEMP = 10.0;
    static const double END_TEMP = 1e-9;
    return START_TEMP + (END_TEMP - START_TEMP) * sec / time_limit;
}

void solve() {
    int N, T, sigma;
    vector<Rect> rectangles;
    input(N, T, sigma, rectangles);

    State state(rectangles);
    for (int i = 0; i < N; ++i) {
        state.placements.push_back({i, 0, 'U', -1});
    }
    state.computeScore();

    State best_state = state;

    Timer timer;
    timer.start();

    while (timer.getSec() < GLOBAL_TIME_LIMIT) {
        double temp = calc_temp(timer.getSec(), GLOBAL_TIME_LIMIT);
        State next_state = state;
        // ランダムに操作を適用する
        if (frand() > 0.5) {
            if (!next_state.placements.empty()) {
                int idx = xor128() % next_state.placements.size();
                next_state.placements[idx].r ^= 1;
            }
        }
        next_state.computeScore();
        int delta = next_state.score - state.score;

        if (delta < 0 || exp(-delta / temp) > frand()) {
            state = next_state;
            if (state < best_state) {
                best_state = state;
            }
        }
    }

    for (int t = 0; t < T; ++t) {
        best_state.print();
        int W_obs, H_obs;
        cin >> W_obs >> H_obs;
    }

    cerr << "Best score: " << best_state.score << endl;
}

int main() {
    global_timer.start();
    input();
    solve();
    return 0;
}