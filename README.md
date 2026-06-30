# Q-Learning Project
Reinforcement learning and probabilistic modeling in a game with discrete strategies

Implementation of tabular Q-Learning in C++, applied in a stochastic duel on a 4×4 board. An AI agent learns to defeat an opponent using probabilistic modeling, maximizing the expected discounted reward. The project combines theoretical decision-making processes in Markov Decision Processes (MDP) with practical reinforcement learning. 

---

## Mathematics/theory

The project is formalized as a finite Markov Decision Process (MDP):

| Component | Description | Value |
|-----------|----------|----------|
| State s | (dx, dy, HP_AI, HP_EN) | 7×7×5×4 = 980 states |
| Actions A | {UP, DOWN, LEFT, RIGHT, ATTACK} | 5 actions |
| Transitions P(s'\|s,a) | Stochastic: rand() < p_hit + random enemy moves | p_AI = 0.55, p_EN = 0.50 |
| Rewards R | WIN=+100, LOSE=-20, STEP=-0.01, HIT=+8, CLOSE=+1.5 | Reward generation |
| Goal | Maximize `E[∑ γᵏ·rₖ]` | γ=0.95 |

### Update the Q-table (empirical approximation of the Bellman equation) 
Q[s][a] += ALPHA * (r + GAMMA * max_next - Q[s][a]);
// α=0,2, γ=0,95, max_next = max_a' Q[s'][a']

This is a stochastic approximation of the mathematical expectation: by the Law of Large Numbers, when the pair (x,a) is visited repeatedly, the value of m[y][a] converges to the true e[G_{total}].

### Corollary: $\epsilon$-greedy

epsilon_t = max(0.05, 1.0 - t / 2500); // linear decay
Guarantees that all states are visited to satisfy the convergence conditions of the Robbins-Monro theorem.

---

## repository

├── train.cpp          # Q-learning, saving metrics in csv format, input/output

├── play.cpp           # game

├── watch.cpp          # view replays

├── stage_metrics.csv  # Q-learning results: win rate, average reward, number of variant steps, etc.

├── qtable.dat         # final Q-table

├── replays.bin        # replays of the median games of each (of 50) stage

└── README.md          # you are here


### Formats of saved data
| file | format | description |
|------|--------|----------|
| stage_metrics.csv | Text CSV | stage, win rate, average steps, step variance, average reward, reward variance |
| qtable.dat | Binary float[980][5] | q-table — final policy after 1 million games |
| replays.bin | Binary header + frame blocks | efficient way to record thousands of games |
---

## req
- C++17 (GCC/Clang/MSVC)
- [Raylib](https://www.raylib.com/) (required only for play.cpp and watch.cpp)

### flags:
train.cpp : -O2
play.cpp + watch.cpp : -lraylib 

---

## results

| stage | Win rate | Average number of steps | Average reward | Variability (number of steps) |
|------|----------|-----------|------------|------------|
| 20,000 | 25.5% | 16.45 | 42.24 | 64.03 |
| 100,000 | 34.5% | 14.89 | 53.9 | 27.2 |
| 500 000 | 58.1% | 15.45 | 82.30 | 25.24 |
| 1 000 000 | 61.4% | 15.44 | 85.61 | 25.14 |

# + see other metrics in the stage_metrics.csv file (your results may differ from mine)
---
