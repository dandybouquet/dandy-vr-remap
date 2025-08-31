#pragma once

#include "vr/actions.hpp"
#include <string>

namespace inputs
{

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

    class Analog : public InputBase
    {
    public:
        inline virtual float GetValue() const = 0;

        virtual std::ostream &DebugString(std::ostream &stream) const override;
    };

    class JoystickAxis : public Analog
    {
    public:
        JoystickAxis(std::shared_ptr<JoystickAction> action, size_t axis);

        virtual void Update() override;

        inline virtual float GetValue() const { return m_value; }

    protected:
        std::shared_ptr<JoystickAction> m_action;
        float m_value = 0.0f;
        size_t m_axis = 0;
    };

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

    class ButtonBooleanOp : public Button
    {
    public:
        enum class Operator
        {
            kAnd,
            kOr,
            kXor,
        };

        ButtonBooleanOp(Operator op, std::shared_ptr<Button> left, std::shared_ptr<Button> right);

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