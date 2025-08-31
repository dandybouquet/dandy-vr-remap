#include "mappings/bindings.hpp"

namespace mappings
{

    BindMapper::BindMapper()
    {
    }

    void BindMapper::AddInput(std::shared_ptr<inputs::InputBase> input)
    {
        m_inputs[input->GetName()] = input;
    }

    void BindMapper::AddOutput(std::shared_ptr<outputs::OutputBase> output)
    {
        m_outputs[output->GetName()] = output;
    }

    void BindMapper::AddBind(std::shared_ptr<BindBase> bind)
    {
        CMG_LOG_DEBUG() << "Adding bind: " << bind->GetName();
        m_binds.push_back(bind);
    }

    void BindMapper::Update()
    {
        for (auto &it : m_inputs)
        {
            if (it.second)
                it.second->Update();
        }

        for (auto &bind : m_binds)
        {
            if (bind)
                bind->Update();
        }
    }

    void ButtonToButton::Update()
    {
        if (inverted)
        {
            if (input->IsPressed())
                output->Release();
            if (input->IsReleased())
                output->Press();
        }
        else
        {
            if (input->IsPressed())
                output->Press();
            if (input->IsReleased())
                output->Release();
        }
    }

    void AxisRangeToButton::AddRange(float minValue, float maxValue,
                                     std::shared_ptr<outputs::Button> output)
    {
        OutputRange range;
        range.minValue = minValue;
        range.maxValue = maxValue;
        range.inverted = false;
        range.output = output;
        outputs.push_back(range);
    }

    void AxisRangeToButton::Update()
    {
        float value = input->GetValue();
        for (auto &range : outputs)
        {
            bool activePrev = range.active;
            range.active = value >= range.minValue && value < range.maxValue;
            if (range.inverted)
                range.active = !range.active;
            if (range.active && !activePrev)
                range.output->Press();
            if (!range.active && activePrev)
                range.output->Release();
        }
    }

    void AxisToAxis::Update()
    {
        float value = input->GetValue();
        if (Math::Abs(value) < deadzone)
        {
            output->SetValue(0.0f);
        }
        else
        {
            value = Math::Abs(Math::Pow(value, sensitivity) * scale);
            value *= Math::Sign(input->GetValue());
            if (inverted)
                value = -value;
            output->SetValue(value);
        }
    }

}
