#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <future>
#include <mutex>
#include <condition_variable>
#include "SDL.h"
#include "snake.h"

class Renderer {
 public:
  Renderer(const std::size_t screen_width, const std::size_t screen_height,
           const std::size_t grid_width, const std::size_t grid_height);
  ~Renderer();
 

  void Render(std::shared_ptr<Snake> const snake );
  void UpdateWindowTitle(int score, int fps, int highScrore);
  void RegisterNewRenderRequest(std::promise<void> *renderCompletePromise);
  void RegisterRenderTerminateRequest();
  void SetFoodCoordinates(int x, int y);
  int GetFoodXCoordinate();
  int GetFoodYCoordinate();
  void DisplayPromptForNewGame();

  

 private:
  SDL_Window *sdl_window;
  SDL_Renderer *sdl_renderer;
  SDL_Point _food;

  const std::size_t screen_width;
  const std::size_t screen_height;
  const std::size_t grid_width;
  const std::size_t grid_height;
  std::mutex _renderMutex;
  std::condition_variable _procedeWithRender;
  bool _newRenderReady;
  bool _terminateRenderThread;
  std::promise<void> *_renderCompletePromisePtr;
  

};
#endif