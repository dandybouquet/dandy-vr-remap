#pragma once

#include "actions.hpp"
#include "outputs.hpp"
#include "bindings.hpp"
#include <vector>

namespace mappings
{
    class BindConfigLoader
    {
    public:
        BindConfigLoader(BindMapper &mapper, ActionSet &actions);

        Error LoadConfig(const Path &path);

    private:
        BindMapper &m_mapper;
        ActionSet &m_actions;
        std::vector<std::shared_ptr<BindBase>> m_binds;
    };
}