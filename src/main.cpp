#include "Maze.h"
#include "Visualizer.h"

int main() {
    // 24 rows x 40 cols matches the Visualizer's layout constants.
    Maze maze(24, 40);
    maze.generateRandom();          // start with a solvable random maze

    Visualizer viz(maze);
    viz.run();
    return 0;
}