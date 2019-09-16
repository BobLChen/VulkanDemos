#include "Common/Common.h"
#include "Common/Log.h"

#include "InputManager.h"

float InputManager::s_MouseDelta    = 0.0f;
bool InputManager::s_IsMouseMoveing = false;

Vector2 InputManager::s_MouseLocation(0.0f, 0.0f);

std::unordered_map<int32, bool>             InputManager::s_KeyActions;
std::unordered_map<int32, bool>             InputManager::s_MouseActions;
std::unordered_map<int32, KeyboardType>     InputManager::s_KeyboardTypesMap;

void InputManager::OnKeyDown(KeyboardType key)
{
    s_KeyActions[(int32)key] = true;
}

void InputManager::OnKeyUp(KeyboardType key)
{
    s_KeyActions[(int32)key] = false;
}

void InputManager::OnMouseDown(MouseType type, const Vector2& pos)
{
    s_MouseActions[(int32)type] = true;
    s_MouseLocation.Set(pos.x, pos.y);
}

void InputManager::OnMouseUp(MouseType type, const Vector2& pos)
{
    s_MouseActions[(int32)type] = false;
    s_MouseLocation.Set(pos.x, pos.y);
}

void InputManager::OnMouseMove(const Vector2& pos)
{
    s_IsMouseMoveing = true;
    s_MouseLocation.Set(pos.x, pos.y);
}

void InputManager::OnMouseWheel(const float delta, const Vector2& pos)
{
    s_MouseDelta = delta;
    s_MouseLocation.Set(pos.x, pos.y);
}