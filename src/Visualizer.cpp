#include "Visualizer.h"

#include <algorithm>
#include <iostream>
#include <sstream>

// ---- palette ----
namespace Col {
    const sf::Color Background{ 30,  30,  35};
    const sf::Color Empty    {235, 235, 240};
    const sf::Color Wall     { 45,  45,  50};
    const sf::Color Start    { 76, 175,  80};
    const sf::Color End      {244,  67,  54};
    const sf::Color Visited  {100, 181, 246};
    const sf::Color Path     {255, 193,   7};
    const sf::Color Head     {255, 152,   0};   // current "frontier" cell
    const sf::Color PanelBg  { 40,  40,  46};
    const sf::Color BtnIdle  { 70,  70,  78};
    const sf::Color BtnHover { 95,  95, 105};
    const sf::Color BtnSel   { 33, 150, 243};
    const sf::Color BtnDis   { 55,  55,  60};
    const sf::Color Text     {240, 240, 240};
    const sf::Color TextDim  {170, 170, 175};
}

Visualizer::Visualizer(Maze& maze)
    : window_(sf::VideoMode(WIN_W, WIN_H), "Maze Solver & Pathfinding Visualizer"),
      maze_(maze)
{
    window_.setFramerateLimit(60);
    loadFont();
    resetOverlay();
    buildUI();
}

// ------------------------------------------------------------------ setup
void Visualizer::loadFont() {
    // Windows ships with Arial; adjust the path if your system differs.
    const std::vector<std::string> candidates = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "arial.ttf"
    };
    for (const auto& p : candidates)
        if (font_.loadFromFile(p)) return;

    std::cerr << "Warning: no TTF font found. Text will be blank.\n";
}

void Visualizer::addBtn(int x, int y, int w, int h,
                        const std::string& text, std::function<void()> onClick)
{
    Button b;
    b.box.setSize(sf::Vector2f(static_cast<float>(w), static_cast<float>(h)));
    b.box.setPosition(static_cast<float>(x), static_cast<float>(y));
    b.box.setFillColor(Col::BtnIdle);
    b.box.setOutlineColor(Col::Text);
    b.box.setOutlineThickness(1.f);

    b.label.setFont(font_);
    b.label.setString(text);
    b.label.setCharacterSize(16);
    b.label.setFillColor(Col::Text);

    sf::FloatRect tb = b.label.getLocalBounds();
    b.label.setOrigin(tb.width / 2.f, tb.height / 2.f);
    b.label.setPosition(b.box.getPosition().x + w / 2.f,
                        b.box.getPosition().y + h / 2.f);
    b.onClick = std::move(onClick);
    buttons_.push_back(std::move(b));
}

void Visualizer::buildUI() {
    const int px = GRID_W + 20;   // panel left edge
    const int bw = UI_W - 40;     // button width
    const int bh = 38;
    const int gap = 10;
    int y = 20;

    // Algorithm selection
    titleText_.setFont(font_);
    titleText_.setString("Maze Solver");
    titleText_.setCharacterSize(22);
    titleText_.setFillColor(Col::Text);
    titleText_.setPosition(static_cast<float>(px), static_cast<float>(y));
    y += 40;

    // Buttons are now spaced out more evenly
    addBtn(px, y, bw, bh, "BFS",      [this]{ actSelectBFS();      }); y += bh + gap;
    addBtn(px, y, bw, bh, "DFS",      [this]{ actSelectDFS();      }); y += bh + gap;
    addBtn(px, y, bw, bh, "Dijkstra", [this]{ actSelectDijkstra(); }); y += bh + gap + 15;

    addBtn(px, y, bw, bh, "Generate Maze", [this]{ actGenerate(); }); y += bh + gap;
    addBtn(px, y, bw, bh, "Clear Walls",   [this]{ actClear();    }); y += bh + gap + 15;

    addBtn(px, y, bw, bh + 8, "Start",  [this]{ actStart();  }); y += bh + gap + 8;
    addBtn(px, y, bw, bh,     "Reset",  [this]{ actReset();  }); y += bh + gap + 25;

    // Stats text block - MOVED DOWN to avoid overlap
    statsText_.setFont(font_);
    statsText_.setCharacterSize(14);
    statsText_.setFillColor(Col::Text);
    statsText_.setPosition(static_cast<float>(px), 448.f); // <-- Changed to 448

    actSelectBFS();   // default selection
}

void Visualizer::resetOverlay() {
    visitedAt_.assign(maze_.rows(), std::vector<int>(maze_.cols(), -1));
    onPath_   .assign(maze_.rows(), std::vector<bool>(maze_.cols(), false));
}

// -------------------------------------------------------------- main loop
void Visualizer::run() {
    while (window_.isOpen()) {
        handleEvents();
        update();
        draw();
    }
}

