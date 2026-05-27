#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <cmath>
#include <algorithm>

// PARAMS
constexpr int GRID     = 4;
constexpr int MAX_ST   = 50;
constexpr int TOTAL_EP = 1000000;
constexpr int STAGE    = 20000;

constexpr float ALPHA  = 0.20f;
constexpr float GAMMA  = 0.95f;
constexpr float EPS_S  = 1.0f, EPS_E = 0.05f, DECAY_EP = 2500.0f;

constexpr float HIT_AI = 0.55f, HIT_EN = 0.50f;
constexpr int HP_AI = 5, HP_EN = 4;

constexpr float R_WIN=100.0f, R_LOSE=-20.0f, R_STEP=-0.01f, R_HIT=8.0f;
constexpr float R_CLOSER=1.5f, R_ADJ=1.0f, R_WASTED=-1.0f, R_TAKEN_HIT=-0.5f;

constexpr int Q_SZ = 980, ACT = 5;
enum Action { UP, DOWN, LEFT, RIGHT, ATTACK };
float Q[Q_SZ][ACT] = {0.0f};

// STRUCTS
#pragma pack(push, 1)
struct Frame { int8_t ax, ay, ah; int8_t ex, ey, eh; int8_t action; bool ended; };
struct StageMetrics { int stage; float win_rate, avg_steps, var_steps, avg_reward, var_reward; };
struct GameRecord { float reward; std::vector<Frame> replay; };
struct ReplayData { int stage; std::vector<Frame> frames; };
#pragma pack(pop)

// ENV
struct Env {
    int ax=0, ay=3, ah=HP_AI, ex=3, ey=0, eh=HP_EN, st=0;
    bool end=false;
    void reset() { ax=0;ay=3;ah=HP_AI; ex=3;ey=0;eh=HP_EN; st=0; end=false; }
    int idx() const {
        int dx=ex-ax+3, dy=ey-ay+3, ha=(ah<1?0:ah-1), he=(eh<1?0:eh-1);
        return ((dx*7+dy)*5+ha)*4+he;
    }
    bool adj() const { return std::abs(ex-ax)+std::abs(ey-ay)==1; }
    float step(int a) {
        if(end) return 0.0f;
        float r = R_STEP;
        if(a == ATTACK) {
            if(adj() && static_cast<float>(std::rand())/RAND_MAX < HIT_AI) { eh--; r+=R_HIT; }
            else r += (adj() ? -1.0f : R_WASTED);
        } else if(a < 4) {
            int nx=ax, ny=ay;
            if(a==0)ny--; else if(a==1)ny++; else if(a==2)nx--; else if(a==3)nx++;
            if(nx>=0 && nx<GRID && ny>=0 && ny<GRID && !(nx==ex && ny==ey)) { ax=nx; ay=ny; if(adj()) r+=R_CLOSER; }
        }
        if(adj()) r += R_ADJ;
        if(eh<=0) { r+=R_WIN; end=true; return r; }
        if(adj() && static_cast<float>(std::rand())/RAND_MAX < HIT_EN) { ah--; r+=R_TAKEN_HIT; }
        else {
            int dx[4]={0,0,1,-1}, dy[4]={1,-1,0,0}; std::vector<int> valid;
            for(int i=0;i<4;++i){ int nx=ex+dx[i], ny=ey+dy[i]; if(nx>=0&&nx<GRID&&ny>=0&&ny<GRID&&!(nx==ax&&ny==ay)) valid.push_back(i); }
            if(!valid.empty()){ int dir=valid[std::rand()%valid.size()]; ex+=dx[dir]; ey+=dy[dir]; }
        }
        if(ah<=0) { r+=R_LOSE; end=true; }
        if(++st>=MAX_ST) end=true;
        return r;
    }
};

// Q-L
int policy(int s, float eps) {
    if(static_cast<float>(std::rand())/RAND_MAX < eps) return std::rand()%ACT;
    int best=0; float maxQ=Q[s][0];
    for(int i=1;i<ACT;++i) if(Q[s][i]>maxQ){maxQ=Q[s][i]; best=i;}
    return best;
}
void update(int s, int a, float r, int s2, bool terminal) {
    float target = terminal ? 0.0f : Q[s2][0];
    for(int i=1;i<ACT;++i) if(Q[s2][i]>target) target=Q[s2][i];
    Q[s][a] += ALPHA * (r + GAMMA*target - Q[s][a]);
}

