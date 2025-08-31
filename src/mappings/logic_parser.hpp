#pragma once

#include "mappings/bindings.hpp"
#include "mappings/sphere_aim_controller.hpp"
#include "inputs/inputs.hpp"
#include "outputs/outputs.hpp"

#include <set>

class LogicParser
{
public:
    static void Tokenize(const std::string &text, std::vector<std::string> &tokens);
    static std::shared_ptr<inputs::Button> ParseButtonLogic(
        const std::string &text, mappings::BindMapper &mapper, ActionSet &actions);
};
