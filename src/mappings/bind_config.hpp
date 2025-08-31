#pragma once

#include "vr/actions.hpp"
#include "outputs/outputs.hpp"
#include "mappings/bindings.hpp"
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