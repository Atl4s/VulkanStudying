#pragma once
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
	Camera();
	~Camera();

	glm::mat4 getView() { return m_view; }
	glm::mat4 getProjection() { return m_projection; }

	glm::vec3 getPosition() { return m_position; }
	glm::vec3 getDirection() { return m_direction; }

private:
	float m_fov;
	float m_near, m_far;

	glm::mat4 m_view;
	glm::mat4 m_projection;

	glm::vec3 m_position;
	glm::vec3 m_direction;
	glm::vec3 m_rotation;
	glm::vec3 m_up;
	glm::vec3 m_right;
};

