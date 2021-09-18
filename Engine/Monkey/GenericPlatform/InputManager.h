#pragma once

#include "Common/Common.h"
#include "Math/Vector2.h"

#include "MouseTypes.h"
#include "KeyboardTypes.h"

#include <unordered_map>
#include <vector>

class Application;

class InputManager
{
private:
    InputManager()
    {

    }

    ~InputManager()
    {

    }

public:

    static void Init();

    FORCE_INLINE static KeyboardType GetKeyFromKeyCode(int32 keyCode)
    {
        auto it = s_KeyboardTypesMap.find(keyCode);
        if (it == s_KeyboardTypesMap.end())
        {
            return KeyboardType::KEY_UNKNOWN;
        }
        return it->second;
    }

    FORCE_INLINE static void Reset()
    {
        s_MouseDelta = 0;
        s_IsMouseMoveing = false;
    }

    FORCE_INLINE static bool IsMouseDown(MouseType type)
    {
        auto it = s_MouseActions.find((int32)type);
        if (it == s_MouseActions.end())
        {
            return false;
        }
        return it->second == true;
    }

    FORCE_INLINE static bool IsMouseUp(MouseType type)
    {
        auto it = s_MouseActions.find((int32)type);
        if (it == s_MouseActions.end())
        {
            return false;
        }
        return it->second == false;
    }

    FORCE_INLINE static bool IsKeyDown(KeyboardType key)
    {
        auto it = s_KeyActions.find((int32)key);
        if (it == s_KeyActions.end())
        {
            return false;
        }
        return it->second == true;
    }

    FORCE_INLINE static bool IsKeyUp(KeyboardType key)
    {
        auto it = s_KeyActions.find((int32)key);
        if (it == s_KeyActions.end())
        {
            return false;
        }
        return it->second == false;
    }

    FORCE_INLINE static const Vector2& GetMousePosition()
    {
        return s_MouseLocation;
    }

    FORCE_INLINE static float GetMouseDelta()
    {
        return s_MouseDelta;
    }

    FORCE_INLINE static bool IsMouseMoving()
    {
        return s_IsMouseMoveing;
    }

protected:

    friend class Application;

    void static OnKeyDown(KeyboardType key);

    void static OnKeyUp(KeyboardType key);

    void static OnMouseDown(MouseType type, const Vector2& pos);

    void static OnMouseUp(MouseType type, const Vector2& pos);

    void static OnMouseMove(const Vector2& pos);

    void static OnMouseWheel(const float delta, const Vector2& pos);

private:

    static bool     s_IsMouseMoveing;
    static float    s_MouseDelta;
    static Vector2  s_MouseLocation;

    static std::unordered_map<int32, bool>              s_KeyActions;
    static std::unordered_map<int32, bool>              s_MouseActions;
    static std::unordered_map<int32, KeyboardType>      s_KeyboardTypesMap;
};
