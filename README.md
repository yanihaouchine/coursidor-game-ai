# Coursidor — game server and AI players for a Quoridor-like board game in C

![C](https://img.shields.io/badge/language-C99-blue) ![GSL](https://img.shields.io/badge/lib-GSL-lightgrey) ![Graphviz](https://img.shields.io/badge/viz-Graphviz-lightgrey)

*[Version française](README.fr.md)*

**Coursidor** is a variant of *Quoridor* played on graphs: two players race across the board to visit a set of objectives and come back, and they can place walls to cut edges and slow their opponent down.
This project implements the **game server**, the **boards** (as sparse graphs) and several **AI players**, loaded at runtime as shared libraries.

<p align="center">
  <img src="report/img/triangular5.png" height="230" alt="Triangular board">
  <img src="report/img/circular5.png" height="230" alt="Circular board">
  <img src="report/img/holey9.png" height="230" alt="Holey board">
</p>
<p align="center"><em>The three board types: triangular, circular and holey (red = objectives).</em></p>

## Features

- **Game server** that loads two players with `dlopen`, enforces the rules, runs series of games and exports every move as a Graphviz DOT file
- **Boards** generated programmatically (triangular, circular, holey) and stored as sparse adjacency matrices with the GNU Scientific Library (CSR format)
- **AI players**, each built as a `.so` plug-in:
  - `yani` — **minimax with alpha–beta pruning** on simulated game states; the evaluation function is the difference between both players' shortest remaining tours, computed by a BFS over *(position, direction, visited-objectives)* states (a small TSP)
  - `tristan` — greedy player that picks the move maximising its BFS-distance lead over the opponent
  - `steven` — naive baseline used to test the server
- **Visualisation**: each game can be rendered as a multi-page PDF (one page per move) with `neato`
- **Tests** with `gcov` coverage, and a LaTeX report (`report/`) with correctness and complexity proofs for the board constructions

## Build & run

Requirements: `gcc`, `make`, [GSL](https://www.gnu.org/software/gsl/) (`libgsl-dev`), and optionally Graphviz and `pdftk` for visualisation.

```bash
make GSL_PATH=/usr                     # builds install/server, install/yani.so, install/tristan.so
./install/server ./install/yani.so ./install/tristan.so -tH -m6 -g10
```

| Option | Meaning |
|--------|---------|
| `-t[T\|C\|H]` | board type: triangular, circular, holey |
| `-m<n>` | board size |
| `-M<n>` | maximum number of turns |
| `-g<n>` | number of games played in a row |
| `-s<n>` | random seed |
| `-p` | print every move |

Render the last game: `./src/generate-multi-graph.sh game.dot` (produces `output.pdf`).
Run the tests: `make test`.

## Project structure

```
src/
├── server.c                 Game loop, rule checking, DOT export
├── board.c, graph.h         Board generation (sparse graphs with GSL)
├── yani.c, opt.c, state.c   Minimax / alpha–beta player and game-state simulation
├── tsp.c, dijkstra.c        Shortest paths and objective tours
├── tristan.c, steven.c      Other players
test/                        Unit tests
report/                      LaTeX report
```

## Context

Imperative-programming project at [ENSEIRB-MATMECA](https://enseirb-matmeca.bordeaux-inp.fr/) (Bordeaux INP), 1st year, 2024–2025.
Team: Lucie Pacou, Solan Boher-Sanchez, Tristan Collemiche, Yani Haouchine.
