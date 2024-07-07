#pragma once

class Scene {
public:
  virtual ~Scene() = default;
  virtual void render() = 0;
};
