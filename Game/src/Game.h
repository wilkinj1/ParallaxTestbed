#pragma once

#include <GameEngine.h>

union SDL_Event;
class Graphics;
class Camera;
class Player;
class Enemy;
class ParallaxSystem;

class Game: public GameEngine
{
  friend class GameEngine;

public:
  ~Game();

protected:
  Game();

  void InitializeImpl();
  void UpdateImpl(float dt);
  void DrawImpl(Graphics *graphics, float dt);

  void Reset();
  void CalculateDrawOrder(std::vector<GameObject *>& drawOrder);
  void CalculateCameraViewpoint(Camera *camera);
  void HandleMouseMotion(SDL_Event e);

  Camera *_gameCamera;
  Camera *_parallaxCamera;

  Player *_player;
  Enemy *_enemy;

  ParallaxSystem *_backgroundParallaxSystem;
  Vector2 _mousePos;
  Vector2 _mouseMotion;
};