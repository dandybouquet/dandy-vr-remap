#include "mappings/bind_config.hpp"
#include "mappings/sphere_aim_controller.hpp"
#include "mappings/logic_parser.hpp"
#include "inputs/inputs.hpp"
#include "outputs/outputs.hpp"

#include "rapidjson/prettywriter.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/document.h"

namespace mappings
{
    namespace
    {
        class LoadFunctions
        {
        public:
            ActionSet &m_actions;
            BindMapper &m_mapper;

            explicit LoadFunctions(ActionSet &actions,
                                   BindMapper &mapper)
                : m_actions(actions), m_mapper(mapper)
            {
            }

            template <class T>
            std::shared_ptr<T> LoadMappingType(rapidjson::Value &data);

            template <class T>
            std::shared_ptr<T> LoadOutput(rapidjson::Value &data);

            template <class T>
            std::shared_ptr<T> LoadInput(rapidjson::Value &data);

            template <>
            std::shared_ptr<outputs::KeyboardKey> LoadOutput(rapidjson::Value &data)
            {
                std::string keyStr = cmg::string::ToLower(
                    std::string(data["key"].GetString()));
                uint8_t scanCode = 0;
                if (keyStr == "space")
                    scanCode = 57;
                else if (keyStr == "escape")
                    scanCode = 1;
                else if (keyStr == "tab")
                    scanCode = 15;
                else if (keyStr == "ctrl")
                    scanCode = 29;
                else if (keyStr == "w")
                    scanCode = 17;
                else if (keyStr == "e")
                    scanCode = 18;
                else if (keyStr == "r")
                    scanCode = 19;
                else if (keyStr == "a")
                    scanCode = 30;
                else if (keyStr == "s")
                    scanCode = 31;
                else if (keyStr == "d")
                    scanCode = 32;
                else if (keyStr == "f")
                    scanCode = 33;
                else if (keyStr == "g")
                    scanCode = 34;
                else if (keyStr == ",")
                    scanCode = 51;
                else if (keyStr == ".")
                    scanCode = 52;
                else
                {
                    CMG_LOG_ERROR() << "Unsupported keyboard key '" << keyStr << "'";
                    return nullptr;
                }
                return std::make_shared<outputs::KeyboardKey>(scanCode);
            }

            template <>
            std::shared_ptr<outputs::MouseButton> LoadOutput(rapidjson::Value &data)
            {
                std::string buttonStr = cmg::string::ToLower(
                    std::string(data["button"].GetString()));
                if (buttonStr == "left")
                    return std::make_shared<outputs::MouseButton>(MouseButtons::left);
                else if (buttonStr == "right")
                    return std::make_shared<outputs::MouseButton>(MouseButtons::right);
                else if (buttonStr == "middle")
                    return std::make_shared<outputs::MouseButton>(MouseButtons::middle);
                CMG_LOG_ERROR() << "Unsupported mouse button '" << buttonStr << "'";
                return nullptr;
            }

            template <>
            std::shared_ptr<outputs::MouseWheelButton> LoadOutput(rapidjson::Value &data)
            {
                std::string directionStr = cmg::string::ToLower(
                    std::string(data["direction"].GetString()));
                if (directionStr == "up")
                    return std::make_shared<outputs::MouseWheelButton>(true);
                else if (directionStr == "down")
                    return std::make_shared<outputs::MouseWheelButton>(false);
                CMG_LOG_ERROR() << "Unsupported mouse wheel direction '" << directionStr << "'";
                return nullptr;
            }

            template <>
            std::shared_ptr<outputs::MouseMovement> LoadOutput(rapidjson::Value &data)
            {
                size_t axis = 0;
                std::string axisStr = cmg::string::ToLower(std::string(data["axis"].GetString()));
                if (axisStr == "x")
                    axis = 0;
                else if (axisStr == "y")
                    axis = 1;
                else if (axisStr == "z")
                    axis = 2;
                return std::make_shared<outputs::MouseMovement>(axis);
            }

            template <>
            std::shared_ptr<outputs::Analog> LoadOutput(rapidjson::Value &data)
            {
                if (data.IsString())
                {
                    std::string value = data.GetString();
                    auto output = m_mapper.GetOutputOfType<outputs::Analog>(value);
                    if (output)
                        return output;
                    return nullptr;
                }

                std::string type = data["type"].GetString();
                if (type == "MouseMovement")
                    return LoadOutput<outputs::MouseMovement>(data);
                CMG_LOG_ERROR() << "Unsupported axis output type '" << type << "'";
                return nullptr;
            }