void Visualizer::handleEvents() {
    sf::Event e;
    while (window_.pollEvent(e)) {
        if (e.type == sf::Event::Closed) { window_.close(); return; }

        if (e.type == sf::Event::MouseButtonPressed) {
            int mx = e.mouseButton.x, my = e.mouseButton.y;

            // Button clicks take priority.
            bool consumed = false;
            for (auto& b : buttons_) {
                if (!b.enabled) continue;
                if (b.box.getGlobalBounds().contains(static_cast<float>(mx),
                                                     static_cast<float>(my))) {
                    b.onClick();
                    consumed = true;
                    break;
                }
            }
            if (!consumed && state_ == State::Idle) {
                if (e.mouseButton.button == sf::Mouse::Left) {
                    leftDown_ = true;
                    handleGridClick(mx, my, true);
                } else if (e.mouseButton.button == sf::Mouse::Right) {
                    rightDown_ = true;
                    handleGridClick(mx, my, false);
                }
            }
        }
        else if (e.type == sf::Event::MouseButtonReleased) {
            if (e.mouseButton.button == sf::Mouse::Left)  leftDown_  = false;
            if (e.mouseButton.button == sf::Mouse::Right) rightDown_ = false;
        }
        else if (e.type == sf::Event::MouseMoved && state_ == State::Idle) {
            if (leftDown_)  handleGridClick(e.mouseMove.x, e.mouseMove.y, true);
            if (rightDown_) handleGridClick(e.mouseMove.x, e.mouseMove.y, false);
        }
    }
}

void Visualizer::update() {
    if (state_ == State::Visiting) {
        if (animClock_.getElapsedTime().asMilliseconds() >= stepDelayMs_) {
            animClock_.restart();
            if (stepIndex_ < visitOrder_.size()) {
                Cell c = visitOrder_[stepIndex_++];
                visitedAt_[c.row][c.col] = static_cast<int>(stepIndex_);
            } else {
                // Visit phase done -> start drawing the path.
                state_ = State::DrawingPath;
                stepIndex_ = 0;
                animClock_.restart();
            }
        }
    }
    else if (state_ == State::DrawingPath) {
        if (animClock_.getElapsedTime().asMilliseconds() >= stepDelayMs_ * 1.5f) {
            animClock_.restart();
            if (stepIndex_ < finalPath_.size()) {
                Cell c = finalPath_[stepIndex_++];
                onPath_[c.row][c.col] = true;
            } else {
                state_ = State::Done;
                setButtonsEnabled(true);
            }
        }
    }
}

void Visualizer::draw() {
    window_.clear(Col::Background);
    drawGrid();
    drawUIPanel();
    drawStats();
    window_.display();
}

// ---------------------------------------------------------------- drawing
sf::Color Visualizer::colorFor(Cell c) const {
    if (onPath_[c.row][c.col])                         return Col::Path;
    int v = visitedAt_[c.row][c.col];
    if (v >= 0 && static_cast<std::size_t>(v) == stepIndex_ &&
        (state_ == State::Visiting || state_ == State::DrawingPath))
        return Col::Head;                              // frontier highlight
    if (v >= 0)                                        return Col::Visited;

    switch (maze_.cellType(c)) {
        case CellType::Wall:  return Col::Wall;
        case CellType::Start: return Col::Start;
        case CellType::End:   return Col::End;
        case CellType::Empty: return Col::Empty;
    }
    return Col::Empty;
}

void Visualizer::drawGrid() {
    sf::RectangleShape cell(sf::Vector2f(CELL - 1, CELL - 1));
    for (int r = 0; r < maze_.rows(); ++r) {
        for (int c = 0; c < maze_.cols(); ++c) {
            Cell cc{r, c};
            cell.setPosition(static_cast<float>(c * CELL),
                             static_cast<float>(r * CELL));
            cell.setFillColor(colorFor(cc));
            window_.draw(cell);
        }
    }
}

