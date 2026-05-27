#include <raylib.h>
#include <cmath>
#include <vector>
#include <ctime>

const int GRID = 4;
const float AI_HIT = 0.6f;
const float ENEMY_HIT = 0.5f;
const int PLAYER_HP_START = 5;
const int ENEMY_HP_START = 4;

enum Action { UP, DOWN, LEFT, RIGHT, ATTACK, N_ACTIONS };

struct Unit { int x, y, hp; };

struct Game {
    Unit ai = {0, 3, PLAYER_HP_START};
    Unit enemy = {3, 0, ENEMY_HP_START};
    bool gameOver = false;
    
    Game() { srand((unsigned)time(NULL)); }
    
    bool isAdjacent() {
        return abs(ai.x - enemy.x) + abs(ai.y - enemy.y) == 1;
    }
    
    void playerMove(Action a) {
        if(gameOver) return;
        int nx = ai.x, ny = ai.y;
        if(a == UP) ny--; if(a == DOWN) ny++;
        if(a == LEFT) nx--; if(a == RIGHT) nx++;
        
        if(a != ATTACK && nx>=0 && nx<GRID && ny>=0 && ny<GRID && !(nx==enemy.x && ny==enemy.y)) {
            ai.x = nx; ai.y = ny;
        }
        
        if(a == ATTACK && isAdjacent()) {
            if((float)rand()/RAND_MAX < AI_HIT) {
                enemy.hp--;
                if(enemy.hp <= 0) gameOver = true;
            }
        }
        
        if(!gameOver) enemyTurn();
        if(ai.hp <= 0) gameOver = true;
    }
    
    void enemyTurn() {
        if(isAdjacent() && (float)rand()/RAND_MAX < ENEMY_HIT) {
            ai.hp--; 
            return;
        }
        
        if((float)rand()/RAND_MAX < 0.7f) {
            int best_dx = 0, best_dy = 0;
            int minDist = abs((enemy.x + 0) - ai.x) + abs((enemy.y + 0) - ai.y);
            
            int dx[4] = {0,0,1,-1}, dy[4] = {1,-1,0,0};
            for(int i=0; i<4; ++i) {
                int ex = enemy.x + dx[i], ey = enemy.y + dy[i];
                if(ex>=0 && ex<GRID && ey>=0 && ey<GRID && !(ex==ai.x && ey==ai.y)) {
                    int d = abs(ex - ai.x) + abs(ey - ai.y);
                    if(d < minDist) { minDist = d; best_dx = dx[i]; best_dy = dy[i]; }
                }
            }
            if(minDist < abs(enemy.x - ai.x) + abs(enemy.y - ai.y)) {
                enemy.x += best_dx; enemy.y += best_dy;
                return;
            }
        }

        int dx[4]={0,0,1,-1}, dy[4]={1,-1,0,0};
        std::vector<int> valid;
        for(int i=0;i<4;++i){
            int ex=enemy.x+dx[i], ey=enemy.y+dy[i];
            if(ex>=0&&ex<GRID&&ey>=0&&ey<GRID&&!(ex==ai.x&&ey==ai.y)) valid.push_back(i);
        }
        if(!valid.empty()) {
            int dir = valid[rand()%valid.size()];
            enemy.x += dx[dir]; enemy.y += dy[dir];
        }
    }
};

int main() {
    InitWindow(400, 450, "the GAME");
    SetTargetFPS(30);
    Game g;
    
    while(!WindowShouldClose()) {
        if(IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) g.playerMove(UP);
        if(IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) g.playerMove(DOWN);
        if(IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) g.playerMove(LEFT);
        if(IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) g.playerMove(RIGHT);
        if(IsKeyPressed(KEY_SPACE) && !g.gameOver) g.playerMove(ATTACK);
        if(IsKeyPressed(KEY_R)) g = Game();
        
        BeginDrawing();
        ClearBackground(BLACK);
        for(int y=0;y<GRID;++y) for(int x=0;x<GRID;++x)
            DrawRectangleLines(x*100, y*100+20, 100, 100, DARKGRAY);
        
        DrawCircle(g.ai.x*100+50, g.ai.y*100+70, 30, BLUE);
        DrawText(TextFormat("%d", g.ai.hp), g.ai.x*100+42, g.ai.y*100+60, 20, WHITE);
        
        DrawCircle(g.enemy.x*100+50, g.enemy.y*100+70, 30, RED);
        DrawText(TextFormat("%d", g.enemy.hp), g.enemy.x*100+42, g.enemy.y*100+60, 20, WHITE);
        
        if(g.gameOver) DrawText(g.enemy.hp<=0 ? "YOU WIN!" : "YOU LOSE", 120, 220, 40, g.enemy.hp<=0 ? GREEN : RED);
        DrawText("WASD/Arrows Move  |  SPACE Attack  |  R Reset", 10, 420, 16, GRAY);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}