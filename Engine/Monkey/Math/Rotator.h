#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Utils/StringUtils.h"

#include "Math.h"
#include "Vector3.h"
#include "Axis.h"

struct Quat;

struct Rotator
{
public:
	float pitch;
	float yaw;
	float roll;

	static const Rotator ZeroRotator;

public:

	Rotator();

	Rotator(float inPitch, float inYaw, float inRoll);

	explicit Rotator(float inF);

	explicit Rotator(const Quat& quat);

	FORCE_INLINE Rotator operator+(const Rotator& r) const;

	FORCE_INLINE Rotator operator-(const Rotator& r) const;

	FORCE_INLINE Rotator operator*(float scale) const;

	FORCE_INLINE Rotator operator*=(float scale);

	FORCE_INLINE bool operator==(const Rotator& r) const;

	FORCE_INLINE bool operator!=(const Rotator& v) const;

	FORCE_INLINE Rotator operator+=(const Rotator& r);

	FORCE_INLINE Rotator operator-=(const Rotator& r);

	FORCE_INLINE bool IsNearlyZero(float tolerance = KINDA_SMALL_NUMBER) const;

	FORCE_INLINE bool IsZero() const;

	FORCE_INLINE bool Equals(const Rotator& r, float tolerance = KINDA_SMALL_NUMBER) const;

	FORCE_INLINE Rotator Add(float deltaPitch, float deltaYaw, float deltaRoll);

	FORCE_INLINE Rotator GetInverse() const;

	FORCE_INLINE Rotator GridSnap(const Rotator& rotGrid) const;

	FORCE_INLINE Vector3 GetVector() const;

	Quat Quaternion() const;

	FORCE_INLINE Vector3 Euler() const;

	FORCE_INLINE Vector3 RotateVector(const Vector3& v) const;

	FORCE_INLINE Vector3 UnrotateVector(const Vector3& v) const;

	FORCE_INLINE Rotator Clamp() const;

	FORCE_INLINE Rotator GetNormalized() const;

	FORCE_INLINE Rotator GetDenormalized() const;

	FORCE_INLINE float GetComponentForAxis(Axis::Type Axis) const;

	FORCE_INLINE void SetComponentForAxis(Axis::Type Axis, float Component);

	FORCE_INLINE void Normalize();

	FORCE_INLINE void GetWindingAndRemainder(Rotator& winding, Rotator& remainder) const;

	FORCE_INLINE float GetManhattanDistance(const Rotator & rotator) const;

	FORCE_INLINE Rotator GetEquivalentRotator() const;

	FORCE_INLINE void SetClosestToMe(Rotator& makeClosest) const;

	FORCE_INLINE std::string ToString() const;

	FORCE_INLINE bool ContainsNaN() const;

	FORCE_INLINE static float ClampAxis(float angle);

	FORCE_INLINE static float NormalizeAxis(float angle);

	FORCE_INLINE static uint8 CompressAxisToByte(float angle);

	FORCE_INLINE static float DecompressAxisFromByte(uint8 angle);

	FORCE_INLINE static uint16 CompressAxisToShort(float angle);

	FORCE_INLINE static float DecompressAxisFromShort(uint16 angle);

	FORCE_INLINE static Rotator MakeFromEuler(const Vector3& Euler);

	FORCE_INLINE void DiagnosticCheckNaN() const
	{

	}

	FORCE_INLINE void DiagnosticCheckNaN(const char* message) const
	{

	}
};

FORCE_INLINE Rotator::Rotator()
{

}

FORCE_INLINE Rotator::Rotator(float inF)
	: pitch(inF)
	, yaw(inF)
	, roll(inF)
{
	DiagnosticCheckNaN();
}

FORCE_INLINE Rotator::Rotator(float inPitch, float inYaw, float inRoll)
	: pitch(inPitch)
	, yaw(inYaw)
	, roll(inRoll)
{
	DiagnosticCheckNaN();
}

FORCE_INLINE Rotator Rotator::operator+(const Rotator& r) const
{
	return Rotator(pitch + r.pitch, yaw + r.yaw, roll + r.roll);
}