void Visualizer::drawUIPanel() {
    sf::RectangleShape panel(sf::Vector2f(static_cast<float>(UI_W),
                                          static_cast<float>(WIN_H)));
    panel.setPosition(static_cast<float>(GRID_W), 0.f);
    panel.setFillColor(Col::PanelBg);
    window_.draw(panel);

    // Buttons (with hover + selected styling).
    sf::Vector2i mp = sf::Mouse::getPosition(window_);
    for (auto& b : buttons_) {
        bool hover = b.box.getGlobalBounds().contains(static_cast<float>(mp.x),
                                                      static_cast<float>(mp.y));
        if (!b.enabled)                 b.box.setFillColor(Col::BtnDis);
        else if (b.selected)            b.box.setFillColor(Col::BtnSel);
        else if (hover)                 b.box.setFillColor(Col::BtnHover);
        else                            b.box.setFillColor(Col::BtnIdle);

        b.label.setFillColor(b.enabled ? Col::Text : Col::TextDim);
        window_.draw(b.box);
        window_.draw(b.label);
    }

    // Legend swatches - MOVED DOWN to sit below the stats
    auto swatch = [&](int y, const std::string& name, sf::Color col) {
        sf::RectangleShape s(sf::Vector2f(16, 16));
        s.setPosition(static_cast<float>(GRID_W + 20), static_cast<float>(y));
        s.setFillColor(col);
        sf::Text t; t.setFont(font_); t.setString(name);
        t.setCharacterSize(13); t.setFillColor(Col::TextDim);
        t.setPosition(static_cast<float>(GRID_W + 44), static_cast<float>(y - 2));
        window_.draw(s); window_.draw(t);
    };
    
    // Legend starts at 540, well below the stats text
    int ly = 540; 
    swatch(ly,      "Start",   Col::Start);
    swatch(ly += 22,"End",     Col::End);
    swatch(ly += 22,"Wall",    Col::Wall);
    swatch(ly += 22,"Visited", Col::Visited);
    swatch(ly += 22,"Path",    Col::Path);

    // Usage hint - MOVED to the very bottom
    sf::Text hint; hint.setFont(font_); hint.setCharacterSize(12);
    hint.setFillColor(Col::TextDim);
    hint.setString("Left-click: add wall\nRight-click: erase wall");
    hint.setPosition(static_cast<float>(GRID_W + 20), static_cast<float>(650)); // <-- Changed to 650
    window_.draw(hint);
}

void Visualizer::drawStats() {
    if (!hasResult_) {
        statsText_.setString("Run an algorithm to see\nperformance statistics.");
    } else {
        std::ostringstream os;
        os << "Algorithm : " << lastResult_.name          << "\n"
           << "Visited   : " << lastResult_.nodesVisited  << "\n"
           << "Path len  : "
           << (lastResult_.pathFound ? std::to_string(lastResult_.path.size()) : std::string("-")) << "\n"
           << "Time      : " << lastResult_.timeMs << " ms\n"
           << "Path      : " << (lastResult_.pathFound ? "found" : "NOT found");
        statsText_.setString(os.str());
    }
    window_.draw(statsText_);
}

// ------------------------------------------------------------- interaction
void Visualizer::handleGridClick(int px, int py, bool makeWall) {
    if (px < 0 || px >= GRID_W || py < 0 || py >= GRID_H) return;
    Cell c{py / CELL, px / CELL};
    if (!maze_.inBounds(c)) return;
    if (maze_.cellType(c) == CellType::Start ||
        maze_.cellType(c) == CellType::End)   return;

    maze_.setWall(c, makeWall);
    resetOverlay();                 // any edit invalidates old visualization
    state_ = State::Idle;
    hasResult_ = false;
}

void Visualizer::setButtonsEnabled(bool enabled) {
    for (auto& b : buttons_) b.enabled = enabled;
    // Keep algorithm-selection buttons always enabled so the user can switch.
    buttons_[0].enabled = buttons_[1].enabled = buttons_[2].enabled = true;
}

// ---------------------------------------------------------- button actions
void Visualizer::actSelectBFS()      { selectedAlgo_ = Algo::BFS;      buttons_[0].selected = true;  buttons_[1].selected = false; buttons_[2].selected = false; }
void Visualizer::actSelectDFS()      { selectedAlgo_ = Algo::DFS;      buttons_[0].selected = false; buttons_[1].selected = true;  buttons_[2].selected = false; }
void Visualizer::actSelectDijkstra() { selectedAlgo_ = Algo::Dijkstra; buttons_[0].selected = false; buttons_[1].selected = false; buttons_[2].selected = true;  }

void Visualizer::actGenerate() {
    if (state_ != State::Idle && state_ != State::Done) return;
    maze_.generateRandom();
    resetOverlay();
    state_ = State::Idle;
    hasResult_ = false;
}

void Visualizer::actClear() {
    if (state_ != State::Idle && state_ != State::Done) return;
    maze_.clearWalls();
    resetOverlay();
    state_ = State::Idle;
    hasResult_ = false;
}

void Visualizer::actStart() {
    if (state_ == State::Visiting || state_ == State::DrawingPath) return;

    // Run the chosen algorithm to completion (very fast).
    switch (selectedAlgo_) {
        case Algo::BFS:      lastResult_ = Algorithms::runBFS     (maze_); break;
        case Algo::DFS:      lastResult_ = Algorithms::runDFS     (maze_); break;
        case Algo::Dijkstra: lastResult_ = Algorithms::runDijkstra(maze_); break;
    }
    hasResult_ = true;

    // Prepare animation buffers.
    resetOverlay();
    visitOrder_ = std::move(lastResult_.visitOrder);
    finalPath_  = lastResult_.path;     // keep a copy for the path phase
    stepIndex_  = 0;
    animClock_.restart();
    state_ = State::Visiting;

    // Disable Start/Generate/Clear/Reset during animation.
    setButtonsEnabled(false);
}

void Visualizer::actReset() {
    resetOverlay();
    visitOrder_.clear();
    finalPath_.clear();
    stepIndex_ = 0;
    state_ = State::Idle;
    setButtonsEnabled(true);
}