// ==================== MAIN ====================
int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    std::cout << "Training " << TOTAL_EP << " episodes...\n";

    Env env; float eps=EPS_S, decay=(EPS_S-EPS_E)/DECAY_EP;
    float sum_t=0, sum_t2=0, sum_r=0, sum_r2=0; int wins=0, count=0;
    for(int i=0;i<Q_SZ;++i) for(int j=0;j<ACT;++j) Q[i][j]=(static_cast<float>(std::rand())/RAND_MAX-0.5f)*0.05f;

    std::vector<GameRecord> stage_games;
    std::vector<StageMetrics> all_metrics;
    std::vector<ReplayData> saved_replays;

    for(int ep=1; ep<=TOTAL_EP; ++ep) {
        env.reset(); int s=env.idx(); float ep_r=0;
        std::vector<Frame> current_replay;
        while(!env.end) {
            int a=policy(s,eps);
            current_replay.push_back({static_cast<int8_t>(env.ax),static_cast<int8_t>(env.ay),static_cast<int8_t>(env.ah),
                                      static_cast<int8_t>(env.ex),static_cast<int8_t>(env.ey),static_cast<int8_t>(env.eh),
                                      static_cast<int8_t>(a),false});
            float r=env.step(a); int s2=env.idx();
            update(s,a,r,s2,env.end); ep_r+=r; s=s2;
        }
        if(!current_replay.empty()) current_replay.back().ended=(env.eh<=0 && env.ah>0);
        stage_games.push_back({ep_r, current_replay});

        float steps=static_cast<float>(env.st);
        sum_t+=steps; sum_t2+=steps*steps; sum_r+=ep_r; sum_r2+=ep_r*ep_r;
        if(env.eh<=0 && env.ah>0) wins++; count++;

        if(ep%STAGE==0) {
            float n=static_cast<float>(count);
            float E_t=sum_t/n, V_t=sum_t2/n-E_t*E_t, E_r=sum_r/n, V_r=sum_r2/n-E_r*E_r, W=wins/n;
            std::cout << "Stage " << ep << "/" << TOTAL_EP << " | Win%=" << W*100 << " | E[R]=" << E_r << "\n";
            std::cout.flush();
            all_metrics.push_back({ep, W, E_t, V_t, E_r, V_r});

            if(!stage_games.empty()) {
                float target=E_r, best_diff=1e9f;
                std::vector<Frame> median;
                for(const auto& g : stage_games) { float d=std::abs(g.reward-target); if(d<best_diff){best_diff=d; median=g.replay;} }
                if(!median.empty()) saved_replays.push_back({ep, std::move(median)});
                stage_games.clear();
            }
            sum_t=sum_t2=sum_r=sum_r2=0; wins=0; count=0;
        }
        eps = std::max(EPS_E, eps - decay);
    }

    // SAVING
    std::cout << "\nFINISHED\n";
    std::ofstream csv("stage_metrics.csv");
    if(csv.is_open()){
        csv << "stage,win_rate,avg_steps,var_steps,avg_reward,var_reward\n";
        for(const auto& m : all_metrics) csv << m.stage << "," << m.win_rate << "," << m.avg_steps << "," << m.var_steps << "," << m.avg_reward << "," << m.var_reward << "\n";
        csv.close(); std::cout << "metrics saved\n";
    }
    if(FILE* fq=std::fopen("qtable.dat","wb")){ std::fwrite(Q, sizeof(float), Q_SZ*ACT, fq); std::fclose(fq); std::cout << "Q-Table saved\n"; }

    if(!saved_replays.empty()){
        if(FILE* f=std::fopen("replays.bin","wb")){
            uint32_t n=static_cast<uint32_t>(saved_replays.size());
            std::fwrite(&n, sizeof(n), 1, f);
            for(const auto& r : saved_replays){
                std::fwrite(&r.stage, sizeof(r.stage), 1, f);
                uint32_t nf=static_cast<uint32_t>(r.frames.size());
                std::fwrite(&nf, sizeof(nf), 1, f);
                if(nf>0) std::fwrite(r.frames.data(), sizeof(Frame), nf, f);
            }
            std::fclose(f); std::cout << "replays saved\n";
        }
    }
    return 0;
}