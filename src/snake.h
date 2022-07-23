#ifndef SNAKE_H
#define SNAKE_H

#include <vector>
#include <mutex>
#include <iostream>
#include "SDL.h"

class Snake {
 public:
  enum class Direction { kUp, kDown, kLeft, kRight };

  Snake(int grid_width, int grid_height)
      : grid_width(grid_width),
        grid_height(grid_height),
        _head_x(grid_width / 2),
        _head_y(grid_height / 2) {}

  void Update();

  void GrowBody();
  bool SnakeCell(int x, int y);
  void ResetSnake();
  float GetSnakeHeadX() {return _head_x;}
  float GetSnakeHeadY() {return _head_y;}

  Direction direction = Direction::kUp;

  float speed{0.1f};
  int size{1};
  bool alive{true};
  std::vector<SDL_Point> body;
  std::mutex snakeMutex;
  
 private:
  void UpdateHead();
  void UpdateBody(SDL_Point &current_cell, SDL_Point &prev_cell);

  bool growing{false};
  int grid_width;
  int grid_height;
  float _head_x;
  float _head_y;

};

#endif