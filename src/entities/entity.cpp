#include "entity.hpp"

// Standard Headers
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "scene.hpp"

Entity::Entity(EntityType type, glm::vec3 model_direction): 
	Renderable(model_direction),
	_direction(1.f, 0.f, 0.f),
	_speed(0.f) {
    _type = type;
}

Entity::Entity(EntityType type): Entity(type, glm::vec3(1.f, 0.f, 0.f)) {}

void Entity::update(float dt) {
    // Update the current rotation based on direction the entity is facing
    // float current_angle = atan2( _direction.x, _direction.z);
    // _rotation = glm::quat(cos(current_angle/2.f), 0.f, sin(current_angle/2.f), 0.f);  
	if (!dragging) {
		if (_origin.y > 0) {
			// glm::vec3 velocity = glm::vec3(get_velocity());
			// velocity.y -= (dt * 9.8f);

			// _speed = glm::length(velocity);

			// if (_speed != 0.0) {
			// 	_direction = velocity / _speed;
			// }
			
			_origin += dt * get_velocity();
		} else {
			_origin.y = 0;
			_speed = 0.f;
		}
	} else {
		_speed = 0.f;
	}
}

void Entity::reset() {}

std::vector<orientation_state> Entity::get_current_path() {
    return {{_origin + _direction, _direction}, {_origin, _direction}};
}

float Entity::get_radius() {
    return _radius;
}

void Entity::drag(const glm::vec3 & origin, const glm::vec3 & direction) {
	dragging = true;
	this->set_position(origin + glm::distance(origin, _origin) * direction);
}

void Entity::stop_dragging() {
	dragging = false;
}

// Does entity collide with this entity if it were to travel from a to b

bool Entity::check_collision(const orientation_state & a, const orientation_state & b, Entity * entity) {
	
	// Check 10 times every 1 unit
	float steps_per_unit = 10.f;
	float step_size = 1.0f / (steps_per_unit * glm::distance(a.position, b.position));
	
	orientation_state test;
	for (float t = 0; t < 1.0f; t += step_size) {
		test.position = glm::mix(a.position, b.position, t);
		test.rotation = glm::mix(a.rotation, b.rotation, t);

		if (check_collision(OBB(entity->get_model_bounding_box(), test))) {
			return true;
		}
	}

	return false;
}

// Check if I would collide with bounding box with given orientation bound
bool Entity::check_collision(const OBB & bbox) {
	// Lets move the bbox towards me
	return OBB(get_model_bounding_box(), get_current_state()).test_obb_obb_collision(bbox);
}

bool Entity::test_ray(glm::vec3 ray_origin, glm::vec3 ray_direction, float& intersection_distance) {
    glm::mat4 model_matrix = get_last_model();
	
	bounding_box bbox;
	bbox.max = model_matrix * glm::vec4(_model_bbox.max, 1);
	bbox.min = 	model_matrix * glm::vec4(_model_bbox.min, 1);

    // Should be max min, so if min is bigger than max we've got nothing
    if (bbox.max.x < bbox.min.x && bbox.max.y < bbox.min.y && bbox.max.z < bbox.min.z) return false;

    // Intersection method from Real-Time Rendering and Essential Mathematics for Games
	
	float tMin = 0.0f;
	float tMax = 100000.0f;

	// glm::vec3 OBBposition_worldspace(model_matrix[3].x, model_matrix[3].y, model_matrix[3].z);

	glm::vec3 delta = - ray_origin;

	// Test intersection with the 2 planes perpendicular to the OBB's X axis
	{
		glm::vec3 xaxis(1.f, 0.f, 0.f);
		float e = glm::dot(xaxis, delta);
		float f = glm::dot(ray_direction, xaxis);

		if ( fabs(f) > 0.001f ){ // Standard case

			float t1 = (e+bbox.min.x)/f; // Intersection with the "left" plane
			float t2 = (e+bbox.max.x)/f; // Intersection with the "right" plane
			// t1 and t2 now contain distances betwen ray origin and ray-plane intersections

			// We want t1 to represent the nearest intersection, 
			// so if it's not the case, invert t1 and t2
			if (t1>t2){
				float w=t1;t1=t2;t2=w; // swap t1 and t2
			}

			// tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
			if ( t2 < tMax )
				tMax = t2;
			// tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
			if ( t1 > tMin )
				tMin = t1;

			// And here's the trick :
			// If "far" is closer than "near", then there is NO intersection.
			// See the images in the tutorials for the visual explanation.
			if (tMax < tMin )
				return false;

		}else{ // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
			if(-e+bbox.min.x > 0.0f || -e+bbox.max.x < 0.0f)
				return false;
		}
	}


	// Test intersection with the 2 planes perpendicular to the OBB's Y axis
	// Exactly the same thing than above.
	{
		glm::vec3 yaxis(0.f, 1.f, 0.f);
		float e = glm::dot(yaxis, delta);
		float f = glm::dot(ray_direction, yaxis);

		if ( fabs(f) > 0.001f ){

			float t1 = (e+bbox.min.y)/f;
			float t2 = (e+bbox.max.y)/f;

			if (t1>t2){float w=t1;t1=t2;t2=w;}

			if ( t2 < tMax )
				tMax = t2;
			if ( t1 > tMin )
				tMin = t1;
			if (tMin > tMax)
				return false;

		}else{
			if(-e+bbox.min.y > 0.0f || -e+bbox.max.y < 0.0f)
				return false;
		}
	}


	// Test intersection with the 2 planes perpendicular to the OBB's Z axis
	// Exactly the same thing than above.
	{
		glm::vec3 zaxis(0.f, 0.f, 1.f);
		float e = glm::dot(zaxis, delta);
		float f = glm::dot(ray_direction, zaxis);

		if ( fabs(f) > 0.001f ){

			float t1 = (e+bbox.min.z)/f;
			float t2 = (e+bbox.max.z)/f;

			if (t1>t2){float w=t1;t1=t2;t2=w;}

			if ( t2 < tMax )
				tMax = t2;
			if ( t1 > tMin )
				tMin = t1;
			if (tMin > tMax)
				return false;

		}else{
			if(-e+bbox.min.z > 0.0f || -e+bbox.max.z < 0.0f)
				return false;
		}
	}

	intersection_distance = tMin;
	return true;
}