FORCE_INLINE Rotator Rotator::operator-(const Rotator& r) const
{
	return Rotator(pitch - r.pitch, yaw - r.yaw, roll - r.roll);
}

FORCE_INLINE Rotator Rotator::operator*(float scale) const
{
	return Rotator(pitch * scale, yaw * scale, roll * scale);
}

FORCE_INLINE Rotator Rotator::operator*= (float scale)
{
	pitch = pitch * scale; 
	yaw   = yaw   * scale;
	roll  = roll  * scale;
	DiagnosticCheckNaN();
	return *this;
}

FORCE_INLINE bool Rotator::operator==(const Rotator& r) const
{
	return pitch == r.pitch && yaw == r.yaw && roll == r.roll;
}

FORCE_INLINE bool Rotator::operator!=(const Rotator& v) const
{
	return pitch != v.pitch || yaw != v.yaw || roll != v.roll;
}

FORCE_INLINE Rotator Rotator::operator+=(const Rotator& r)
{
	pitch += r.pitch; 
	yaw   += r.yaw; 
	roll  += r.roll;
	DiagnosticCheckNaN();
	return *this;
}

FORCE_INLINE Rotator Rotator::operator-=(const Rotator& r)
{
	pitch -= r.pitch; 
	yaw   -= r.yaw; 
	roll  -= r.roll;
	DiagnosticCheckNaN();
	return *this;
}

FORCE_INLINE bool Rotator::IsNearlyZero(float tolerance) const
{
	return MMath::Abs(NormalizeAxis(pitch)) <= tolerance
		&& MMath::Abs(NormalizeAxis(yaw))   <= tolerance
		&& MMath::Abs(NormalizeAxis(roll))  <= tolerance;
}

FORCE_INLINE bool Rotator::IsZero() const
{
	return (ClampAxis(pitch) == 0.f) && (ClampAxis(yaw) == 0.f) && (ClampAxis(roll) == 0.f);
}

FORCE_INLINE bool Rotator::Equals(const Rotator& rhs, float tolerance) const
{
	return (MMath::Abs(NormalizeAxis(pitch - rhs.pitch)) <= tolerance)
		&& (MMath::Abs(NormalizeAxis(yaw   - rhs.yaw))   <= tolerance)
		&& (MMath::Abs(NormalizeAxis(roll  - rhs.roll))  <= tolerance);
}

FORCE_INLINE Rotator Rotator::Add(float deltaPitch, float deltaYaw, float deltaRoll)
{
	yaw   += deltaYaw;
	pitch += deltaPitch;
	roll  += deltaRoll;
	DiagnosticCheckNaN();
	return *this;
}

FORCE_INLINE Rotator Rotator::GridSnap(const Rotator& rotGrid) const
{
	return Rotator
	(
		MMath::GridSnap(pitch, rotGrid.pitch),
		MMath::GridSnap(yaw,   rotGrid.yaw),
		MMath::GridSnap(roll,  rotGrid.roll)
	);
}

FORCE_INLINE Rotator Rotator::Clamp() const
{
	return Rotator(ClampAxis(pitch), ClampAxis(yaw), ClampAxis(roll));
}

FORCE_INLINE float Rotator::ClampAxis(float angle)
{
	angle = MMath::Fmod(angle, 360.f);
	if (angle < 0.f) {
		angle += 360.f;
	}
	return angle;
}

FORCE_INLINE float Rotator::NormalizeAxis(float angle)
{
	angle = ClampAxis(angle);
	if (angle > 180.f) {
		angle -= 360.f;
	}
	return angle;
}

FORCE_INLINE uint8 Rotator::CompressAxisToByte(float angle)
{
	return MMath::RoundToInt(angle * 256.f / 360.f) & 0xFF;
}

FORCE_INLINE float Rotator::DecompressAxisFromByte(uint8 angle)
{
	return (angle * 360.f / 256.f);
}

FORCE_INLINE uint16 Rotator::CompressAxisToShort(float angle)
{
	return MMath::RoundToInt(angle * 65536.f / 360.f) & 0xFFFF;
}

FORCE_INLINE float Rotator::DecompressAxisFromShort(uint16 angle)
{
	return (angle * 360.f / 65536.f);
}

