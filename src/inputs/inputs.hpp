#pragma once

#include "vr/actions.hpp"
#include <string>

namespace inputs
{
    /// @brief Base class for all inputs
    class InputBase
    {
    public:
        virtual void Update() {}

        void SetName(const std::string &name) { m_name = name; }
        inline const std::string &GetName() const { return m_name; }

        virtual std::ostream &DebugString(std::ostream &stream) const
        {
            stream << m_name << ":";
            return stream;
        }

        virtual std::string ToString() const
        {
            return std::string("(") + m_name + ")";
        }

    protected:
        std::string m_name;
    };

    /// @brief Base class for analog inputs which return a single float value
    class Analog : public InputBase
    {
    public:
        inline virtual float GetValue() const { return m_value; }

        virtual void Update() override;

        virtual std::ostream &DebugString(std::ostream &stream) const override;

    protected:
        float m_value = 0.0f;
    };

    /// @brief Analog input which gets the position of a joystick axis
    class JoystickAxis : public Analog
    {
    public:
        JoystickAxis(std::shared_ptr<JoystickAction> action, size_t axis);

        virtual void Update() override;

    protected:
        std::shared_ptr<JoystickAction> m_action;
        size_t m_axis = 0;
    };

    /// @brief Base class for button inputs which have a boolean on/off state
    class Button : public InputBase
    {
    public:
        virtual void Update() override;

        virtual bool IsDown() const;
        virtual bool IsPressed() const;
        virtual bool IsReleased() const;

        virtual std::ostream &DebugString(std::ostream &stream) const override;

    protected:
        bool m_down = false;
        bool m_downPrev = false;
    };

    /// @brief Button input which gets the state of a VR Button Action
    class ButtonFromAction : public Button
    {
    public:
        ButtonFromAction(std::shared_ptr<ButtonAction> action);

        virtual void Update() override;

        virtual std::string ToString() const
        {
            return m_action->identifier;
        }

    protected:
        std::shared_ptr<ButtonAction> m_action;
    };

    /// @brief Binary logic operation between two buttons. Supports 'AND', and
    /// 'OR'.
    class ButtonBooleanOp : public Button
    {
    public:
        enum class Operator
        {
            kAnd,
            kOr,
            kXor,
        };

        ButtonBooleanOp(Operator op, std::shared_ptr<Button> left,
                        std::shared_ptr<Button> right);

        virtual void Update() override;

        virtual std::string ToString() const override
        {
            std::string str = m_left->ToString();
            if (m_operator == Operator::kAnd)
                str += " && ";
            else if (m_operator == Operator::kOr)
                str += " || ";
            else if (m_operator == Operator::kXor)
                str += " != ";
            str += m_right->ToString();
            return str;
        }

    private:
        Operator m_operator = Operator::kAnd;
        std::shared_ptr<Button> m_left;
        std::shared_ptr<Button> m_right;
    };

    /// @brief Unary logic operation for a button. Only 'NOT' is supported.
    class ButtonUnaryOp : public Button
    {
    public:
        enum class Operator
        {
            kNot,
        };

        ButtonUnaryOp(Operator op, std::shared_ptr<Button> right);

        virtual void Update() override;

        virtual std::string ToString() const override
        {
            std::string str = "";
            if (m_operator == Operator::kNot)
                str += "!";
            str += m_right->ToString();
            return str;
        }

    private:
        Operator m_operator = Operator::kNot;
        std::shared_ptr<Button> m_right;
    };
}