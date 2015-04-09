#include "Game.h"
#include <GameObject.h>
#include <SDL.h>
#include <math.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#include <InputManager.h>
#include <Graphics/GraphicsOpenGL.h>

#include "Cube.h"
#include <Cameras/Camera.h>
#include <Cameras/PerspectiveCamera.h>
#include <Cameras/OrthographicCamera.h>

#include <Parallax/ParallaxLayer.h>
#include <Parallax/ParallaxSystem.h>
#include "Player.h"
#include "Enemy.h"

// Initializing our static member pointer.
GameEngine* GameEngine::_instance = nullptr;

GameEngine* GameEngine::CreateInstance()
{
  if (_instance == nullptr)
  {
    _instance = new Game();
  }
  return _instance;
}

Game::Game() : GameEngine()
{

}

Game::~Game()
{
  // Clean up our characters.
  delete _player;
  _player = nullptr;

  delete _enemy;
  _enemy = nullptr;

  // Clean up our pointers.
  delete _parallaxCamera;
  _parallaxCamera = nullptr;

  ParallaxLayer *layerToDelete = nullptr;
  while (_backgroundParallaxSystem->LayerCount() > 0)
  {
    // Delete all of the layers inside of our parallax system.
    layerToDelete = _backgroundParallaxSystem->PopLayer();

    delete layerToDelete;
    layerToDelete = nullptr;
  }

  delete _backgroundParallaxSystem;
  _backgroundParallaxSystem;
}

SDL_Renderer *_renderer;
SDL_Texture *_texture;

void Game::InitializeImpl()
{
  SDL_SetWindowTitle(_window, "Game");

  float nearPlane = 0.01f;
  float farPlane = 100.0f;
  Vector4 position(3.5f, 2.5f, 2.5f, 0.0f);
  Vector4 lookAt = Vector4::Normalize(Vector4::Difference(Vector4(0.0f, 0.0f, 0.0f, 0.0f), position));
  Vector4 up(0.0f, 1.0f, 0.0f, 0.0f);

  //_camera = new PerspectiveCamera(100.0f, 1.0f, nearPlane, farPlane, position, lookAt, up);
  _gameCamera = new OrthographicCamera(-10.0f, 10.0f, 10.0f, -10.0f, nearPlane, farPlane, position, lookAt, up);

  position = Vector4(0.0f, 0.0f, 2.5f, 0.0f);
  lookAt = Vector4::Normalize(Vector4::Difference(Vector4(0.0f, 0.0f, 0.0f, 0.0f), position));
  _parallaxCamera = new OrthographicCamera(-10.0f, 10.0f, 10.0f, -10.0f, nearPlane, farPlane, position, lookAt, up);

  /*_objects.push_back(new Cube(Vector3(0.0f, 2.0f, 0.0f)));
  _objects.push_back(new Cube(Vector3(1.0f, 1.0f, 0.0f)));
  _objects.push_back(new Cube(Vector3(1.0f, 0.0f, 1.0f)));
  _objects.push_back(new Cube(Vector3(0.0f, 1.0f, 1.0f)));*/

  _player = new Player();
  _enemy = new Enemy();
  _backgroundParallaxSystem = new ParallaxSystem();

  _objects.push_back(_player);
  _objects.push_back(_enemy);

  for (auto itr = _objects.begin(); itr != _objects.end(); itr++)
  {
    (*itr)->Initialize(_graphicsObject);
  }

  // Position our characters.
  _player->GetTransform().position.x = -5.0f;
  _enemy->GetTransform().position.x = 5.0f;

  _enemy->SetApproachProximity(5.0f);
  _enemy->SetApproachSpeed(5.0f);

  ParallaxLayer *_parallax = new ParallaxLayer("res/UV.jpg", Vector2(0.25f, 0.25f));
  _parallax->GetTransform().position.y -= 5.1f;
  _backgroundParallaxSystem->PushLayer(_parallax);

  ParallaxLayer *face = new ParallaxLayer("res/face.png", Vector2(0.15f, 0.15f));
  face->GetTransform().position.z += 1.0f;
  _backgroundParallaxSystem->PushLayer(face);

  _backgroundParallaxSystem->Initialize(_graphicsObject);

  InputManager::GetInstance()->AssignEvent(SDL_MOUSEMOTION, this, (InputMemberEvent)(&Game::HandleMouseMotion));
}

