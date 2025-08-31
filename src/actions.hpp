#pragma once

#include <cstdio>
#include <iostream>
#include <string>
#include <iostream>
#include <array>
#include <map>

#include <openvr.h>
#include <cmgCore/cmg_core.h>
#include <cmgMath/cmg_math.h>

/// @brief Base class for a VR action bind
class Action
{
public:
	explicit Action(const std::string &identifier);

	virtual void Update() {}
	virtual std::ostream &DebugString(std::ostream &stream) const
	{
		stream << identifier << ":";
		return stream;
	}

	std::string identifier;
	vr::VRActionHandle_t handle = vr::k_ulInvalidActionHandle;
};

/// @brief A joystick action, with multiple analog axes
class JoystickAction : public Action
{
public:
	explicit JoystickAction(const std::string &identifier) : Action(identifier) {}

	virtual void Update() override;
	virtual std::ostream &DebugString(std::ostream &stream) const override;

	Vector2f position = Vector2f::ZERO;
	Vector2f delta = Vector2f::ZERO;
};

/// @brief A binary button action, that can be pressed/released
class ButtonAction : public Action
{
public:
	explicit ButtonAction(const std::string &identifier) : Action(identifier) {}

	virtual void Update() override;
	virtual std::ostream &DebugString(std::ostream &stream) const override;

	bool down = false;
	bool pressed = false;
	bool released = false;
};

class HapticAction : public Action
{
public:
	explicit HapticAction(const std::string &identifier) : Action(identifier) {}
};

class ActionSet
{
public:
	vr::VRActionSetHandle_t handle = vr::k_ulInvalidActionSetHandle;

	ActionSet(const std::string &identifier);

	Error Load(const Path &path);

	void AddAction(std::shared_ptr<Action> action);
	std::shared_ptr<Action> GetAction(const std::string &name);

	template <class T>
	std::shared_ptr<T> GetActionOfType(const std::string &name)
	{
		auto action = GetAction(name);
		if (action == nullptr)
			return nullptr;
		return std::dynamic_pointer_cast<T>(action);
	}

	void Update();
	inline std::map<std::string, std::shared_ptr<Action>> &GetActions() { return m_actions; }

private:
	std::map<std::string, std::shared_ptr<Action>> m_actions;
};
