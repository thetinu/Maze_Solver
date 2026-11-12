#pragma once

#include "Maze.h"
#include <string>
#include <vector>

// Everything an algorithm produces, in a form the Visualizer can animate.
struct AlgorithmResult {
    std::string name;              // "BFS", "DFS", "Dijkstra"
    std::vector<Cell> visitOrder;  // cells in the exact order they were first visited
    std::vector<Cell> path;        // reconstructed path from start to end (empty if none)
    bool   pathFound = false;
    int    nodesVisited = 0;
    double timeMs = 0.0;           // algorithm execution time (excludes animation)
};

// Each algorithm is a pure function over the Maze. No SFML, no globals.
class Algorithms {
public:
    static AlgorithmResult runBFS     (const Maze& maze);
    static AlgorithmResult runDFS     (const Maze& maze);
    static AlgorithmResult runDijkstra(const Maze& maze);
};