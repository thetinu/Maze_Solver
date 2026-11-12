#include "Algorithms.h"

#include <chrono>
#include <limits>
#include <queue>
#include <stack>
#include <utility>
#include <vector>

using Clock = std::chrono::high_resolution_clock;

namespace {

// Reconstruct the path by walking parent pointers from end back to start.
std::vector<Cell> reconstruct(const Cell& start, const Cell& end,
                              const std::vector<std::vector<Cell>>& parent) {
    std::vector<Cell> path;
    if (!parent[end.row][end.col].valid()) return path;   // unreachable

    Cell cur = end;
    while (cur != start) {
        path.push_back(cur);
        cur = parent[cur.row][cur.col];
        if (!cur.valid()) return {};   // safety: broken chain
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}

} // namespace

// ---------------------------------------------------------------------------
// BFS — Queue, level-by-level expansion, shortest path on an unweighted grid.
// ---------------------------------------------------------------------------
AlgorithmResult Algorithms::runBFS(const Maze& maze) {
    AlgorithmResult res;
    res.name = "BFS";

    const int R = maze.rows(), C = maze.cols();
    std::vector<std::vector<bool>>   visited(R, std::vector<bool>(C, false));
    std::vector<std::vector<Cell>>   parent (R, std::vector<Cell>(C, Cell{-1,-1}));

    std::queue<Cell> q;                      // <-- the Queue (DSA concept)
    q.push(maze.start());
    visited[maze.start().row][maze.start().col] = true;

    auto t0 = Clock::now();

    while (!q.empty()) {
        Cell c = q.front(); q.pop();
        res.visitOrder.push_back(c);
        ++res.nodesVisited;

        if (c == maze.end()) break;

        for (Cell n : maze.neighbors(c)) {
            if (!visited[n.row][n.col] && !maze.isWall(n)) {
                visited[n.row][n.col] = true;
                parent[n.row][n.col] = c;      // parent tracking
                q.push(n);
            }
        }
    }

    auto t1 = Clock::now();
    res.timeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    res.path = reconstruct(maze.start(), maze.end(), parent);
    res.pathFound = !res.path.empty();
    return res;
}

// ---------------------------------------------------------------------------
// DFS — Iterative Stack, deep exploration with backtracking.
// NOTE: DFS does NOT guarantee the shortest path.
// ---------------------------------------------------------------------------
AlgorithmResult Algorithms::runDFS(const Maze& maze) {
    AlgorithmResult res;
    res.name = "DFS";

    const int R = maze.rows(), C = maze.cols();
    std::vector<std::vector<bool>> visited(R, std::vector<bool>(C, false));
    std::vector<std::vector<Cell>> parent (R, std::vector<Cell>(C, Cell{-1,-1}));

    std::stack<Cell> s;                      // <-- the Stack (DSA concept)
    s.push(maze.start());

    auto t0 = Clock::now();

    while (!s.empty()) {
        Cell c = s.top(); s.pop();

        if (visited[c.row][c.col]) continue; // lazy duplicate discard
        visited[c.row][c.col] = true;
        res.visitOrder.push_back(c);
        ++res.nodesVisited;

        if (c == maze.end()) break;

        for (Cell n : maze.neighbors(c)) {
            if (!visited[n.row][n.col] && !maze.isWall(n)) {
                parent[n.row][n.col] = c;    // parent tracking
                s.push(n);
            }
        }
    }

    auto t1 = Clock::now();
    res.timeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    res.path = reconstruct(maze.start(), maze.end(), parent);
    res.pathFound = !res.path.empty();
    return res;
}

// ---------------------------------------------------------------------------
// Dijkstra — Priority Queue, shortest path for non-negative edge weights.
// With uniform costs this degenerates to BFS in behavior, but the priority
// queue is exercised exactly as the textbook algorithm specifies.
// ---------------------------------------------------------------------------
AlgorithmResult Algorithms::runDijkstra(const Maze& maze) {
    AlgorithmResult res;
    res.name = "Dijkstra";

    const int R = maze.rows(), C = maze.cols();
    const int INF = std::numeric_limits<int>::max();

    std::vector<std::vector<int>>  dist  (R, std::vector<int>(C, INF));
    std::vector<std::vector<bool>> closed(R, std::vector<bool>(C, false));
    std::vector<std::vector<Cell>> parent(R, std::vector<Cell>(C, Cell{-1,-1}));

    using Entry = std::pair<int, Cell>;                 // (distance, cell)
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pq;  // <-- Priority Queue

    dist[maze.start().row][maze.start().col] = 0;
    pq.push({0, maze.start()});

    auto t0 = Clock::now();

    while (!pq.empty()) {
        auto [d, c] = pq.top(); pq.pop();

        if (closed[c.row][c.col]) continue;             // stale entry
        closed[c.row][c.col] = true;
        res.visitOrder.push_back(c);
        ++res.nodesVisited;

        if (c == maze.end()) break;

        for (Cell n : maze.neighbors(c)) {
            if (maze.isWall(n) || closed[n.row][n.col]) continue;
            int nd = d + 1;                              // uniform edge weight
            if (nd < dist[n.row][n.col]) {
                dist[n.row][n.col] = nd;
                parent[n.row][n.col] = c;
                pq.push({nd, n});
            }
        }
    }

    auto t1 = Clock::now();
    res.timeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    res.path = reconstruct(maze.start(), maze.end(), parent);
    res.pathFound = !res.path.empty();
    return res;
}