
#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stdlib.h>
#include <string>
#include <functional>

#include "glm/glm.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/matrix_transform.hpp"
using namespace glm;

namespace mu
{

const vec3
    X = vec3(1, 0, 0),
    Y = vec3(0, 1, 0),
    Z = vec3(0, 0, 1),
    ZERO_3 = vec3(0);

const float

    PI = pi<float>(),

    DEGREES_TO_RAD = PI / 180.0f,
    RAD_TO_DEGREES = 180.0f / PI;


inline float random()
{
    return (float) std::rand() / RAND_MAX;
}

inline float random(float max)
{
    return random() * max;
}


inline float random(float min, float max)
{
    return random(max - min) + min;
}

inline int randomInt(int max)
{
    if (max == 0) return 0;
    return std::rand() % max;
}

inline int randomInt(int min, int max)
{
    return randomInt(max - min) + min;
}

inline vec3 calculateNormal(const vec3 &p0, const vec3 &p1, const vec3 &p2)
{
    return normalize(cross(p1 - p0, p2 - p0));
}

// following 3 functions are 99% copied from: https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/

// Given three collinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
inline bool onSegment(const vec2 &p, const vec2 &q, const vec2 &r)
{
    return q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
           q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y);

}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are collinear
// 1 --> Clockwise
// 2 --> Counterclockwise
inline int orientation(const vec2 &p, const vec2 &q, const vec2 &r)
{
    // See https://www.geeksforgeeks.org/orientation-3-ordered-points/
    // for details of below formula.
    int val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);

    if (val == 0) return 0;  // collinear

    return (val > 0)? 1: 2; // clock or counterclockwise
}

inline bool lineSegmentsIntersect(const vec2 &p1, const vec2 &q1, const vec2 &p2, const vec2 &q2)
{
    // Find the four orientations needed for general and
    // special cases
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    // General case
    if (o1 != o2 && o3 != o4)
        return true;

    // Special Cases
    // p1, q1 and p2 are collinear and p2 lies on segment p1q
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;

    // p1, q1 and q2 are collinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;

    // p2, q2 and p1 are collinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;

     // p2, q2 and q1 are collinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;

    return false; // Doesn't fall in any of the above cases
}

inline float sign(const vec2 &p0, const vec2 &p1, const vec2 &p2)
{
    return (p0.x - p2.x) * (p1.y - p2.y) - (p1.x - p2.x) * (p0.y - p2.y);
}

inline bool pointInTriangle(const vec2 &point, const vec2 &p0, const vec2 &p1, const vec2 &p2)
{
    float d1 = sign(point, p0, p1),
        d2 = sign(point, p1, p2),
        d3 = sign(point, p2, p0);

    bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

/**
 * Executes 'callback' size^2 times in a spiral loop.
 * 'callback' must return true to continue, or false to beak the loop.
 * 
 * Example:
 * if size is 3 then 'callback' will be called 9 times with these positions:
 * (0, 0) (1, 0) (1, 1) (0, 1) (-1, 1) (-1, 0) (-1, -1) (0, -1) (1, -1)
 */
inline void spiral(int size, const std::function<bool(ivec2 pos)> &callback)
{
    ivec2 pos(0), dir(1, 0);
    int steps = 1, stepsToDo = 1, changeSteps = 2;
    for (int i = 0; i < size * size; i++)
    {
        if (!callback(pos)) break;

        pos += dir;
        if (--stepsToDo == 0)
        {
            dir = ivec2(-dir.y, dir.x);
            if (--changeSteps == 0)
            {
                changeSteps = 2;
                steps++;
            }
            stepsToDo = steps;
        }
    }
}

inline glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest) {
	start = glm::normalize(start);
	dest = glm::normalize(dest);

	float cosTheta = glm::dot(start, dest);
	glm::vec3 rotationAxis;

	if (cosTheta < -1 + 0.001f){
		// special case when vectors in opposite directions:
		// there is no "ideal" rotation axis
		// So guess one; any will do as long as it's perpendicular to start
		rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
		if (glm::length2(rotationAxis) < 0.01 ) // bad luck, they were parallel, try again!
			rotationAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);

		rotationAxis = normalize(rotationAxis);
		return glm::angleAxis(glm::radians(180.0f), rotationAxis);
	}

	rotationAxis = glm::cross(start, dest);

	float s = sqrt( (1+cosTheta)*2 );
	float invs = 1 / s;

	return glm::quat(
		s * 0.5f, 
		rotationAxis.x * invs,
		rotationAxis.y * invs,
		rotationAxis.z * invs
	);
}

} // namespace mu

#endif
