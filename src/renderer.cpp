#include "renderer.h"
#include <iostream>
#include <string>
#include <future>
#include <mutex>
#include <condition_variable>

Renderer::Renderer(const std::size_t screen_width,
                   const std::size_t screen_height,
                   const std::size_t grid_width, const std::size_t grid_height)
    : screen_width(screen_width),
      screen_height(screen_height),
      grid_width(grid_width),
      grid_height(grid_height) {
  
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize.\n";
    std::cerr << "SDL_Error: " << SDL_GetError() << "\n";
  }

  // Create Window
  sdl_window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, screen_width,
                                screen_height, SDL_WINDOW_SHOWN);

  if (nullptr == sdl_window) {
    std::cerr << "Window could not be created.\n";
    std::cerr << " SDL_Error: " << SDL_GetError() << "\n";
  }

  // Create renderer
  sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
  if (nullptr == sdl_renderer) {
    std::cerr << "Renderer could not be created.\n";
    std::cerr << "SDL_Error: " << SDL_GetError() << "\n";
  }
}

Renderer::~Renderer() {
  SDL_DestroyWindow(sdl_window);
  SDL_Quit();
}

// This method allows the caller to initiate a new render request to the Renderer:Render() method running in a thread.  Additionally, a pointer to a 
// renderPromiseComplete promise is sent.  Renderere::Render() uses this to indicate that the requested 
void Renderer::RegisterNewRenderRequest(std::promise<void> * renderCompletePromisePtr) {

  // Adjust all of the synchronizing information under the _renderMutex to avoid any real time issues with the Renderer::Render() method running in a thread.
  std::lock_guard<std::mutex> renderGuard(_renderMutex);

  // Under the renderGuard, transfer the renterComplete promise to be set when the render is complete, and finally set the _procedeWithRender condition variable to 
  // signal to the Renderer::Render() running in a thread that it needs to start the render process.
  _renderCompletePromisePtr = renderCompletePromisePtr;
  _newRenderReady = true;
  _procedeWithRender.notify_one();

}

// This method allows the caller to initiate a request to the Renderer::Render() method running in a thread to shut down and exit the thread.  
void Renderer::RegisterRenderTerminateRequest() {
  // Set up the terminate information under the _renderMutex to avoid any real time interactions with Renderer::Render() running in a thread.
  std::lock_guard<std::mutex> renderGuard(_renderMutex);

  // Using the _procedeWithRender condition variable, communicate to the render thread that it needs to shut down.
  _terminateRenderThread = true;
  _procedeWithRender.notify_one();
}

void Renderer::SetFoodCoordinates(int x, int y) {
  _food.x = x;
  _food.y = y;
}

int Renderer::GetFoodXCoordinate() {
  return(_food.x);
}

int Renderer::GetFoodYCoordinate() {
  return(_food.y);
}

// This method is run in a thread.  To initiate a render request, the snake head and body and food
void Renderer::Render(std::shared_ptr<Snake> const snake) {

  bool newRender;
  std::unique_lock<std::mutex> renderLock(_renderMutex);

  while(true) {
    // Gain the lock of the render mutex to check the _procededWithRender condition variable.
    if (!renderLock.owns_lock()) {
      renderLock.lock();
    }

    // Wait for the _procededWithRender condition variable for a request from the user.  There are only 2 conditions where it is set:  1)_the user has a new
    // render to perform or 2) the user is requesting to exit and thus terminate the thread.
    _procedeWithRender.wait(renderLock, [this]{return ((_newRenderReady == true) || (_terminateRenderThread == true));});
    _newRenderReady = false;
    renderLock.unlock();

    if (_terminateRenderThread) {
      break;
    }
   
    SDL_Rect block;
    block.w = screen_width / grid_width;
    block.h = screen_height / grid_height;

    // Clear screen
    SDL_SetRenderDrawColor(sdl_renderer, 0x1E, 0x1E, 0x1E, 0xFF);
    SDL_RenderClear(sdl_renderer);

    // Render food
    SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xCC, 0x00, 0xFF);
    block.x = _food.x * block.w;
    block.y = _food.y * block.h;
    SDL_RenderFillRect(sdl_renderer, &block);

    // Render snake's body
    SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    for (SDL_Point const &point : snake->body) {
      block.x = point.x * block.w;
      block.y = point.y * block.h;
      SDL_RenderFillRect(sdl_renderer, &block);
    }

    // Render snake's head
    block.x = static_cast<int>(snake->GetSnakeHeadX()) * block.w;
    block.y = static_cast<int>(snake->GetSnakeHeadY()) * block.h;
    if (snake->alive) {
      SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x7A, 0xCC, 0xFF);
    } else {
      SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0x00, 0x00, 0xFF);
    }
    SDL_RenderFillRect(sdl_renderer, &block);

    // Update Screen
    SDL_RenderPresent(sdl_renderer);

    // Signal to the game that the rendering is complete.
    _renderCompletePromisePtr->set_value();

  }
}

// This displays the score, frames per second, andhigh score in the window title bar.
void Renderer::UpdateWindowTitle(int score, int fps, int highScore) {
  std::string title{"Snake Score: " + std::to_string(score) + " FPS: " + std::to_string(fps) + "  High Score: " + std::to_string(highScore)};
  SDL_SetWindowTitle(sdl_window, title.c_str());
}

// After the snake has died, display the 2 choices in the window title to the user - "y" to start another game, "n" to end.
void Renderer::DisplayPromptForNewGame() {
  std::string title{"****New Game? Press Y for yes, N for no****"};
  SDL_SetWindowTitle(sdl_window, title.c_str());
}


