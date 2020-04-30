
#include "planet_generator.h"

#include <iostream>
#include <glm/gtx/rotate_vector.hpp>

#include "utils/math_utils.h"

PlanetGenerator::PlanetGenerator(Planet *planet, IslandContextProvider islContextProvider, int nrOfIslands)
    : plt(planet),
      islContextProvider(islContextProvider), 
      nrOfIslands(nrOfIslands), minNrOfIslands(nrOfIslands)
{
}

void PlanetGenerator::generate()
{
    std::cout << "Starting generation of " << plt->name << std::endl;

    // int failedTries = 0;

    // while (!tryToGenerate())
    // {
    //     std::cout << "Planet generation failed\n";
    //     if (++failedTries > 3)
    //     {
    //         std::cout << "nr of Islands is too high!!!!\n\n\n";
    //         minNrOfIslands--;
    //         failedTries = 0;
    //     }
    //     plt->destroyLandMasses();
    // }

    VertAttributes attrs;
    attrs.add_(VertAttributes::POSITION)
        .add_(VertAttributes::NORMAL)
        .add_(VertAttributes::TANGENT)
        .add_(VertAttributes::TEX_COORDS);

    plt->mesh= plt->sphere.generate(plt->name + "_mesh", plt->config.segments, plt->config.rings, attrs);
    plt->upload();

    // std::cout << "Generation of " << plt->name << " done. Placed " << plt->land.size() << "/" << nrOfIslands << " islands.\n";
    std::cout << "Generation of " << plt->name << " done.\n";
}

bool PlanetGenerator::tryToGenerate()
{
    int maxTries = 50;
    int failedTries = 0;

    // while (plt->land.size() < nrOfIslands && failedTries < maxTries)
    // {
    //     auto context = islContextProvider(plt->land.size());
    //     LandMass *isl = context.islandGenerator.generateEssentials();

    //     if (isl->percentageUnderwater() < .87 && placeOnPlanet(isl, context.minLatitude, context.maxLatitude))
    //     {
    //         failedTries = 0;
    //         context.islandGenerator.finishGeneration();
    //     }
    //     else 
    //     {
    //         failedTries++;
    //         delete isl; // so sad
    //     }
    // }
    // return plt->land.size() >= minNrOfIslands;
}

bool PlanetGenerator::placeOnPlanet(LandMass *isl, float minLat, float maxLat)
{
    int failedTries = 0;
    while (true)
    {
        float lon = mu::random(360), lat = mu::random(minLat, maxLat);

        if (tryToPlaceOnPlanet(isl, lon, lat)) return true;
        if (++failedTries >= 500) return false;
    }
}

bool PlanetGenerator::tryToPlaceOnPlanet(LandMass *isl, float lon, float lat)
{
    // isl->longitude = lon;
    // isl->latitude = lat;
    // isl->placeOnPlanet();

    // if (overflowsLongitude(isl)) return false;

    // for (LandMass *isl1 : plt->land)
    //     if (overlaps(isl, isl1)) return false;

    // isl->transformVertices();
    // plt->land.push_back(isl);
    // return true;
}

bool PlanetGenerator::overflowsLongitude(LandMass *isl)
{
    auto &outlines = isl->outlinesLongLat;
    for (auto &outline : outlines)
    {
        float prevLon = outline.points[0].x;
        for (int i = 1; i < outline.points.size(); i++)
        {
            float 
                lon = outline.points[i].x,
                dist = abs(prevLon - lon),
                minLon = min(lon, prevLon), maxLon = max(lon, prevLon),
                distWithOverflow = min(dist, minLon + 360 - maxLon);

            if (distWithOverflow != dist)
                return true;

            prevLon = lon;
        }
    }
    return false;
}

bool PlanetGenerator::overlaps(LandMass *isl0, LandMass *isl1)
{
    for (auto &outline0 : isl0->outlinesLongLat)
    {
        for (auto &outline1 : isl1->outlinesLongLat)
        {
            for (auto &p : outline1.points)
                if (outline0.contains(p.x, p.y)) return true;

            for (auto &p : outline0.points)
                if (outline1.contains(p.x, p.y)) return true;
        }
    }
    return false;
}