            template <>
            std::shared_ptr<outputs::Button> LoadOutput(rapidjson::Value &data)
            {
                if (data.IsString())
                {
                    std::string value = data.GetString();
                    auto output = m_mapper.GetOutputOfType<outputs::Button>(value);
                    if (output)
                        return output;
                    return nullptr;
                }

                std::string type = data["type"].GetString();
                if (type == "KeyboardKey")
                    return LoadOutput<outputs::KeyboardKey>(data);
                else if (type == "MouseButton")
                    return LoadOutput<outputs::MouseButton>(data);
                else if (type == "MouseWheelButton")
                    return LoadOutput<outputs::MouseWheelButton>(data);
                CMG_LOG_ERROR() << "Unsupported button output type '" << type << "'";
                return nullptr;
            }

            template <>
            std::shared_ptr<inputs::Button> LoadInput(rapidjson::Value &data)
            {
                if (data.IsString())
                {
                    std::string value = data.GetString();
                    auto input = m_mapper.GetInputOfType<inputs::Button>(value);
                    if (input)
                        return input;

                    auto action = m_actions.GetActionOfType<ButtonAction>(value);
                    if (action)
                        return std::make_shared<inputs::ButtonFromAction>(action);

                    input = LogicParser::ParseButtonLogic(value, m_mapper, m_actions);
                    if (input)
                        CMG_LOG_INFO() << "LOGIC: " << input->ToString();
                    return input;
                }
                else
                {
                    return nullptr;
                }
            }

            template <>
            std::shared_ptr<inputs::Analog> LoadInput(rapidjson::Value &data)
            {
                if (data.IsString())
                {
                    std::string value = data.GetString();
                    auto input = m_mapper.GetInputOfType<inputs::Analog>(value);
                    if (input)
                        return input;
                    return nullptr;
                }

                size_t axis = 0;
                if (data["axis"].IsInt())
                    axis = data["axis"].GetInt();
                else
                {
                    std::string axisStr = data["axis"].GetString();
                    if (axisStr == "x")
                        axis = 0;
                    else if (axisStr == "y")
                        axis = 1;
                    else if (axisStr == "z")
                        axis = 2;
                }

                std::string path = data["path"].GetString();
                auto action = m_actions.GetActionOfType<JoystickAction>(path);
                if (action)
                    return std::make_shared<inputs::JoystickAxis>(action, axis);
                return nullptr;
            }

            template <>
            std::shared_ptr<ButtonToButton> LoadMappingType(rapidjson::Value &data)
            {
                auto input = LoadInput<inputs::Button>(data["input"]);
                if (input == nullptr)
                    return nullptr;

                auto output = LoadOutput<outputs::Button>(data["output"]);
                if (output == nullptr)
                    return nullptr;

                return std::make_shared<ButtonToButton>(input, output);
            }

            template <>
            std::shared_ptr<AxisToAxis> LoadMappingType(rapidjson::Value &data)
            {
                auto input = LoadInput<inputs::Analog>(data["input"]);
                if (input == nullptr)
                    return nullptr;

                auto output = LoadOutput<outputs::Analog>(data["output"]);
                if (output == nullptr)
                    return nullptr;

                auto bind = std::make_shared<AxisToAxis>(input, output);
                if (data.HasMember("sensitivity"))
                    bind->sensitivity = data["sensitivity"].GetFloat();
                if (data.HasMember("deadzone"))
                    bind->deadzone = data["deadzone"].GetFloat();
                if (data.HasMember("scale"))
                    bind->scale = data["scale"].GetFloat();
                if (data.HasMember("inverted"))
                    bind->inverted = data["inverted"].GetBool();
                return bind;
            }

            template <>
            std::shared_ptr<AxisRangeToButton> LoadMappingType(rapidjson::Value &data)
            {
                auto input = LoadInput<inputs::Analog>(data["input"]);
                if (input == nullptr)
                    return nullptr;

                auto bind = std::make_shared<AxisRangeToButton>(input);

                rapidjson::Value &rangeListData = data["ranges"];
                for (auto it = rangeListData.Begin(); it != rangeListData.End(); it++)
                {
                    auto &rangeData = *it;
                    float minValue = rangeData["min"].GetFloat();
                    float maxValue = rangeData["max"].GetFloat();
                    auto output = LoadOutput<outputs::Button>(rangeData["output"]);
                    if (output == nullptr)
                        return nullptr;
                    bind->AddRange(minValue, maxValue, output);
                }
                return bind;
            }

            template <>
            std::shared_ptr<SphereAimController> LoadMappingType(rapidjson::Value &data)
            {
                CMG_LOG_ERROR() << "Not Implemented: LoadMappingType<SphereAimController>";
                return nullptr;
            }

