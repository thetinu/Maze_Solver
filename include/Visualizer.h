#pragma once

#include "Maze.h"
#include "Algorithms.h"

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <vector>

class Visualizer {
public:
    explicit Visualizer(Maze& maze);
    void run();

private:
    // ---- layout constants ----
    static constexpr int CELL   = 25;        // pixel size of one cell
    static constexpr int COLS   = 40;
    static constexpr int ROWS   = 24;
    static constexpr int GRID_W = COLS * CELL;   // 1000
    static constexpr int GRID_H = ROWS * CELL;   // 600
    static constexpr int UI_W   = 280;
    static constexpr int WIN_W  = GRID_W + UI_W; // 1280
    static constexpr int WIN_H  = 720;

    // ---- animation states ----
    enum class State { Idle, Visiting, DrawingPath, Done };

    // ---- a clickable UI button ----
    struct Button {
        sf::RectangleShape box;
        sf::Text           label;
        std::function<void()> onClick;
        bool selected = false;
        bool enabled  = true;
    };

    // ---- core objects ----
    sf::RenderWindow window_;
    sf::Font         font_;
    Maze&            maze_;

    // ---- algorithm selection & results ----
    enum class Algo { BFS, DFS, Dijkstra };
    Algo selectedAlgo_ = Algo::BFS;
    AlgorithmResult lastResult_{};
    bool hasResult_ = false;

    // ---- animation ----
    State state_ = State::Idle;
    std::vector<Cell> visitOrder_;
    std::vector<Cell> finalPath_;
    std::size_t stepIndex_ = 0;
    sf::Clock   animClock_;
    float       stepDelayMs_ = 15.0f;

    // ---- per-cell overlay (what the user sees on top of the maze) ----
    //  -1  = no overlay
    // >=0  = cell was visited at this step index
    std::vector<std::vector<int>> visitedAt_;
    std::vector<std::vector<bool>> onPath_;

    // ---- UI ----
    std::vector<Button> buttons_;
    sf::Text statsText_;
    sf::Text titleText_;

    // ---- mouse drag state ----
    bool leftDown_  = false;
    bool rightDown_ = false;

    // ---- setup / lifecycle ----
    void loadFont();
    void buildUI();
    void addBtn(int x, int y, int w, int h,
                const std::string& text, std::function<void()> onClick);

    // ---- main loop phases ----
    void handleEvents();
    void update();
    void draw();

    // ---- drawing helpers ----
    void drawGrid();
    void drawUIPanel();
    void drawStats();
    sf::Color colorFor(Cell c) const;

    // ---- interaction ----
    void handleGridClick(int px, int py, bool makeWall);
    void resetOverlay();

    // ---- button actions ----
    void actSelectBFS();
    void actSelectDFS();
    void actSelectDijkstra();
    void actGenerate();
    void actClear();
    void actStart();
    void actReset();

    void setButtonsEnabled(bool enabled);
};