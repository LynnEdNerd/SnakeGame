#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <future>
#include "snake.h"

class Controller {
 public:
  void HandleInput(std::shared_ptr<Snake> snake, std::promise<void> &&inputPromise, std::future<void>&&snakeDied) const;
  bool GetUserOkForNewGame();

 private:
  void ChangeDirection(std::shared_ptr<Snake> snake, Snake::Direction input,
                       Snake::Direction opposite) const;

};

#endif
