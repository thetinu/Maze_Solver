#include "Maze.h"

#include <cstdlib>
#include <ctime>
#include <queue>
#include <random>

Maze::Maze(int rows, int cols)
    : rows_(rows), cols_(cols),
      grid_(rows, std::vector<CellType>(cols, CellType::Empty))
{
    // Sensible default start / end positions, well inside the border.
    start_ = {2, 2};
    end_   = {rows - 3, cols - 3};
    grid_[start_.row][start_.col] = CellType::Start;
    grid_[end_.row][end_.col]     = CellType::End;
}

CellType Maze::cellType(Cell c) const {
    if (!inBounds(c)) return CellType::Wall;   // out-of-bounds treated as wall
    return grid_[c.row][c.col];
}

bool Maze::isWall(Cell c) const {
    return cellType(c) == CellType::Wall;
}

bool Maze::inBounds(Cell c) const {
    return c.row >= 0 && c.row < rows_ && c.col >= 0 && c.col < cols_;
}

void Maze::setWall(Cell c, bool wall) {
    if (!inBounds(c)) return;
    if (grid_[c.row][c.col] == CellType::Start ||
        grid_[c.row][c.col] == CellType::End) return;   // never overwrite terminals
    grid_[c.row][c.col] = wall ? CellType::Wall : CellType::Empty;
}

void Maze::setStart(Cell c) {
    if (!inBounds(c)) return;
    grid_[start_.row][start_.col] = CellType::Empty;
    start_ = c;
    grid_[c.row][c.col] = CellType::Start;
}

void Maze::setEnd(Cell c) {
    if (!inBounds(c)) return;
    grid_[end_.row][end_.col] = CellType::Empty;
    end_ = c;
    grid_[c.row][c.col] = CellType::End;
}

std::vector<Cell> Maze::neighbors(Cell c) const {
    static const int dr[] = {-1, 1, 0, 0};
    static const int dc[] = { 0, 0,-1, 1};
    std::vector<Cell> out;
    out.reserve(4);
    for (int k = 0; k < 4; ++k) {
        Cell n{c.row + dr[k], c.col + dc[k]};
        if (inBounds(n)) out.push_back(n);
    }
    return out;
}

bool Maze::isSolvable() const {
    // Quick BFS reachability check from start to end.
    std::vector<std::vector<bool>> seen(rows_, std::vector<bool>(cols_, false));
    std::queue<Cell> q;
    q.push(start_);
    seen[start_.row][start_.col] = true;
    while (!q.empty()) {
        Cell c = q.front(); q.pop();
        if (c == end_) return true;
        for (Cell n : neighbors(c)) {
            if (!seen[n.row][n.col] && !isWall(n)) {
                seen[n.row][n.col] = true;
                q.push(n);
            }
        }
    }
    return false;
}

void Maze::generateRandom() {
    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    // Try a few times; random walls can occasionally disconnect the grid.
    for (int attempt = 0; attempt < 50; ++attempt) {
        for (int r = 0; r < rows_; ++r) {
            for (int c = 0; c < cols_; ++c) {
                grid_[r][c] = (dist(rng) < 0.30) ? CellType::Wall : CellType::Empty;
            }
        }
        grid_[start_.row][start_.col] = CellType::Start;
        grid_[end_.row][end_.col]     = CellType::End;

        if (isSolvable()) return;
    }
    // Fallback: completely open grid (always solvable).
    clearWalls();
}

void Maze::clearWalls() {
    for (int r = 0; r < rows_; ++r)
        for (int c = 0; c < cols_; ++c)
            if (grid_[r][c] == CellType::Wall)
                grid_[r][c] = CellType::Empty;
}