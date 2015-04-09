#include "Enemy.h"
#include "Cube.h"
#include <math.h>

Enemy::~Enemy()
{
  delete _enemyCube;
  _enemyCube = nullptr;
}

void Enemy::Initialize(Graphics *graphics)
{
  _enemyCube = new Cube();
  _enemyCube->Initialize(graphics);

  _transform.scale = Vector3(0.5f, 0.5f, 0.5f);

  _approachProximity = 0.0f;
  _approachSpeed = 0.0f;
}

void Enemy::Update(float dt)
{
  // Position the enemy cube wherever the enemy is.
  _enemyCube->GetTransform().position = _transform.position;
  _enemyCube->GetTransform().scale = _transform.scale;
}

void Enemy::Draw(Graphics *graphics, Matrix4x4 relativeTo, float dt)
{
  _enemyCube->Draw(graphics, relativeTo, dt);
}

void Enemy::SetApproachProximity(float proximity)
{
  _approachProximity = proximity;
}

void Enemy::SetApproachSpeed(float speed)
{
  _approachSpeed = speed;
}

bool Enemy::Approach(Vector3 destination, float dt)
{
  bool didMove = true;
  bool didApproach = false;

  // Find the distance from our destination.
  Vector3 distanceVector = Vector3::Difference(destination, _transform.position);

  Vector3 directionVector = Vector3::Normalize(distanceVector);
  float distance = Vector3::Magnitude(distanceVector);

  //If we're further out than our approach proximity, let's move closer.
  if (distance > _approachProximity)
  {
    // Calculating how fast we should go based on how far away we are.
    directionVector.x *= (_approachSpeed * dt);
    directionVector.y *= (_approachSpeed * dt);
    directionVector.z *= (_approachSpeed * dt);
  }
  // If we're already too close, we'll need to duck back.
  else if (_approachProximity != 0.0f)
  {
    // Calculating how fast we should go based on how far away we are.
    float percentage = 1.0f - (distance / _approachProximity);

    directionVector.x *= -(_approachSpeed * dt) * percentage;
    directionVector.y *= -(_approachSpeed * dt) * percentage;
    directionVector.z *= -(_approachSpeed * dt) * percentage;
  }

  // Adding our direction to our position.
  _transform.position.x += directionVector.x;
  _transform.position.y += directionVector.y;
  _transform.position.z += directionVector.z;

  if (Vector3::Magnitude(directionVector) > 0.0f)
  {
    didApproach = true;
  }

  return didApproach;
}