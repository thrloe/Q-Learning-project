# Q-Learning project
Reinforcement Learning & Probabilistic Modeling in a Discrete Strategy Game


A C++ implementation of tabular Q-Learning applied to a stochastic 4×4 grid duel. An AI agent learns to defeat a probabilistic enemy by maximizing expected discounted rewards. The project bridges theoretical Markov Decision Processes (MDP) with practical reinforcement learning.

---

## матан/теорвер

Проект формализован как конечный Марковский процесс принятия решений (MDP):

| Компонент | Описание | Значение |
|-----------|----------|----------|
| Состояние s | (dx, dy, HP_AI, HP_EN) | 7×7×5×4 = 980 состояний |
| Действия A | {UP, DOWN, LEFT, RIGHT, ATTACK} | 5 действий |
| Переходы P(s'\|s,a) | Стохастические: rand() < p_hit + случайные ходы врага | p_AI = 0.55, p_EN = 0.50 |
| Награды R | WIN=+100, LOSE=-20, STEP=-0.01, HIT=+8, CLOSER=+1.5 | Reward shaping |
| Цель | Максимизировать `E[∑ γᵏ·rₖ]` | γ=0.95 |

### Обновление Q-таблицы (эмпирическая апраксимация уравнения Беллмана) 
Q[s][a] += ALPHA * (r + GAMMA * max_next - Q[s][a]);
// α=0.2, γ=0.95, max_next = max_a' Q[s'][a']

Это стохастическая аппроксимация матожидания: по Закону Больших Чисел при многократном посещении пары (s,a) значение Q[s][a] сходится к истинному E[R_{total}].

### Исследование: $\epsilon$-greedy

epsilon_t = max(0.05, 1.0 - t / 2500);  // линейное затухание
Гарантирует посещение всех состояний для выполнения условий сходимости теоремы Роббинса-Монро.

---

## repo

├── train.cpp          # Q-Learning, csv metrics saving, I/O

├── play.cpp           # the game

├── watch.cpp          # watch replays 

├── stage_metrics.csv  # Q-L results: win_rate, avg_reward, var_steps and more

├── qtable.dat         # final Q-table

├── replays.bin        # replays of the median games from each (of 50) stages

└── README.md          # you are here


### Форматы сохранённых данных
| file | format | desc |
|------|--------|----------|
| stage_metrics.csv | Text CSV | stage,win_rate,avg_steps,var_steps,avg_reward,var_reward |
| qtable.dat | Binary float[980][5] | q-table - resulting policy after 1M games |
| replays.bin | Binary header + blocks of frames | effective way to record thousands of games |
---

## req
- C++17 (GCC/Clang/MSVC)
- [Raylib](https://www.raylib.com/) (only needed for play.cpp and watch.cpp)

### flags:
train.cpp : -O2
play.cpp + watch.cpp : -lraylib 

---

## results

| stage | Win Rate | Avg Steps | Avg Reward | Var(Steps) |
|------|----------|-----------|------------|------------|
| 20 000 | 25.5% | 16.45 | 42.24 | 64.03 |
| 100 000 | 34.5% | 14.89 | 53.9 | 27.2 |
| 500 000 | 58.1% | 15.45 | 82.30 | 25.24 |
| 1 000 000 | 61.4% | 15.44 | 85.61 | 25.14 |

# + look for more in stage_metrics.csv (your training results may vary from mine)
---
