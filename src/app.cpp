#include "app.hpp"

#include "mappings/bind_config.hpp"
#include <cstdio>
#include <iostream>
#include <Windows.h>
#include <cmgMath/cmg_math.h>

namespace
{
#ifndef MAX_UNICODE_PATH
#define MAX_UNICODE_PATH 32767
#endif

#ifndef MAX_UNICODE_PATH_IN_UTF8
#define MAX_UNICODE_PATH_IN_UTF8 (MAX_UNICODE_PATH * 4)
#endif

	/** Returns the path (including filename) to the current executable */
	Path GetExecutablePath()
	{
#if defined(_WIN32)
		wchar_t *pwchPath = new wchar_t[MAX_UNICODE_PATH];
		char *pchPath = new char[MAX_UNICODE_PATH_IN_UTF8];
		::GetModuleFileNameW(NULL, pwchPath, MAX_UNICODE_PATH);
		WideCharToMultiByte(CP_UTF8, 0, pwchPath, -1, pchPath, MAX_UNICODE_PATH_IN_UTF8, NULL, NULL);
		delete[] pwchPath;

		std::string sPath = pchPath;
		delete[] pchPath;
		return sPath;
#elif defined(OSX)
		char rchPath[1024];
		uint32_t nBuff = sizeof(rchPath);
		bool bSuccess = _NSGetExecutablePath(rchPath, &nBuff) == 0;
		rchPath[nBuff - 1] = '\0';
		if (bSuccess)
			return rchPath;
		else
			return "";
#elif defined LINUX
		char rchPath[1024];
		size_t nBuff = sizeof(rchPath);
		ssize_t nRead = readlink("/proc/self/exe", rchPath, nBuff - 1);
		if (nRead != -1)
		{
			rchPath[nRead] = 0;
			return rchPath;
		}
		else
		{
			return "";
		}
#else
		AssertMsg(false, "Implement Plat_GetExecutablePath");
		return "";
#endif
	}

	//-----------------------------------------------------------------------------
	// Purpose: Helper to get a string from a tracked device property and turn it
	//			into a std::string
	//-----------------------------------------------------------------------------
	std::string GetTrackedDeviceString(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL)
	{
		uint32_t unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
		if (unRequiredBufferLen == 0)
			return "";

		char *pchBuffer = new char[unRequiredBufferLen];
		unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
		std::string sResult = pchBuffer;
		delete[] pchBuffer;
		return sResult;
	}
}

App::App() {}

App::~App()
{
	Terminate();
}

void App::Terminate()
{
	if (m_hmd)
	{
		CMG_LOG_INFO() << "Shutting down VR Runtime";
		vr::VR_Shutdown();
		m_hmd = nullptr;
	}
}

void App::OnInitialize()
{
	CMG_LOG_INFO() << "Initializing VR Runtime";

	// Loading the SteamVR Runtime
	vr::EVRInitError error = vr::VRInitError_None;
	m_hmd = vr::VR_Init(&error, vr::VRApplication_Background);
	if (error != vr::VRInitError_None || m_hmd == nullptr)
	{
		m_hmd = nullptr;
		CMG_LOG_ERROR() << "Failed to initialize VR runtime: " << vr::VR_GetVRInitErrorAsEnglishDescription(error);
		Quit();
		return;
	}

	// Get HMD info
	std::string hmdTrackingSystem = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	std::string hmdSerialNumber = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);
	CMG_LOG_INFO() << "HMD Tracking System: " << hmdTrackingSystem;
	CMG_LOG_INFO() << "HMD Serial Number: " << hmdSerialNumber;

	// Load actions
	Path configDir = GetExecutablePath().GetParent() / "../../config";
	Path actionManifestPath = configDir / "actions.json";
	CMG_LOG_INFO() << "Action Manifest Path: " << actionManifestPath;
	vr::VRInput()->SetActionManifestPath(actionManifestPath.c_str());
	m_actions = std::make_shared<Tf2ActionSet>();
	m_actions->Load(actionManifestPath);

	// Load bind mappings
	mappings::BindConfigLoader bindConfigLoader(m_bindMapper, *m_actions);
	Path bindConfigPath = configDir / "tf2_binds.json";
	bindConfigLoader.LoadConfig(bindConfigPath);

	// Create Aim Controller
	m_aimController = std::make_shared<mappings::SphereAimController>(
		m_rightController,
		m_bindMapper.GetInputOfType<inputs::Button>("enable_look"),
		m_bindMapper.GetOutputOfType<outputs::MouseMovement>("look_x"),
		m_bindMapper.GetOutputOfType<outputs::MouseMovement>("look_y"));
	m_aimController->SetName("Aim");
	m_bindMapper.AddBind(m_aimController);

	// Load assets
	auto resourceManager = GetResourceManager();
	resourceManager->LoadBuiltInFont(m_font, BuiltInFonts::FONT_CONSOLE);
}

void App::OnQuit()
{
	Terminate();
}

void App::OnResizeWindow(int width, int height)
{
}

