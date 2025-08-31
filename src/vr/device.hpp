#pragma once

#include <cstdio>
#include <iostream>
#include <array>
#include <string>

#include <openvr.h>
#include <cmgMath/cmg_math.h>

/// @brief State of a VR device/tracker
class VrDevice
{
public:
	std::string name;
	std::string type;
	uint32_t index = 0;
	Vector3f position = Vector3f::ZERO;
	Vector3f velocity = Vector3f::ZERO;
	Matrix3f orientation = Matrix3f::IDENTITY;
	bool connected = false;
	bool poseValid = false;
};
