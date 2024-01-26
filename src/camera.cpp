#include "camera.h"

Camera::Camera() {

}

glm::mat4 Camera::updateViewMatrix() {

	float cX = sin(rotation.x);
	float sX = cos(rotation.x);
	float cY = cos(rotation.y);
	float sY = sin(rotation.y);

	glm::vec3 position = glm::vec3(cX * cY, sX * cY, sY) * std::exp(-zoom);
	return glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0, 0, 1));
}
