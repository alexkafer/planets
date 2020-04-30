
#ifndef EARTH_GENERATOR_H
#define EARTH_GENERATOR_H

#include "common/planet.hpp"
#include "planet_generator.h"

SharedMesh earthMeshGenerator(Planet *earth);

void generateEarth(Planet *earth);

#endif