#pragma once

#include "actions.hpp"
#include "bindings.hpp"
#include "outputs.hpp"
#include "inputs.hpp"
#include "vr_device.hpp"
#include <cmgMath/cmg_math.h>

namespace mappings
{

    class SphereAimController : public BindBase
    {
    public:
        SphereAimController(
            std::shared_ptr<VrDevice> device,
            std::shared_ptr<inputs::Button> enableButton,
            std::shared_ptr<outputs::Axis> outputX,
            std::shared_ptr<outputs::Axis> outputY)
            : m_inputDevice(device),
              m_enableButton(enableButton),
              m_outputX(outputX),
              m_outputY(outputY)
        {
        }

        inline void SetInputDevice(
            std::shared_ptr<VrDevice> inputDevice) { m_inputDevice = inputDevice; }

        inline void SetEnabled(bool enabled) { m_enabled = enabled; }
        virtual void Update() override;

        float m_radius = 3.0f;
        bool m_enabled = false;
        float m_azimuthOffset = 0.0f;
        float m_elevationOffset = 0.0f;
        float m_azimuth = 0.0f;
        float m_elevation = 0.0f;
        Vector3f m_direction = Vector3f::ZERO;
        Vector3f m_directionOffset = Vector3f::ZERO;
        Vector3f m_rayHitPoint = Vector3f::ZERO;

        std::shared_ptr<VrDevice> m_inputDevice;
        std::shared_ptr<inputs::Button> m_enableButton;
        std::shared_ptr<outputs::Axis> m_outputX;
        std::shared_ptr<outputs::Axis> m_outputY;

        Vector2i m_mouseOffset = Vector2i::ZERO;
        Vector3f m_center = Vector3f::ZERO;
    };

}
