#include <iostream>
#include "controller.h"
#include "game.h"
#include "renderer.h"
#include "disk.h"
#include <memory>

int main() {
  constexpr std::size_t kFramesPerSecond{60};
  constexpr std::size_t kMsPerFrame{1000 / kFramesPerSecond};
  constexpr std::size_t kScreenWidth{640};
  constexpr std::size_t kScreenHeight{640};
  constexpr std::size_t kGridWidth{32};
  constexpr std::size_t kGridHeight{32};

  std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>(kScreenWidth, kScreenHeight, kGridWidth, kGridHeight);
  Controller controller;
  Disk disk = Disk("./HighScore");
  Game game(kGridWidth, kGridHeight, std::move(disk));
  game.Run(controller, std::move(renderer), kMsPerFrame);
  std::cout << "Game has terminated successfully!\n";
  std::cout << "Score: " << game.GetScore() << "\n";
  std::cout << "Size: " << game.GetSize() << "\n";
  return 0;
}
