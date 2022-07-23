#include "controller.h"
#include <iostream>
#include <future>
#include "SDL.h"
#include "snake.h"

void Controller::ChangeDirection(std::shared_ptr<Snake> snake, Snake::Direction input,
                                 Snake::Direction opposite) const {
  if (snake->direction != opposite || snake->size == 1) {
    // Use the mutuex when changing the snake's direction since the Game::Update() and its call tree use snake->direction in making calculations.
    std::lock_guard<std::mutex> snakeLock(snake->snakeMutex);
    snake->direction = input;
  }
  return;
}


// This method is run in a thread initiated by Game::Run().  It will stay in the while loop until either 1) the host closes the game window by pressing "x"
// on the game window or 2) the snake dies.  The closing of the window is signalled to gameRun() by setting the gameEndInputPromise.  The death of the snake
// is signalled from Game::Run() with the snakeDied future.  Both of these events will cause Controller::HandleInput() to exit and thus terminate the thread.
void Controller::HandleInput( std::shared_ptr<Snake> snake,  std::promise<void> &&gameEndInputPromise, std::future<void> &&snakeDiedFuture) const {
  SDL_Event e;

  while (SDL_WaitEvent(&e) && snakeDiedFuture.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
    if (e.type == SDL_QUIT) {
      // In this scenario the user has pushed the "x" on the game window to shut the game down.  Set the promise to indicate to the Game::Run() method
      // that the user is exiting the game and then exit this routine and thread.
      gameEndInputPromise.set_value();
      break;
      // running = false;
    } else if (e.type == SDL_KEYDOWN) {
      switch (e.key.keysym.sym) {
        case SDLK_UP:
          ChangeDirection(snake, Snake::Direction::kUp,
                          Snake::Direction::kDown);
          break;

        case SDLK_DOWN:
          ChangeDirection(snake, Snake::Direction::kDown,
                          Snake::Direction::kUp);
          break;

        case SDLK_LEFT:
          ChangeDirection(snake, Snake::Direction::kLeft,
                          Snake::Direction::kRight);
          break;

        case SDLK_RIGHT:
          ChangeDirection(snake, Snake::Direction::kRight,
                          Snake::Direction::kLeft);
          break;
      } 
    }
  }

  // Get the snakeDied future prior to exiting the routine and thread if the promise is set.  If this HandleInput is being exited because the user
  // requested the game window closed, the snakeDiedFuture will not be set.
  if (snakeDiedFuture.wait_for(std::chrono::milliseconds(0))== std::future_status::ready) {
    snakeDiedFuture.get();
  }
}

// This routine is called after a game has been completed.  It returns true if the user pushes "y" and false if the user pushes either "n" OR shuts down the
// game window by pushing "x" in the game title bar.
bool Controller::GetUserOkForNewGame()
{
  SDL_Event e;
  bool returnValue = false;

  // lookingForInput remains true until either "y" or "n" is pushed.  
  bool lookingForInput = true;

  while(lookingForInput) {
    SDL_WaitEvent(&e);
    if (e.type != SDL_QUIT) {
      switch (e.key.keysym.sym) {
      // case SDLK_Y:
        case SDLK_y:
          // In this case the user desires to play another game.  Return true.
          lookingForInput = false;
          returnValue = true;
          break;
        case SDLK_n:
          // In this case the user does not desire to play another game.  Return false.
          lookingForInput = false;
          break;
        default:
          // Something other than "y" or "n" received.  Keep looking for input.
          break;
      }
    } else {
      // This occurs when the user has shut down the game window.  Exit the while loop and return false.
      break;
    }
  }
  return returnValue;

}


