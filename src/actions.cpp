#include "actions.hpp"

#include <cstdio>
#include <iostream>
#include "rapidjson/filewritestream.h"
#include "rapidjson/document.h"

ActionSet::ActionSet(const std::string &identifier)
{
	vr::VRInput()->GetActionSetHandle(identifier.c_str(), &handle);
}

std::shared_ptr<Action> ActionSet::GetAction(const std::string &name)
{
	auto it = m_actions.find(name);
	if (it != m_actions.end())
		return it->second;
	return nullptr;
}

void ActionSet::AddAction(std::shared_ptr<Action> action)
{
	m_actions[action->identifier] = action;
}

void ActionSet::Update()
{
	vr::VRActiveActionSet_t actionSet = {0};
	actionSet.ulActionSet = handle;
	vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);

	for (auto it : m_actions)
	{
		it.second->Update();
	}
}

Action::Action(const std::string &identifier) : identifier(identifier)
{
	CMG_LOG_DEBUG() << "Loading action: " << identifier;
	vr::EVRInputError error = vr::VRInput()->GetActionHandle(identifier.c_str(), &handle);
	if (error != vr::VRInputError_None)
	{
		CMG_LOG_ERROR() << "  ERROR " << error;
		throw std::exception("INPUT ERROR");
	}
}

void ButtonAction::Update()
{
	vr::InputDigitalActionData_t actionData;
	vr::VRInput()->GetDigitalActionData(handle, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle);
	bool downPrev = down;
	down = actionData.bActive && actionData.bState;
	pressed = down && !downPrev;
	released = !down && downPrev;
}

std::ostream &ButtonAction::DebugString(std::ostream &stream) const
{
	stream << identifier << ": ";
	if (pressed)
		stream << "PRESSED";
	if (released)
		stream << "RELEASED";
	else
		stream << (down ? "DOWN" : "UP");
	return stream;
}

void JoystickAction::Update()
{
	vr::InputAnalogActionData_t analogData;
	auto error = vr::VRInput()->GetAnalogActionData(handle,
													&analogData, sizeof(analogData), vr::k_ulInvalidInputValueHandle);

	Vector2f positionPrev = position;
	if (error == vr::VRInputError_None && analogData.bActive)
	{
		position.x = analogData.x;
		position.y = analogData.y;
	}
	else
	{
		position = Vector2f::ZERO;
	}

	delta = position - positionPrev;
}

std::ostream &JoystickAction::DebugString(std::ostream &stream) const
{
	stream << identifier << ": x=" << position.x << ", y=" << position.y;
	return stream;
}

Error ActionSet::Load(const Path &path)
{
	CMG_LOG_INFO() << "Loading action config: " << path;

	// Open the json file
	String json;
	rapidjson::Document document;
	Error error = File::OpenAndGetContents(path, json);
	if (error.Failed())
		return error.Uncheck();
	document.Parse(json.c_str());
	if (document.HasParseError())
		return CMG_ERROR(Error::kFileCorrupt);

	// Process all mappings
	rapidjson::Value &actionList = document["actions"];
	for (auto it = actionList.Begin(); it != actionList.End(); it++)
	{
		auto &actionData = *it;
		std::string name = actionData["name"].GetString();
		std::string type = actionData["type"].GetString();

		if (type == "boolean")
		{
			AddAction(std::make_shared<ButtonAction>(name));
		}
		else if (type == "vector2")
		{
			AddAction(std::make_shared<JoystickAction>(name));
		}
		else if (type == "pose")
		{
		}
		else if (type == "vibration")
		{
		}
		else
		{
			CMG_LOG_ERROR() << "Unsupported action type: \"" << type << "\"";
		}
	}

	return CMG_ERROR_SUCCESS;
}