void Game::UpdateImpl(float dt)
{
  //SDL_Event evt;
  //SDL_PollEvent(&evt);
  InputManager::GetInstance()->Update(dt);

  if (InputManager::GetInstance()->GetKeyState(SDLK_LEFT, SDL_KEYDOWN) == true)
  {
    _player->GetTransform().position.x -= 1.0f * dt;
  }
  else if (InputManager::GetInstance()->GetKeyState(SDLK_RIGHT, SDL_KEYDOWN) == true)
  {
    _player->GetTransform().position.x += 1.0f * dt;
  }

  _enemy->Approach(_player->GetTransform().position, dt);

  for (auto itr = _objects.begin(); itr != _objects.end(); itr++)
  {
    (*itr)->Update(dt);
  }

  _backgroundParallaxSystem->Update(_mouseMotion, dt);
  _mouseMotion = Vector2::Zero();
}

void Game::DrawImpl(Graphics *graphics, float dt)
{
  std::vector<GameObject *> renderOrder = _objects;
  //CalculateDrawOrder(renderOrder);

  // Draw parallax backgrounds
  glPushMatrix();
  {
    CalculateCameraViewpoint(_parallaxCamera);

    _backgroundParallaxSystem->Draw(graphics, _gameCamera->GetProjectionMatrix(), dt);
  }
  glPopMatrix();

  // Draw scenery on top.
  glPushMatrix();
  {
    glClear(GL_DEPTH_BUFFER_BIT);
    CalculateCameraViewpoint(_gameCamera);

    for (auto itr = renderOrder.begin(); itr != renderOrder.end(); itr++)
    {
      (*itr)->Draw(graphics, _gameCamera->GetProjectionMatrix(), dt);
    }
  }
  glPopMatrix();
}

void Game::HandleMouseMotion(SDL_Event e)
{
  // Get the size of the screen.
  int w, h;
  SDL_GetWindowSize(_window, &w, &h);

  Vector2 oldMouse = _mousePos;

  /* Subtract the middle of the screen from current cursor position to
   * store the relative distance. */
  _mousePos.x = e.motion.x - (w / 2.0f);
  _mousePos.y = e.motion.y - (h / 2.0f);

  // Calculate the difference in the distance between last frame, and current frame.
  _mouseMotion.x = oldMouse.x - _mousePos.x;
  _mouseMotion.y = _mousePos.y - oldMouse.y;
}

void Game::CalculateDrawOrder(std::vector<GameObject *>& drawOrder)
{
  // SUPER HACK GARBAGE ALGO.
  drawOrder.clear();

  auto objectsCopy = _objects;
  auto farthestEntry = objectsCopy.begin();
  while (objectsCopy.size() > 0)
  {
    bool entryFound = true;
    for (auto itr = objectsCopy.begin(); itr != objectsCopy.end(); itr++)
    {
      if (farthestEntry != itr)
      {
        if ((*itr)->GetTransform().position.y < (*farthestEntry)->GetTransform().position.y)
        {
          entryFound = false;
          farthestEntry = itr;
          break;
        }
      }
    }

    if (entryFound)
    {
      GameObject *farthest = *farthestEntry;

      drawOrder.push_back(farthest);
      objectsCopy.erase(farthestEntry);
      farthestEntry = objectsCopy.begin();
    }
  }
}

void Game::CalculateCameraViewpoint(Camera *camera)
{
  camera->Apply();

  Vector4 xAxis(1.0f, 0.0f, 0.0f, 0.0f);
  Vector4 yAxis(0.0f, 1.0f, 0.0f, 0.0f);
  Vector4 zAxis(0.0f, 0.0f, 1.0f, 0.0f);

  Vector3 cameraVector(camera->GetLookAtVector().x, camera->GetLookAtVector().y, camera->GetLookAtVector().z);
  Vector3 lookAtVector(0.0f, 0.0f, -1.0f);

  Vector3 cross = Vector3::Normalize(Vector3::Cross(cameraVector, lookAtVector));
  float dot = MathUtils::ToDegrees(Vector3::Dot(lookAtVector, cameraVector));

  glRotatef(cross.x * dot, 1.0f, 0.0f, 0.0f);
  glRotatef(cross.y * dot, 0.0f, 1.0f, 0.0f);
  glRotatef(cross.z * dot, 0.0f, 0.0f, 1.0f);

  glTranslatef(-camera->GetPosition().x, -camera->GetPosition().y, -camera->GetPosition().z);
}