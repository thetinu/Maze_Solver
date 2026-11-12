#pragma once

#include <vector>

// A cell is identified by its (row, col) grid coordinates.
struct Cell {
    int row = -1;
    int col = -1;

    bool operator==(const Cell& o) const { return row == o.row && col == o.col; }
    bool operator!=(const Cell& o) const { return !(*this == o); }
    
    // Added operator< so std::pair<int, Cell> can be compared in the priority queue
    bool operator<(const Cell& o) const {
        if (row != o.row) return row < o.row;
        return col < o.col;
    }
    
    bool valid() const { return row >= 0 && col >= 0; }
};

enum class CellType : unsigned char {
    Empty,
    Wall,
    Start,
    End
};

// ... (rest of the Maze.h file remains exactly the same)

// Maze owns the 2-D grid and exposes graph-like operations (neighbors,
// bounds checks) that the algorithms use. It knows nothing about SFML.
class Maze {
public:
    Maze(int rows, int cols);

    int rows() const { return rows_; }
    int cols() const { return cols_; }

    Cell start() const { return start_; }
    Cell end()   const { return end_; }

    CellType cellType(Cell c) const;
    bool     isWall  (Cell c) const;
    bool     inBounds(Cell c) const;

    void setWall(Cell c, bool wall);
    void setStart(Cell c);
    void setEnd  (Cell c);

    // 4-directional neighbors (up, down, left, right) that lie in bounds.
    std::vector<Cell> neighbors(Cell c) const;

    // Replace the grid with a new random layout. The generated maze is
    // guaranteed to be solvable (start can reach end).
    void generateRandom();

    // Remove all walls; keep start and end in place.
    void clearWalls();

private:
    int rows_, cols_;
    Cell start_, end_;
    std::vector<std::vector<CellType>> grid_;

    bool isSolvable() const;
};