FORCE_INLINE Rotator Rotator::GetNormalized() const
{
	Rotator rot = *this;
	rot.Normalize();
	return rot;
}

FORCE_INLINE Rotator Rotator::GetDenormalized() const
{
	Rotator rot = *this;
	rot.pitch = ClampAxis(rot.pitch);
	rot.yaw   = ClampAxis(rot.yaw);
	rot.roll  = ClampAxis(rot.roll);
	return rot;
}

FORCE_INLINE void Rotator::Normalize()
{
	pitch = NormalizeAxis(pitch);
	yaw   = NormalizeAxis(yaw);
	roll  = NormalizeAxis(roll);
	DiagnosticCheckNaN();
}

FORCE_INLINE float Rotator::GetComponentForAxis(Axis::Type axis) const
{
	switch (axis)
	{
	case Axis::X:
		return roll;
	case Axis::Y:
		return pitch;
	case Axis::Z:
		return yaw;
	default:
		return 0.f;
	}
}

FORCE_INLINE void Rotator::SetComponentForAxis(Axis::Type axis, float component)
{
	switch (axis)
	{
    case Axis::Axis_None:
        break;
    case Axis::X:
		roll = component;
		break;
	case Axis::Y:
		pitch = component;
		break;
	case Axis::Z:
		yaw = component;
		break;
	}
}

FORCE_INLINE Vector3 Rotator::GetVector() const
{
	float cp, sp, cy, sy;
	MMath::SinCos(&sp, &cp, MMath::DegreesToRadians(pitch));
	MMath::SinCos(&sy, &cy, MMath::DegreesToRadians(yaw));
	Vector3 v = Vector3(cp * cy, cp * sy, sp);
	return v;
}

FORCE_INLINE Vector3 Rotator::Euler() const
{
	return Vector3(roll, pitch, yaw);
}

FORCE_INLINE Rotator Rotator::MakeFromEuler(const Vector3& euler)
{
	return Rotator(euler.x, euler.y, euler.z);
}

FORCE_INLINE void Rotator::GetWindingAndRemainder(Rotator& winding, Rotator& remainder) const
{
	remainder.yaw = NormalizeAxis(yaw);
	winding.yaw = yaw - remainder.yaw;

	remainder.pitch = NormalizeAxis(pitch);
	winding.pitch = pitch - remainder.pitch;

	remainder.roll = NormalizeAxis(roll);
	winding.roll = roll - remainder.roll;
}

FORCE_INLINE std::string Rotator::ToString() const
{
	return StringUtils::Printf("P=%f Y=%f r=%f", pitch, yaw, roll);
}

FORCE_INLINE bool Rotator::ContainsNaN() const
{
	return 
		!MMath::IsFinite(pitch) ||
		!MMath::IsFinite(yaw) ||
		!MMath::IsFinite(roll);
}

FORCE_INLINE float Rotator::GetManhattanDistance(const Rotator & rotator) const
{
	return MMath::Abs<float>(yaw - rotator.yaw) + MMath::Abs<float>(pitch - rotator.pitch) + MMath::Abs<float>(roll - rotator.roll);
}

FORCE_INLINE Rotator Rotator::GetEquivalentRotator() const
{
	return Rotator(180.0f - pitch, yaw + 180.0f, roll + 180.0f);
}

FORCE_INLINE void Rotator::SetClosestToMe(Rotator& makeClosest) const
{
	Rotator otherChoice = makeClosest.GetEquivalentRotator();
	float firstDiff  = GetManhattanDistance(makeClosest);
	float secondDiff = GetManhattanDistance(otherChoice);
	if (secondDiff < firstDiff) {
		makeClosest = otherChoice;
	}
}

// FORCE_INLINE functions
template<class U>
FORCE_INLINE Rotator MMath::Lerp(const Rotator& a, const Rotator& b, const U& alpha)
{
	return a + (b - a).GetNormalized() * alpha;
}

template<class U>
FORCE_INLINE Rotator MMath::LerpRange(const Rotator& a, const Rotator& b, const U& alpha)
{
	return (a * (1 - alpha) + b * alpha).GetNormalized();
}
