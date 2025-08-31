#pragma once

#include "actions.hpp"
#include <cmgInput/cmg_input.h>

namespace outputs
{

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

    protected:
        std::string m_name;
    };

    class Button : public OutputBase
    {
    public:
        virtual void Press() { m_down = true; }
        virtual void Release() { m_down = false; }

        virtual std::ostream &DebugString(std::ostream &stream) const
        {
            stream << m_name << ": " << (m_down ? "DOWN" : "UP");
            return stream;
        }

    protected:
        bool m_down = false;
    };

    class Axis : public OutputBase
    {
    public:
        virtual void SetValue(float value) {}
    };

    class MouseWheel : public OutputBase
    {
    public:
    };

    class KeyboardKey : public Button
    {
    public:
        KeyboardKey(int32_t scanCode) : scanCode(scanCode) {}

        virtual void Press() override;
        virtual void Release() override;

        int32_t scanCode = 0;
    };

    class MouseButton : public Button
    {
    public:
        MouseButton(MouseButtons::value_type button) : button(button) {}

        virtual void Press() override;
        virtual void Release() override;

        MouseButtons::value_type button = MouseButtons::left;
    };

    class MouseWheelButton : public Button
    {
    public:
        MouseWheelButton(bool positive) : positive(positive) {}

        virtual void Press() override;

        bool positive = true;
    };

    class MouseMovement : public Axis
    {
    public:
        MouseMovement(size_t axis) : m_axis(axis) {}

        virtual void SetValue(float value) override;

        virtual std::ostream &DebugString(std::ostream &stream) const
        {
            stream << m_name << ": " << m_value;
            return stream;
        }

    private:
        size_t m_axis = 0;
        int32_t m_value = 0;
    };

}