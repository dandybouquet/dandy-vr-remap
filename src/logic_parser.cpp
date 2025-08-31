#include "logic_parser.hpp"
#include "sphere_aim_controller.hpp"
#include "inputs.hpp"
#include "outputs.hpp"

#include "rapidjson/prettywriter.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/document.h"
#include <set>

void LogicParser::Tokenize(const std::string &text, std::vector<std::string> &tokens)
{
    std::set<std::string> double_special_chars = {"&&", "||"};
    std::set<char> special_chars = {'(', '(', '!', '&', '|'};

    std::string token = cmg::string::Trim(text.substr(0, 1));

    // Iterate every character in the string
    size_t length = text.length();
    for (size_t index = 1; index < length + 1; index++)
    {
        char c = ' ';
        if (index < length)
            c = text[index];

        // Check if this character completes the token
        bool complete = false;
        if (index >= length)
            complete = true;
        else if (double_special_chars.find(token) != double_special_chars.end())
            complete = true;
        else if (token.length() == 1 && special_chars.find(token[0]) != special_chars.end())
        {
            std::string tokenWithChar = token;
            tokenWithChar += c;
            if (double_special_chars.find(tokenWithChar) == double_special_chars.end())
                complete = true;
        }
        else if (special_chars.find(c) != special_chars.end())
            complete = true;

        // If complete, then begin a new token
        if (complete)
        {
            cmg::string::TrimIP(token);
            if (token.length() > 0)
                tokens.push_back(token);
            token = "";
        }

        // Add the character to the token
        if (token.length() > 0 || c != ' ')
            token += c;
    }
}

std::shared_ptr<inputs::Button> LogicParser::ParseButtonLogic(
    const std::string &text, mappings::BindMapper &mapper, ActionSet &actions)
{
    struct Node
    {
        std::string token = "";
        std::shared_ptr<inputs::Button> input;
    };

    std::vector<std::string> tokens;
    std::vector<Node> nodes;
    LogicParser::Tokenize(text, tokens);
    for (auto token : tokens)
    {
        Node node;
        node.token = token;
        nodes.push_back(node);
    }

    // std::cout << "text: " << text << std::endl;
    // std::cout << "tokens: ";
    // for (auto token : tokens)
    //     std::cout << "\"" << token << "\", ";
    // std::cout << std::endl;

    bool updated = true;
    while (updated)
    {
        updated = false;

        for (size_t i = 0; i < nodes.size(); i++)
        {
            auto &node = nodes[i];
            bool last = i == tokens.size() - 1;
            bool first = i == 0;
            if (node.input != nullptr)
                continue;

            if (node.token == "!")
            {
                if (last)
                    return nullptr;
                if (nodes[i + 1].input)
                {
                    node.input = std::make_shared<inputs::ButtonUnaryOp>(
                        inputs::ButtonUnaryOp::Operator::kNot,
                        nodes[i + 1].input);
                    nodes.erase(nodes.begin() + i + 1);
                    updated = true;
                    i--;
                }
            }
            else if (node.token == "&&" || node.token == "||")
            {
                if (first || last)
                    return nullptr;
                if (nodes[i - 1].input && nodes[i + 1].input)
                {
                    node.input = std::make_shared<inputs::ButtonBooleanOp>(
                        node.token == "&&" ? inputs::ButtonBooleanOp::Operator::kAnd : inputs::ButtonBooleanOp::Operator::kOr,
                        nodes[i - 1].input, nodes[i + 1].input);
                    nodes.erase(nodes.begin() + i + 1);
                    nodes.erase(nodes.begin() + i - 1);
                    i -= 2;
                    updated = true;
                }
            }
            else
            {
                node.input = mapper.GetInputOfType<inputs::Button>(node.token);
                if (node.input == nullptr)
                {
                    auto action = actions.GetActionOfType<ButtonAction>(node.token);
                    if (action != nullptr)
                        node.input = std::make_shared<inputs::ButtonFromAction>(action);
                }
                if (node.input == nullptr)
                {
                    CMG_LOG_ERROR() << "Unknown input: " << node.token;
                    return nullptr;
                }
                updated = true;
            }
        }
    }

    if (nodes.size() != 1)
    {
        CMG_LOG_ERROR() << "error parsing logic: " << text;
        return nullptr;
    }
    else if (nodes[0].input == nullptr)
    {
        CMG_LOG_ERROR() << "error parsing logic: " << text;
    }

    return nodes[0].input;
}