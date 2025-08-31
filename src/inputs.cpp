#include "inputs.hpp"

namespace inputs
{

    void Button::Update()
    {
        m_downPrev = m_down;
    }

    bool Button::IsDown() const
    {
        return m_down;
    }

    bool Button::IsPressed() const
    {
        return m_down && !m_downPrev;
    }

    bool Button::IsReleased() const
    {
        return !m_down && m_downPrev;
    }

    std::ostream &Button::DebugString(std::ostream &stream) const
    {
        stream << m_name << ": " << (IsDown() ? "DOWN" : "UP");
        return stream;
    }

    ButtonFromAction::ButtonFromAction(std::shared_ptr<ButtonAction> action)
        : m_action(action)
    {
    }

    void ButtonFromAction::Update()
    {
        Button::Update();
        m_down = m_action->down;
    }

    ButtonBooleanOp::ButtonBooleanOp(Operator op, std::shared_ptr<Button> left, std::shared_ptr<Button> right)
        : m_operator(op),
          m_left(left),
          m_right(right)
    {
    }

    void ButtonBooleanOp::Update()
    {
        Button::Update();
        m_left->Update();
        m_right->Update();
        if (m_operator == Operator::kAnd)
            m_down = m_left->IsDown() && m_right->IsDown();
        else if (m_operator == Operator::kOr)
            m_down = m_left->IsDown() || m_right->IsDown();
        else if (m_operator == Operator::kXor)
            m_down = m_left->IsDown() != m_right->IsDown();
    }

    ButtonUnaryOp::ButtonUnaryOp(Operator op, std::shared_ptr<Button> right)
        : m_operator(op),
          m_right(right)
    {
    }

    void ButtonUnaryOp::Update()
    {
        Button::Update();
        m_right->Update();
        if (m_operator == Operator::kNot)
            m_down = !m_right->IsDown();
        else
            m_down = m_right->IsDown();
    }

    std::ostream &Analog::DebugString(std::ostream &stream) const
    {
        stream << m_name << ": " << GetValue();
        return stream;
    }

    JoystickAxis::JoystickAxis(std::shared_ptr<JoystickAction> action, size_t axis)
        : m_action(action), m_axis(axis)
    {
    }

    void JoystickAxis::Update()
    {
        m_value = m_action->position[m_axis];
    }
}