void App::OnUpdate(float dt)
{
	Mouse *mouse = GetMouse();
	Keyboard *keyboard = GetKeyboard();

	// Escape: Quit
	if (keyboard->IsKeyPressed(Keys::escape))
	{
		Quit();
		return;
	}

	// Enter: toggle control mapping
	if (keyboard->IsKeyPressed(Keys::enter))
	{
		m_controlMappingEnabled = !m_controlMappingEnabled;
		return;
	}

	// Update VR actions
	m_actions->Update();

	// Get poses for all trackers
	std::array<vr::TrackedDevicePose_t, vr::k_unMaxTrackedDeviceCount> devicePoses;
	m_hmd->GetDeviceToAbsoluteTrackingPose(
		vr::TrackingUniverseStanding, 0.0f, devicePoses.data(), devicePoses.size());

	for (int index = 0; index < vr::k_unMaxTrackedDeviceCount; ++index)
	{
		auto it = m_devices.find(index);
		auto &pose = devicePoses[index];
		m_hmd->GetTrackedDeviceClass(index);
		if (m_hmd->GetTrackedDeviceClass(index) != vr::TrackedControllerRole_Invalid)
		{
			if (it == m_devices.end())
				auto device = AddDevice(index, pose);
			else
				UpdateDevice(it->second, pose);
		}
	}

	// Update control mapping
	if (m_controlMappingEnabled)
	{
		m_aimController->SetInputDevice(m_rightController);
		m_bindMapper.Update();
	}
}

std::shared_ptr<VrDevice> App::AddDevice(uint32_t index, const vr::TrackedDevicePose_t &pose)
{
	std::shared_ptr<VrDevice> device = std::make_shared<VrDevice>();
	device->index = index;
	m_devices[device->index] = device;
	UpdateDevice(device, pose);
	CMG_LOG_INFO() << "New Device: " << "index=" << index << ", type=\"" << device->type << "\"";
	return device;
}

void App::UpdateDevice(std::shared_ptr<VrDevice> device, const vr::TrackedDevicePose_t &pose)
{
	switch (m_hmd->GetTrackedDeviceClass(device->index))
	{
	case vr::TrackedDeviceClass_Controller:
	{
		device->type = "Controller";
		auto role = m_hmd->GetControllerRoleForTrackedDeviceIndex(device->index);
		if (role == vr::TrackedControllerRole_LeftHand)
		{
			device->type = "Controller (Left)";
			m_leftController = device;
		}
		else if (role == vr::TrackedControllerRole_RightHand)
		{
			device->type = "Controller (Right)";
			m_rightController = device;
		}
		break;
	}
	case vr::TrackedDeviceClass_HMD:
		device->type = "HMD";
		break;
	case vr::TrackedDeviceClass_Invalid:
		device->type = "Invalid";
		break;
	case vr::TrackedDeviceClass_GenericTracker:
		device->type = "Generic Tracker";
		break;
	case vr::TrackedDeviceClass_TrackingReference:
		device->type = "Tracking Reference";
		break;
	default:
		device->type = "Unknown";
		break;
	}

	device->connected = pose.bDeviceIsConnected;
	device->poseValid = pose.bPoseIsValid;

	// Update pose data
	if (device->poseValid)
	{
		device->velocity.x = pose.vVelocity.v[0];
		device->velocity.y = pose.vVelocity.v[1];
		device->velocity.z = pose.vVelocity.v[2];
		device->position.x = pose.mDeviceToAbsoluteTracking.m[0][3];
		device->position.y = pose.mDeviceToAbsoluteTracking.m[1][3];
		device->position.z = pose.mDeviceToAbsoluteTracking.m[2][3];
		for (size_t col = 0; col < 3; col++)
		{
			for (size_t row = 0; row < 3; row++)
			{
				device->orientation.c[col][row] = pose.mDeviceToAbsoluteTracking.m[row][col];
			}
		}
	}
}

