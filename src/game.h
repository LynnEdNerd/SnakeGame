#ifndef GAME_H
#define GAME_H

#include <random>
#include <memory>
#include "SDL.h"
#include "controller.h"
#include "renderer.h"
#include "snake.h"
#include "disk.h"

class BaseGame {
  public:
    virtual void Run(Controller &controller, std::unique_ptr<Renderer> renderer, std::size_t target_frame_duration) = 0;
};

class Game :  public BaseGame {
 public:
  Game(std::size_t grid_width, std::size_t grid_height, Disk &&disk);
  
  void Run(Controller &controller, std::unique_ptr<Renderer> renderer,  std::size_t target_frame_duration) override;

  int GetScore() const;
  int GetSize() const;
  std::size_t foo;

 private:
  std::shared_ptr<Snake> _snake;
  std::unique_ptr<Renderer> _renderer;
  // std::unique_ptr<Disk> _disk;
  Disk _disk;


  std::random_device dev;
  std::mt19937 engine;
  std::uniform_int_distribution<int> random_w;
  std::uniform_int_distribution<int> random_h;
  int _highScore;
  int score{0};

  void PlaceFood(Renderer * renderer);
  void Update(Renderer * renderer);
  void ResetToNewGame();
};

#endif