#pragma once

#include <GameObject.h>

class Cube;

class Enemy : public GameObject
{
public:
  ~Enemy();

  void Initialize(Graphics *graphics);

  void Update(float dt);
  void Draw(Graphics *graphics, Matrix4x4 relativeTo, float dt);

  bool Approach(Vector3 destination, float dt);

  void SetApproachProximity(float proximity);
  void SetApproachSpeed(float speed);

protected:
  Cube *_enemyCube;
  float _approachProximity; // how close before we stop.
  float _approachSpeed; // units / second.
};