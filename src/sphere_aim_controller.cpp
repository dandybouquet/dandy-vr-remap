#include "sphere_aim_controller.hpp"

namespace mappings
{

    void SphereAimController::Update()
    {
        if (!m_inputDevice || !m_inputDevice->connected || !m_inputDevice->poseValid)
            return;

        bool enabledPrev = m_enabled;
        m_enabled = m_enableButton->IsDown();
        Vector2i mouseOffsetPrev = m_mouseOffset;

        Matrix3f orientation = m_inputDevice->orientation;
        Vector3f direction = orientation * -Vector3f::UNITZ;

        float azimuth = Math::ATan2(direction.z, direction.x);
        float elevation = Math::ASin(direction.y);

        // Cast a ray from the controller to an imaginary sphere boundary around
        // the center point. Use the intersection point as the look direction.
        Sphere sphere;
        sphere.position = m_center;
        sphere.radius = m_radius;
        Ray ray;
        ray.origin = m_inputDevice->position;
        ray.direction = -direction;
        m_direction = direction;
        float rayHitDistance = 0.0f;
        m_rayHitPoint = m_center;
        if (sphere.CastRay(ray, rayHitDistance))
        {
            auto rayHitPoint = ray.GetPoint(rayHitDistance);
            m_rayHitPoint = rayHitPoint;
            direction = rayHitPoint - m_center;
            direction.Normalize();
        }

        // If became enabled, then reset center point and home angles
        if (!m_enabled)
        {
            m_mouseOffset = Vector2i::ZERO;
            m_center = m_inputDevice->position;
            m_azimuthOffset = azimuth;
            m_elevationOffset = elevation;
            m_directionOffset = direction;
        }

        // Get the delta azimuth/elevation angles
        float azDelta = azimuth - m_azimuthOffset;
        if (azDelta > Math::PI)
            azDelta -= Math::TWO_PI;
        if (azDelta < -Math::PI)
            azDelta += Math::TWO_PI;
        float elDelta = elevation - m_elevationOffset;
        m_azimuth = azimuth;
        m_elevation = elevation;

        // Convert az/el angles to pixels
        const float degreesToPixels = 40;
        m_mouseOffset.x = static_cast<int32_t>(Math::ToDegrees(azDelta) * degreesToPixels);
        m_mouseOffset.y = static_cast<int32_t>(-Math::ToDegrees(elDelta) * degreesToPixels);

        if (m_enabled)
        {
            m_outputX->SetValue(
                static_cast<float>(m_mouseOffset.x - mouseOffsetPrev.x));
            m_outputY->SetValue(
                static_cast<float>(m_mouseOffset.y - mouseOffsetPrev.y));
        }
        else
        {
            mouseOffsetPrev = m_mouseOffset;
            m_outputX->SetValue(0.0f);
            m_outputY->SetValue(0.0f);
        }
    }

}