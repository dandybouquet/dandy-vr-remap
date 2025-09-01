#pragma once

#include <cmgInput/cmg_input.h>

namespace outputs
{

    /// @brief Base class for all outputs
    class OutputBase
    {
    public:
        void SetName(const std::string &name) { m_name = name; }
        inline const std::string &GetName() const { return m_name; }

        virtual std::ostream &DebugString(std::ostream &stream) const
        {
            stream << m_name << ":";
            return stream;
        }

		virtual void PreUpdate() {}
		virtual void Update() {}

    protected:
        std::string m_name;
    };

    /// @brief Base class for binary/digital button outputs
    class Button : public OutputBase
    {
    public:
        bool IsDown() const { return m_down; }
        bool IsPressed() const { return m_down && !m_downPrev; }
        bool IsReleased() const { return !m_down && m_downPrev; }

        void SetState(bool state) { m_down = m_down || state; }
		 
        virtual void OnPressed() {}
        virtual void OnReleased() {}
		virtual void PreUpdate() override;
		virtual void Update() override;

        virtual std::ostream &DebugString(std::ostream &stream) const
        {
            stream << m_name << ": " << (m_down ? "DOWN" : "UP");
            return stream;
        }

    protected:
        bool m_down = false;
        bool m_downPrev = false;
    };

    /// @brief Base class for analog outputs with a single float value
    class Analog : public OutputBase
    {
    public:
        inline virtual float GetValue() const { return m_value; }
        inline virtual void SetValue(float value) { m_value += value; }

		virtual void PreUpdate() override;

		virtual std::ostream &DebugString(std::ostream &stream) const
		{
			stream << m_name << ": " << m_value;
			return stream;
		}

    protected:
        float m_value = 0;
    };

    /// @brief Button output which presses a key on the keyboard
    class KeyboardKey : public Button
    {
    public:
        KeyboardKey(int32_t scanCode) : scanCode(scanCode) {}

        virtual void OnPressed() override;
        virtual void OnReleased() override;

        int32_t scanCode = 0;
    };

    /// @brief Button output which presses a mouse button
    class MouseButton : public Button
    {
    public:
        MouseButton(MouseButtons::value_type button) : button(button) {}

        virtual void OnPressed() override;
        virtual void OnReleased() override;

        MouseButtons::value_type button = MouseButtons::left;
    };

    /// @brief Button output which moves the mouse wheel up or down one tick
    class MouseWheelButton : public Button
    {
    public:
        MouseWheelButton(bool positive) : positive(positive) {}

        virtual void OnPressed() override;

        bool positive = true;
    };

    /// @brief Analog output which moves the mouse cursor along the X or Y axis
    class MouseMovement : public Analog
    {
    public:
        MouseMovement(size_t axis) : m_axis(axis) {}

        virtual void Update() override;

    private:
        size_t m_axis = 0;
    };

}