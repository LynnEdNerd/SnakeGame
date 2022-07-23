#include "game.h"
#include <iostream>
#include <future>
#include <thread>
#include <memory>
#include <chrono>
#include "SDL.h"



Game::Game(std::size_t grid_width, std::size_t grid_height, Disk &&disk)
    : 
      engine(dev()),
      random_w(0, static_cast<int>(grid_width - 1)),
      random_h(0, static_cast<int>(grid_height - 1)),
      _disk(disk)
      {
  _snake = std::make_shared<Snake>(grid_width, grid_height);
  
}


// This method resets the state information when the user chooses to play an additional game.
void Game::ResetToNewGame()
{
  score = 0;
  Game::PlaceFood(Game::_renderer.get());
  Game::_snake->ResetSnake();
}

// This is the dispatch loop for the game.  It initiates 2 threads:  1) Controller::HandleInput() and 2) Renderer::Render(). It stays active until either the user
// chooses against playing a new game OR the user shuts down the game window.  
void Game::Run(Controller &controller, std::unique_ptr<Renderer> renderer, std::size_t target_frame_duration) {
  Uint32 title_timestamp = SDL_GetTicks();
  Uint32 frame_start;
  Uint32 frame_end;
  Uint32 frame_duration;
  int frame_count = 0;
  // running is an indicator of whether the user has shut down the game window.  It remains true until the user requests shutting the game window.
  bool running = true;
 
  _renderer = std::move(renderer);

  Game::_highScore = _disk.readHighScore();
  PlaceFood( _renderer.get() );

  // Start Render in a thread.
  std::thread renderThread = std::thread (&Renderer::Render, _renderer.get(), _snake);

  do {
    // Start HandleInput in a thread.  The gameEndPromise variable is a communication mechanism to enable the Controller::HandleInput method, running in a thread,
    // to communicate that the user has terminated play (i.e., he or she pushed the "x" on the game window title bar.)
    std::promise<void> gameEndInputPromise;
    std::future<void> gameEndInputFuture = gameEndInputPromise.get_future();
    // The snakeDiedFuture is passed to the Controller::HandleInput thread to communicate that the snake has died andthe current game has ended.  This allows the 
    // HandleInput thread to exit.
    std::promise<void> snakeDiedPromise;
    std::future<void> snakeDiedFuture = snakeDiedPromise.get_future();
    std::thread inputThread = std::thread(&Controller::HandleInput , &controller, _snake, std::move(gameEndInputPromise), std::move(snakeDiedFuture));

    // Remain in the while loop as long as the user has not terminated the game.  In this case pushing the "x" on the game window or ^c on the keyboard.
    while ( running && _snake->alive) {
      
      // Check to see if the user shut down the game by closing the window.
      if (gameEndInputFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
        gameEndInputFuture.get();
        running = false;
      }

      frame_start = SDL_GetTicks();

      // Update the game state - snake head, snake body, and food location.  Detect if snake has "eaten" food and if the snake head has collided with the snake body.
      Update(_renderer.get());
      if (!_snake->alive) {
        // Signal the Controller::HandleInput thread the snake died.  This will cause the thread to terminate since there is no more need for input to guide the
        // snake movement.
        snakeDiedPromise.set_value();
      }

      // With the game state information updated, request the render thread to update the screen.  renderCompletePromise is created and a pointer to it is passed 
      // to the render thread. This is needed due to the fact that occasionally the rendering takes longer than the target_frame_duration.  This is seen
      // at startup where threads are being invoked and the SDL system is getting up and running.  Since this is being run on a multi tasking operating system, the 
      // potential for this scenario, though very unlikely, can happen at any time in a heavily used system.
      std::promise<void> renderCompletePromise;
      std::future<void> renderCompleteFuture = renderCompletePromise.get_future();
      _renderer->RegisterNewRenderRequest(&renderCompletePromise);
      
      frame_end = SDL_GetTicks();

      // Keep track of how long each loop through the input/update/render cycle
      // takes.
      frame_count++;
      frame_duration = frame_end - frame_start;

      // After every 500 milliseconds, update the window title.
      if (frame_end - title_timestamp >= 500) {
        // Multiply the frame_count by 2.  The sampliing is occuring every 500 milliseconds.  Double it to yield the frames per seconds.
        _renderer->UpdateWindowTitle(score, frame_count * 2, _highScore);
        frame_count = 0;
        title_timestamp = frame_end;
      }

      // If the time for this frame is too small (i.e. frame_duration is
      // smaller than the target ms_per_frame), delay the loop to
      // achieve the correct frame rate.
      if (frame_duration < target_frame_duration) {
          SDL_Delay(target_frame_duration - frame_duration);
      }
      // Wait for the rendering to complete.  This is an issue during the first couple of frames of the game.  However, the possibility of the render
      // thread getting delayed is always present, so this synchronizes the game with the render thread.
      renderCompleteFuture.wait();
    
    }

    if (score > _highScore) {
      _highScore = score;
      _disk.writeHighScore(_highScore);
    }

    if (running) { 
      // The snake has died, and now display to the user the choice to start a new game or end and exit the application.
      _renderer->DisplayPromptForNewGame(); 
    }
    
    inputThread.join();

    if (running) {
       if (controller.GetUserOkForNewGame()) {
        // Since the user requested a new game, reset the state information (i.e,, snake head and body, food location, and score)
        ResetToNewGame();
      } else {
        // User no longer wishes to play, so exit the application.
        running = false;
      }
    }
  } while (running);

  // Inform the Renderer::Render() method that it needs to stop and exit.
  _renderer->RegisterRenderTerminateRequest();
  renderThread.join();
}

void Game::PlaceFood(Renderer *renderer) {
  int x, y;
  while (true) {
    x = random_w(engine);
    y = random_h(engine);
    // Check that the location is not occupied by a snake item before placing
    // food.
    if (!_snake->SnakeCell(x, y)) {
      renderer->SetFoodCoordinates(x, y);
      return;
    }
  }
}

void Game::Update(Renderer *renderer) {
  if (!_snake->alive) {
    return;
  }

  // Protect access to the snake and food as they are being updated so that 
  // any potential real time interactions with the rendering process and input process are eliminated.
  std::lock_guard<std::mutex> snakeUpdateProtect(_snake->snakeMutex);
  _snake->Update();

  int new_x = static_cast<int>(_snake->GetSnakeHeadX());
  int new_y = static_cast<int>(_snake->GetSnakeHeadY());

  // Check if there's food over here
  if (renderer->GetFoodXCoordinate() == new_x && renderer->GetFoodYCoordinate() == new_y) {
    score++;
    PlaceFood(renderer);
    // Grow snake and increase speed.
    _snake->GrowBody();
    _snake->speed += 0.005;
  }
}

int Game::GetScore() const { return score; }
int Game::GetSize() const { return _snake->size; }