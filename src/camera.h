#pragma once

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "input.h"


struct DragState {
	// Whether a drag action is ongoing (i.e., we are between mouse press and mouse release)
	bool active = false;
	// The position of the mouse at the beginning of the drag action
	glm::vec2 startMouse;
	// The camera state at the beginning of the drag action
	glm::vec2 startRotation;

	// Constant settings
	float sensitivity = 0.01f;
	float scrollSensitivity = 0.1f;
};

class Camera {
public:
	Camera();

	glm::mat4 updateViewMatrix();

//	glm::vec3 getPosition() const;
//	glm::vec3 getDirection() const;
//	glm::vec3 getUp() const;
//	glm::vec3 getRight() const;
//
//	float getAspectRatio() const;
//	float getFov() const;
//	float getNear() const;
//	float getFar() const;


	// x is the rotation of the camera around the global vertical axis, affected by mouse.x movement
	// y is the rotation of the camera around its local horizontal axis, affected by mouse.y movement
	// in radians
	glm::vec2 rotation = glm::vec2(0.0f, glm::radians(45.0f));

	DragState dragState;

	// Zoom is the position of the camera along its local forward axis, affected by scroll
	float zoom = 0.0f;

private:


};
