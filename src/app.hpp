#pragma once

#include <cstdio>
#include <iostream>
#include <vector>
#include <map>

#include <openvr.h>
#include <cmgApplication/cmg_application.h>
#include <cmgCore/cmg_core.h>
#include <cmgGraphics/cmg_graphics.h>
#include <cmgMath/cmg_math.h>

#include "vr/actions.hpp"
#include "vr/device.hpp"
#include "mappings/bindings.hpp"
#include "mappings/sphere_aim_controller.hpp"

class Tf2ActionSet : public ActionSet
{
public:
	std::shared_ptr<ButtonAction> jump;
	std::shared_ptr<ButtonAction> duck;
	std::shared_ptr<ButtonAction> primaryAttack;
	std::shared_ptr<ButtonAction> secondaryAttack;
	std::shared_ptr<ButtonAction> specialAttack;
	std::shared_ptr<ButtonAction> reload;
	std::shared_ptr<ButtonAction> calibrate;
	std::shared_ptr<ButtonAction> enableLook;
	std::shared_ptr<ButtonAction> prevWeapon;
	std::shared_ptr<ButtonAction> nextWeapon;
	std::shared_ptr<ButtonAction> taunt;
	std::shared_ptr<JoystickAction> movement;
	std::shared_ptr<JoystickAction> turning;
	std::shared_ptr<HapticAction> hapticLeft;
	std::shared_ptr<HapticAction> hapticRight;

	Tf2ActionSet() : ActionSet("/actions/tf2")
	{
		AddAction(jump = std::make_shared<ButtonAction>("/actions/tf2/in/Jump"));
		AddAction(duck = std::make_shared<ButtonAction>("/actions/tf2/in/Duck"));
		AddAction(primaryAttack = std::make_shared<ButtonAction>("/actions/tf2/in/PrimaryAttack"));
		AddAction(secondaryAttack = std::make_shared<ButtonAction>("/actions/tf2/in/SecondaryAttack"));
		AddAction(specialAttack = std::make_shared<ButtonAction>("/actions/tf2/in/SpecialAttack"));
		AddAction(reload = std::make_shared<ButtonAction>("/actions/tf2/in/Reload"));
		AddAction(calibrate = std::make_shared<ButtonAction>("/actions/tf2/in/Calibrate"));
		AddAction(enableLook = std::make_shared<ButtonAction>("/actions/tf2/in/EnableLook"));
		AddAction(prevWeapon = std::make_shared<ButtonAction>("/actions/tf2/in/PreviousWeapon"));
		AddAction(nextWeapon = std::make_shared<ButtonAction>("/actions/tf2/in/NextWeapon"));
		AddAction(taunt = std::make_shared<ButtonAction>("/actions/tf2/in/Taunt"));
		AddAction(movement = std::make_shared<JoystickAction>("/actions/tf2/in/Movement"));
		AddAction(turning = std::make_shared<JoystickAction>("/actions/tf2/in/Turning"));
		AddAction(hapticLeft = std::make_shared<HapticAction>("/actions/tf2/out/HapticLeft"));
		AddAction(hapticRight = std::make_shared<HapticAction>("/actions/tf2/out/HapticRight"));
	}
};

class App : public Application
{
public:
	App();
	~App();

	void Terminate();

	void OnInitialize() override;
	void OnQuit() override;
	void OnResizeWindow(int width, int height) override;
	void OnUpdate(float timeDelta) override;
	void OnRender() override;

private:
	std::shared_ptr<VrDevice> AddDevice(uint32_t index, const vr::TrackedDevicePose_t &pose);
	void UpdateDevice(std::shared_ptr<VrDevice> device, const vr::TrackedDevicePose_t &pose);

	vr::IVRSystem *m_hmd = nullptr;

	std::shared_ptr<Tf2ActionSet> m_actions;
	std::map<uint32_t, std::shared_ptr<VrDevice>> m_devices;
	std::shared_ptr<VrDevice> m_rightController = nullptr;
	std::shared_ptr<VrDevice> m_leftController = nullptr;

	mappings::BindMapper m_bindMapper;
	std::shared_ptr<mappings::SphereAimController> m_aimController;

	bool m_controlMappingEnabled = true;

	Font::sptr m_font = nullptr;
};
