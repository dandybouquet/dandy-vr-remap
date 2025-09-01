#include "outputs/outputs.hpp"
#include <Windows.h>

namespace outputs
{

	void Button::PreUpdate()
	{
		m_downPrev = m_down;
		m_down = false;
	}

    void Button::Update()
    {
        if (IsPressed())
            OnPressed();
        if (IsReleased())
            OnReleased();
    }

    void Analog::PreUpdate()
    {
        m_value = 0.0f;
    }

    void KeyboardKey::OnPressed()
    {
        INPUT ip;
        ip.type = INPUT_KEYBOARD;
        ip.ki.wScan = 0;
        ip.ki.time = 0;
        ip.ki.dwExtraInfo = 0;
        ip.ki.wVk = 0;
        ip.ki.wScan = scanCode;
        ip.ki.dwFlags = KEYEVENTF_SCANCODE;
        SendInput(1, &ip, sizeof(INPUT));
    }

    void KeyboardKey::OnReleased()
    {
        INPUT ip;
        ip.type = INPUT_KEYBOARD;
        ip.ki.wScan = 0;
        ip.ki.time = 0;
        ip.ki.dwExtraInfo = 0;
        ip.ki.wVk = 0;
        ip.ki.wScan = scanCode;
        ip.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
        SendInput(1, &ip, sizeof(INPUT));
    }

    void MouseButton::OnPressed()
    {
        INPUT input;
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        if (button == MouseButtons::right)
            input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        else if (button == MouseButtons::middle)
            input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
        SendInput(1, &input, sizeof(INPUT));
    }

    void MouseButton::OnReleased()
    {
        INPUT input;
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        if (button == MouseButtons::right)
            input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        else if (button == MouseButtons::middle)
            input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
        SendInput(1, &input, sizeof(INPUT));
    }

    void MouseWheelButton::OnPressed()
    {
        INPUT input;
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_WHEEL;
        input.mi.mouseData = positive ? 1 : -1;
        input.mi.dx = 0;
        input.mi.dy = 0;
        input.mi.time = 0;
        input.mi.dwExtraInfo = 0;
        SendInput(1, &input, sizeof(INPUT));
    }

    void MouseMovement::Update()
    {
        LONG intValue = static_cast<LONG>(m_value);
        if (intValue == 0)
            return;
        INPUT input;
        input.type = INPUT_MOUSE;
        input.mi.dx = m_axis == 0 ? intValue : 0;
        input.mi.dy = m_axis == 1 ? intValue : 0;
        input.mi.mouseData = 0;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        input.mi.time = 0;
        input.mi.dwExtraInfo = 0;
        SendInput(1, &input, sizeof(INPUT));

		Analog::Update();
	}

}
