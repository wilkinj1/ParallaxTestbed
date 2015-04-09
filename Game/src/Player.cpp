#include "Player.h"
#include "Cube.h"

Player::~Player()
{
  delete _playerCube;
  _playerCube = nullptr;
}

void Player::Initialize(Graphics *graphics)
{
  _playerCube = new Cube();
  _playerCube->Initialize(graphics);

  _transform.scale = Vector3(2.0f, 2.0f, 2.0f);
}

void Player::Update(float dt)
{
  // Position the enemy cube wherever the enemy is.
  _playerCube->GetTransform().position = _transform.position;
  _playerCube->GetTransform().scale = _transform.scale;
}

void Player::Draw(Graphics *graphics, Matrix4x4 relativeTo, float dt)
{
  _playerCube->Draw(graphics, relativeTo, dt);
}