            std::shared_ptr<BindBase> LoadMapping(
                rapidjson::Value &data)
            {
                std::string name = data["name"].GetString();
                std::string type = data["type"].GetString();
                std::shared_ptr<BindBase> bind = nullptr;
                CMG_LOG_DEBUG() << "Loading config for " << type << " \"" << name << "\"";

                if (type == "ButtonToButton")
                    bind = LoadMappingType<ButtonToButton>(data);
                else if (type == "AxisRangeToButtons")
                    bind = LoadMappingType<AxisRangeToButton>(data);
                else if (type == "AxisToAxis")
                    bind = LoadMappingType<AxisToAxis>(data);
                else if (type == "SphereAimController")
                    bind = LoadMappingType<SphereAimController>(data);
                else
                {
                    CMG_LOG_ERROR() << "Unsupported mapping type: \"" << type << "\"";
                }

                if (bind)
                    bind->SetName(name);

                return bind;
            }
        };
    }

    BindConfigLoader::BindConfigLoader(BindMapper &mapper, ActionSet &actions)
        : m_mapper(mapper), m_actions(actions)
    {
    }

    Error BindConfigLoader::LoadConfig(const Path &path)
    {
        CMG_LOG_INFO() << "Loading bind config: " << path;

        // Open the json file
        String json;
        rapidjson::Document document;
        Error error = File::OpenAndGetContents(path, json);
        if (error.Failed())
            return error.Uncheck();
        document.Parse(json.c_str());
        if (document.HasParseError())
            return CMG_ERROR(Error::kFileCorrupt);

        LoadFunctions loadFuncs(m_actions, m_mapper);

        CMG_LOG_DEBUG() << "Loading button inputs";
        rapidjson::Value &inputListButtons = document["inputs"]["buttons"];
        std::map<std::string, std::shared_ptr<inputs::Button>> buttonInputs;
        for (auto it = inputListButtons.MemberBegin(); it != inputListButtons.MemberEnd(); it++)
        {
            std::string name = it->name.GetString();
            std::shared_ptr<inputs::Button> input = loadFuncs.LoadInput<inputs::Button>(it->value);
            if (input)
            {
                CMG_LOG_DEBUG() << "  " << name;
                input->SetName(name);
                buttonInputs[name] = input;
                m_mapper.AddInput(input);
            }
        }

        CMG_LOG_DEBUG() << "Loading analog inputs";
        rapidjson::Value &inputListAnalog = document["inputs"]["analog"];
        std::map<std::string, std::shared_ptr<inputs::Analog>> analogInputs;
        for (auto it = inputListAnalog.MemberBegin(); it != inputListAnalog.MemberEnd(); it++)
        {
            std::string name = it->name.GetString();
            std::shared_ptr<inputs::Analog> input = loadFuncs.LoadInput<inputs::Analog>(it->value);
            if (input)
            {
                CMG_LOG_DEBUG() << "  " << name;
                input->SetName(name);
                analogInputs[name] = input;
                m_mapper.AddInput(input);
            }
        }

        CMG_LOG_DEBUG() << "Loading button outputs";
        rapidjson::Value &outputListButtons = document["outputs"]["buttons"];
        std::map<std::string, std::shared_ptr<outputs::Button>> buttonOutputs;
        for (auto it = outputListButtons.MemberBegin(); it != outputListButtons.MemberEnd(); it++)
        {
            std::string name = it->name.GetString();
            auto output = loadFuncs.LoadOutput<outputs::Button>(it->value);
            if (output)
            {
                CMG_LOG_DEBUG() << "  " << name;
                output->SetName(name);
                buttonOutputs[name] = output;
                m_mapper.AddOutput(output);
            }
        }

        CMG_LOG_DEBUG() << "Loading analog outputs";
        rapidjson::Value &outputListAnalog = document["outputs"]["analog"];
        for (auto it = outputListAnalog.MemberBegin(); it != outputListAnalog.MemberEnd(); it++)
        {
            std::string name = it->name.GetString();
            auto output = loadFuncs.LoadOutput<outputs::Analog>(it->value);
            if (output)
            {
                CMG_LOG_DEBUG() << "  " << name;
                output->SetName(name);
                m_mapper.AddOutput(output);
            }
        }

        CMG_LOG_DEBUG() << "Automatically mapping buttons";
        for (auto iterOut : buttonOutputs)
        {
            std::string name = iterOut.first;
            auto iterIn = buttonInputs.find(name);
            if (iterIn != buttonInputs.end())
            {
                auto bind = std::make_shared<ButtonToButton>(iterIn->second, iterOut.second);
                bind->SetName(name);
                m_mapper.AddBind(bind);
                CMG_LOG_DEBUG() << "  " << name;
            }
        }

        // Process all mappings
        CMG_LOG_DEBUG() << "Loading mappings";

        rapidjson::Value &mappingData = document["mappings"];
        for (auto it = mappingData.Begin(); it != mappingData.End(); it++)
        {
            std::shared_ptr<BindBase> mapping = loadFuncs.LoadMapping(*it);
            if (mapping)
                m_mapper.AddBind(mapping);
        }

        return CMG_ERROR_SUCCESS;
    }
}
