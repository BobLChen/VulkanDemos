#include "DVKCamera.h"
#include "GenericPlatform/InputManager.h"

namespace vk_demo
{

	DVKCamera::DVKCamera()
	{
		freeze = Vector3(0, 0, 0);
		m_LastMouse = InputManager::GetMousePosition();
	}

	void DVKCamera::Update(float time, float delta)
	{
		float mouseSpeedX = InputManager::GetMousePosition().x - m_LastMouse.x;
		float mouseSpeedY = InputManager::GetMousePosition().y - m_LastMouse.y;

		if (InputManager::IsMouseUp(MouseType::MOUSE_BUTTON_LEFT)) {
			m_Drag = false;
		}

		if (m_Drag)
		{
			if (InputManager::IsKeyDown(KeyboardType::KEY_SPACE))
			{
				m_World.TranslateX(-mouseSpeedX * m_World.GetOrigin().Size() / 300);
				m_World.TranslateY( mouseSpeedY * m_World.GetOrigin().Size() / 300);
			}
			else
			{
				m_SpinX += mouseSpeedX * smooth * speedFactor;
				m_SpinY += mouseSpeedY * smooth * speedFactor;
			}
		}

		if (InputManager::GetMouseDelta() != 0.0f) {
			m_SpinZ = (m_World.GetOrigin().Size() + 0.1f) * speedFactor * InputManager::GetMouseDelta() / 20.0f;
		}

		m_SpinX *= 1.0f - freeze.y;
		m_SpinY *= 1.0f - freeze.x;
		m_SpinZ *= 1.0f - freeze.z;

		m_World.TranslateZ(m_SpinZ);
		m_World.RotateY(m_SpinX, false, &Vector3::ZeroVector);
		m_World.RotateX(m_SpinY, true,  &Vector3::ZeroVector);

		m_SpinX *= (1 - smooth);
		m_SpinY *= (1 - smooth);
		m_SpinZ *= (1 - smooth);

		if (InputManager::IsMouseDown(MouseType::MOUSE_BUTTON_LEFT)) {
			m_Drag = true;
		}

		m_LastMouse = InputManager::GetMousePosition();
	}
}
