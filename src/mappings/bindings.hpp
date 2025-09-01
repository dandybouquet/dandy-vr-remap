#pragma once

#include "vr/actions.hpp"
#include "outputs/outputs.hpp"
#include "inputs/inputs.hpp"
#include <vector>

namespace mappings
{

    /// @brief Base class for all input-to-output mappings
    class BindBase
    {
    public:
        /// @brief Update the bind, reading input state and updating output
        /// state
        virtual void Update() {}

        void SetName(const std::string &name) { m_name = name; }
        inline const std::string &GetName() const { return m_name; }

    protected:
        std::string m_name;
    };

    /// @brief Maps a button input to a button output
    class ButtonToButton : public BindBase
    {
    public:
        ButtonToButton(
            std::shared_ptr<inputs::Button> input,
            std::shared_ptr<outputs::Button> output) : input(input), output(output) {}

        virtual void Update() override;

    private:
        std::shared_ptr<inputs::Button> input;
        std::shared_ptr<outputs::Button> output;
        bool inverted = false;
    };

    /// @brief Maps one or more ranges of an analog value to button presses
    class AxisRangeToButton : public BindBase
    {
    public:
        AxisRangeToButton(
            std::shared_ptr<inputs::Analog> input) : input(input) {}

        void AddRange(float minValue, float maxValue,
                      std::shared_ptr<outputs::Button> output);

        virtual void Update() override;

    private:
        struct OutputRange
        {
            float minValue = 0.0f;
            float maxValue = 0.0f;
            bool inverted = false;
            std::shared_ptr<outputs::Button> output;
            bool active = false;
        };

        std::vector<OutputRange> outputs;
        std::shared_ptr<inputs::Analog> input;
    };

    /// @brief Maps an input analog value to an output analog value with optional
    /// scaling and deadzone
    class AxisToAxis : public BindBase
    {
    public:
        AxisToAxis(
            std::shared_ptr<inputs::Analog> input,
            std::shared_ptr<outputs::Analog> output) : input(input), output(output) {}

        virtual void Update() override;

        std::shared_ptr<inputs::Analog> input;
        std::shared_ptr<outputs::Analog> output;
        float scale = 1.0f;
        float sensitivity = 1.0f;
        float deadzone = 0.0f;
        bool inverted = false;
    };

    /// @brief Manager class which can process multiple bind mappings
    class BindMapper
    {
    public:
        using InputMap = std::map<std::string, std::shared_ptr<inputs::InputBase>>;
        using OutputMap = std::map<std::string, std::shared_ptr<outputs::OutputBase>>;

        BindMapper();

        inline InputMap &GetInputs() { return m_inputs; }
        inline OutputMap &GetOutputs() { return m_outputs; }

        template <class T>
        std::shared_ptr<T> GetInputOfType(const std::string &name)
        {
            auto it = m_inputs.find(name);
            if (it == m_inputs.end())
                return nullptr;
            return std::dynamic_pointer_cast<T>(it->second);
        }

        template <class T>
        std::shared_ptr<T> GetOutputOfType(const std::string &name)
        {
            auto it = m_outputs.find(name);
            if (it == m_outputs.end())
                return nullptr;
            return std::dynamic_pointer_cast<T>(it->second);
        }

        /// @brief Add a new input
        /// @param input the input to add
        void AddInput(std::shared_ptr<inputs::InputBase> input);

        /// @brief Add a new output
        /// @param output the output to add
        void AddOutput(std::shared_ptr<outputs::OutputBase> output);

        /// @brief Add a new bind mapping
        /// @param bind the bind mapping to add
        void AddBind(std::shared_ptr<BindBase> bind);

        /// @brief Updates all bind mappings
        void Update();

    private:
        InputMap m_inputs;
        OutputMap m_outputs;
        std::vector<std::shared_ptr<BindBase>> m_binds;
    };

}