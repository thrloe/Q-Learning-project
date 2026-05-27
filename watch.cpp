#include <raylib.h>
#include <cstdio>
#include <vector>
#include <cstdint>

#pragma pack(push, 1)
struct Frame { int8_t ax, ay, ah; int8_t ex, ey, eh; int8_t action; bool ended; };
#pragma pack(pop)

struct Replay { int stage; std::vector<Frame> frames; };
std::vector<Replay> library;
size_t idx = 0, frameIdx = 0;
float timer = 0;
bool paused = false, loaded = false;

bool loadLibrary() {
    FILE* f = std::fopen("replays.bin", "rb");
    if(!f) return false;
    uint32_t n = 0;
    if(std::fread(&n, sizeof(n), 1, f) != 1) { std::fclose(f); return false; }
    library.resize(n);
    for(uint32_t i=0; i<n; ++i){
        std::fread(&library[i].stage, sizeof(int), 1, f);
        uint32_t nf = 0;
        std::fread(&nf, sizeof(nf), 1, f);
        library[i].frames.resize(nf);
        if(nf > 0) std::fread(library[i].frames.data(), sizeof(Frame), nf, f);
    }
    std::fclose(f);
    return !library.empty();
}

int main() {
    InitWindow(420, 560, "Q-Learning Replay Viewer");
    SetTargetFPS(30);
    loaded = loadLibrary();

    while(!WindowShouldClose()) {
        if(IsKeyPressed(KEY_RIGHT) && idx + 1 < library.size()) { idx++; frameIdx=0; timer=0; }
        if(IsKeyPressed(KEY_LEFT)  && idx > 0)                 { idx--; frameIdx=0; timer=0; }
        if(IsKeyPressed(KEY_SPACE)) paused = !paused;
        if(IsKeyPressed(KEY_R))     { frameIdx=0; timer=0; paused=false; }

        if(loaded && !paused && frameIdx < library[idx].frames.size()) {
            Frame &c = library[idx].frames[frameIdx];
            timer += GetFrameTime();
            float dur = c.ended ? 4.0f : 0.3f;
            if(timer >= dur) {
                timer = 0;
                if(c.ended) frameIdx = 0;
                else { frameIdx++; if(frameIdx >= library[idx].frames.size()) frameIdx = 0; }
            }
        }

        BeginDrawing(); ClearBackground(BLACK);
        DrawText(TextFormat("Stage: %d | %zu/%zu", loaded ? library[idx].stage : 0, idx+1, library.size()), 10, 10, 18, loaded ? GREEN : RED);

        if(loaded && frameIdx < library[idx].frames.size()) {
            Frame &c = library[idx].frames[frameIdx];
            for(int y=0; y<4; ++y) for(int x=0; x<4; ++x) DrawRectangleLines(x*100, y*100+70, 100, 100, DARKGRAY);
            if(c.ah>0) { DrawCircle(c.ax*100+50, c.ay*100+120, 30, BLUE); DrawText(TextFormat("%d", c.ah), c.ax*100+42, c.ay*100+110, 20, WHITE); }
            if(c.eh>0) { DrawCircle(c.ex*100+50, c.ey*100+120, 30, RED);   DrawText(TextFormat("%d", c.eh), c.ex*100+42, c.ey*100+110, 20, WHITE); }
            const char* acts[] = {"UP","DOWN","LEFT","RIGHT","ATTACK"};
            DrawText(acts[c.action], 160, 500, 18, GREEN);
            if(c.ended) { DrawRectangle(130,230,160,50, Fade(BLACK,0.7f)); DrawText("WIN!", 160, 240, 40, GREEN); }
        } else if(loaded) { DrawText("REPLAY COMPLETE", 100, 250, 24, WHITE); }
        else { DrawText("replays.bin not found", 120, 250, 20, RED); }

        if(paused) DrawText("PAUSED", 170, 270, 30, YELLOW);
        DrawText(" <- -> Stage | SPACE Pause | R Reset ", 10, 530, 14, GRAY);
        EndDrawing();
    }
    CloseWindow(); return 0;
}
