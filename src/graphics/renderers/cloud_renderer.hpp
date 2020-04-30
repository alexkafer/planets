#pragma once

// Local Headers
#include "common/planet.hpp"
#include "graphics/camera.hpp"
#include "graphics/texture.hpp"

#include "utils/resource_manager.hpp"

struct Cloud
{
    float lon, lat, speed, timeSinceSpawn, timeToDespawn;
    int spawnPoints;
};

class CloudRenderer
{
  public:
    CloudRenderer(Planet * planet);

    void render(double dt);

  private:
    SharedMesh quad;
    Planet * planet;
    SharedTexture noiseTex;
    std::vector<Cloud> clouds;

};