void App::OnRender()
{
	auto renderDevice = GetRenderDevice();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Begin drawing HUD
	glUseProgram(0);
	Graphics2D g(GetWindow());
	g.Clear(Color::BLACK);
	g.SetWindowOrthoProjection();
	Vector2f windowSize((float)GetWindow()->GetWidth(), (float)GetWindow()->GetHeight());

	// Display device status
	float x = 16;
	for (auto it : m_devices)
	{
		std::stringstream ss;
		auto &device = it.second;
		ss << "Device " << device->index << "\n";
		ss << device->type << "\n";
		if (device->connected)
			ss << "connected\n";
		else
			ss << "disconnected\n";
		if (device->poseValid)
			ss << "pose valid\n";
		else
			ss << "pose invalid\n";
		ss << "position" << "\n";
		ss << "  x: " << device->position.x << "\n";
		ss << "  y: " << device->position.y << "\n";
		ss << "  z: " << device->position.z << "\n";
		ss << "direction" << "\n";
		auto direction = device->orientation.c1;
		ss << "  x: " << direction.x << "\n";
		ss << "  y: " << direction.y << "\n";
		ss << "  z: " << direction.z << "\n";
		std::string text = ss.str();
		auto textBounds = g.MeasureString(m_font.get(), text);
		Vector2f pos(x, 16.0f);
		textBounds.position = pos;
		textBounds.size.x = 160 - 8;
		textBounds.Inflate(4, 4);
		Color color = Color::YELLOW;
		if (!device->connected)
			color = Color::GRAY;
		else if (!device->poseValid)
			color = Color::RED;
		g.DrawString(m_font.get(), text, pos, color);
		g.DrawRect(textBounds, color);
		x += 160;
	}

	// Action list
	// {
	// 	std::stringstream ss;
	// 	ss << "VR Actions:\n";

	// 	size_t index = 0;
	// 	for (auto it : m_actions->GetActions())
	// 	{
	// 		it.second->DebugString(ss) << "\n";
	// 		index++;
	// 	}
	// 	g.DrawString(m_font.get(), ss.str(), Vector2f(16, 260), Color::YELLOW);
	// }

	{
		std::stringstream ss;
		ss << "Control Mapping: " << (m_controlMappingEnabled ? "ENABLED" : "DISABLED") << "\n";
		g.DrawString(m_font.get(), ss.str(), Vector2f(16, 240), Color::YELLOW);
	}

	// Input list
	{
		std::stringstream ss;
		ss << "Inputs:\n";
		size_t index = 0;
		for (auto it : m_bindMapper.GetInputs())
		{
			it.second->DebugString(ss) << "\n";
			index++;
		}
		g.DrawString(m_font.get(), ss.str(), Vector2f(16, 260), Color::YELLOW);
	}

	// Output list
	{
		std::stringstream ss;
		ss << "Outputs:\n";
		size_t index = 0;
		for (auto it : m_bindMapper.GetOutputs())
		{
			it.second->DebugString(ss) << "\n";
			index++;
		}
		g.DrawString(m_font.get(), ss.str(), Vector2f(300, 260), Color::YELLOW);
	}

	g.SetTransformation(
		Matrix4f::CreateTranslation(600, 420, 0) *
		Matrix4f::CreateScale(70.0f) *
		Matrix4f::CreateRotation(Vector3f::UNITZ, Math::PI));

	// Draw play area rect
	vr::IVRChaperone *chaperone = vr::VRChaperone();
	if (chaperone)
	{
		vr::HmdQuad_t rect;
		chaperone->GetPlayAreaRect(&rect);
		for (size_t i = 0; i < 4; i++)
		{
			auto v1 = rect.vCorners[i];
			auto v2 = rect.vCorners[(i + 1) % 4];
			Vector3f vec1(v1.v[0], v1.v[1], v1.v[2]);
			Vector3f vec2(v2.v[0], v2.v[1], v2.v[2]);
			g.DrawLine(vec1.GetXZ(), vec2.GetXZ(), Color::GRAY);
		}
	}

	// Draw devices
	for (auto &it : m_devices)
	{
		if (it.second->connected)
		{
			Matrix3f orientation = it.second->orientation;
			Vector3f direction = orientation * -Vector3f::UNITZ;
			g.DrawLine(
				it.second->position.GetXZ(),
				(it.second->position + direction * 0.2f).GetXZ(),
				Color::YELLOW);
			g.FillCircle(it.second->position.GetXZ(), 0.05f, Color::YELLOW);
		}
	}

	if (m_aimController->m_inputDevice)
	{
		g.DrawLine(
			m_aimController->m_center.GetXZ(),
			(m_aimController->m_center + m_aimController->m_directionOffset * m_aimController->m_radius).GetXZ(),
			Color::GRAY);
		g.DrawCircle(m_aimController->m_center.GetXZ(), m_aimController->m_radius, Color::GREEN);

		g.DrawLine(m_aimController->m_inputDevice->position.GetXZ(),
				   m_aimController->m_inputDevice->position.GetXZ() + m_aimController->m_direction.GetXZ(),
				   Color::GREEN);
		g.DrawLine(m_aimController->m_inputDevice->position.GetXZ(),
				   m_aimController->m_rayHitPoint.GetXZ(),
				   Color::DARK_RED);
		g.DrawLine(
			m_aimController->m_center.GetXZ(),
			(m_aimController->m_center + m_aimController->m_direction * m_aimController->m_radius).GetXZ(),
			Color::RED);
		g.FillCircle(m_aimController->m_rayHitPoint.GetXZ(), 0.04f, Color::RED);
		g.FillCircle(m_aimController->m_inputDevice->position.GetXZ(), 0.06f, Color::GREEN);
	}

	/*
	float w = 8;
	g.FillRect(0, 0, w * 16, w * 16, Color::GRAY);
	for (uint32 i = 0; i < m_colorPalette.size(); ++i) {
		uint32 row = i / 16;
		uint32 col = i % 16;
		g.FillRect(col * w, row * w, w, w, m_colorPalette[i].first);
	}
	g.DrawRect(0, 0, w * 16, w * 16, Color::WHITE);*/
}
