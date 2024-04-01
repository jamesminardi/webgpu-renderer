#include "camera.h"

Camera::Camera() {
}

glm::mat4 Camera::updateViewMatrix() {

	float sinPhi = sin(rotation.x);
	float cosPhi = cos(rotation.x);
	float cosTheta = cos(rotation.y);
	float sinTheta = sin(rotation.y);

	position = glm::vec3(sinTheta * sinPhi,  cosTheta, sinTheta * cosPhi) * std::exp(-zoom);
	position += center;
	return glm::lookAt(position, center, glm::vec3(0, 1, 0));
}
