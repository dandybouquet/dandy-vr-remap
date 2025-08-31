#pragma once

#include "bindings.hpp"
#include "sphere_aim_controller.hpp"
#include "inputs.hpp"
#include "outputs.hpp"

#include <set>

class LogicParser
{
public:
    static void Tokenize(const std::string &text, std::vector<std::string> &tokens);
    static std::shared_ptr<inputs::Button> ParseButtonLogic(
        const std::string &text, mappings::BindMapper &mapper, ActionSet &actions);
};
