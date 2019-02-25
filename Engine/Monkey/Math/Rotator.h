#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Math.h"
#include "Vector.h"
#include "Axis.h"
#include "Utils/StringUtils.h"

struct Quat;

struct Rotator
{
public:
	float pitch;
	float yaw;
	float roll;

	static const Rotator ZeroRotator;

public:

	FORCEINLINE void DiagnosticCheckNaN() const 
	{

	}

	FORCEINLINE void DiagnosticCheckNaN(const char* message) const 
	{

	}

	FORCEINLINE Rotator() 
	{ 

	}

	explicit FORCEINLINE Rotator(float inF);

	FORCEINLINE Rotator(float inPitch, float inYaw, float inRoll);

	explicit Rotator(const Quat& Quat);

	Rotator operator+(const Rotator& r) const;

	Rotator operator-(const Rotator& r) const;

	Rotator operator*(float scale) const;

	Rotator operator*=(float scale);

	bool operator==(const Rotator& r) const;

	bool operator!=(const Rotator& v) const;

	Rotator operator+=(const Rotator& r);

	Rotator operator-=(const Rotator& r);

	bool IsNearlyZero(float tolerance = KINDA_SMALL_NUMBER) const;

	bool IsZero() const;

	bool Equals(const Rotator& r, float tolerance = KINDA_SMALL_NUMBER) const;

	Rotator Add(float deltaPitch, float deltaYaw, float deltaRoll);

	Rotator GetInverse() const;

	Rotator GridSnap(const Rotator& rotGrid) const;

	Vector GetVector() const;

	Quat Quaternion() const;

	Vector Euler() const;

	Vector RotateVector(const Vector& v) const;

	Vector UnrotateVector(const Vector& v) const;

	Rotator Clamp() const;

	Rotator GetNormalized() const;

	Rotator GetDenormalized() const;

	float GetComponentForAxis(Axis::Type Axis) const;

	void SetComponentForAxis(Axis::Type Axis, float Component);

	void Normalize();

	void GetWindingAndRemainder(Rotator& winding, Rotator& remainder) const;

	float GetManhattanDistance(const Rotator & rotator) const;

	Rotator GetEquivalentRotator() const;

	void SetClosestToMe(Rotator& makeClosest) const;

	std::string ToString() const;

	bool ContainsNaN() const;

public:

	static float ClampAxis(float angle);

	static float NormalizeAxis(float angle);

	static uint8 CompressAxisToByte(float angle);

	static float DecompressAxisFromByte(uint8 angle);

	static uint16 CompressAxisToShort(float angle);

	static float DecompressAxisFromShort(uint16 angle);

	static Rotator MakeFromEuler(const Vector& Euler);
};

FORCEINLINE Rotator::Rotator(float inF)
	: pitch(inF)
	, yaw(inF)
	, roll(inF)
{
	DiagnosticCheckNaN();
}

FORCEINLINE Rotator::Rotator(float inPitch, float inYaw, float inRoll)
	: pitch(inPitch)
	, yaw(inYaw)
	, roll(inRoll)
{
	DiagnosticCheckNaN();
}

FORCEINLINE Rotator Rotator::operator+(const Rotator& r) const
{
	return Rotator(pitch + r.pitch, yaw + r.yaw, roll + r.roll);
}

FORCEINLINE Rotator Rotator::operator-(const Rotator& r) const
{
	return Rotator(pitch - r.pitch, yaw - r.yaw, roll - r.roll);
}

FORCEINLINE Rotator Rotator::operator*(float scale) const
{
	return Rotator(pitch*scale, yaw*scale, roll*scale);
}

FORCEINLINE Rotator Rotator::operator*= (float scale)
{
	pitch = pitch * scale; 
	yaw   = yaw * scale;
	roll  = roll * scale;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE bool Rotator::operator==(const Rotator& r) const
{
	return pitch == r.pitch && yaw == r.yaw && roll == r.roll;
}

FORCEINLINE bool Rotator::operator!=(const Rotator& v) const
{
	return pitch != v.pitch || yaw != v.yaw || roll != v.roll;
}

FORCEINLINE Rotator Rotator::operator+=(const Rotator& r)
{
	pitch += r.pitch; 
	yaw   += r.yaw; 
	roll  += r.roll;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE Rotator Rotator::operator-=(const Rotator& r)
{
	pitch -= r.pitch; 
	yaw   -= r.yaw; 
	roll  -= r.roll;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE bool Rotator::IsNearlyZero(float tolerance) const
{
	return MMath::Abs(NormalizeAxis(pitch)) <= tolerance
		&& MMath::Abs(NormalizeAxis(yaw))   <= tolerance
		&& MMath::Abs(NormalizeAxis(roll))  <= tolerance;
}

FORCEINLINE bool Rotator::IsZero() const
{
	return (ClampAxis(pitch) == 0.f) && (ClampAxis(yaw) == 0.f) && (ClampAxis(roll) == 0.f);
}

FORCEINLINE bool Rotator::Equals(const Rotator& r, float tolerance) const
{
	return (MMath::Abs(NormalizeAxis(pitch - r.pitch)) <= tolerance)
		&& (MMath::Abs(NormalizeAxis(yaw   - r.yaw))   <= tolerance)
		&& (MMath::Abs(NormalizeAxis(roll  - r.roll))  <= tolerance);
}

FORCEINLINE Rotator Rotator::Add(float deltaPitch, float deltaYaw, float deltaRoll)
{
	yaw   += deltaYaw;
	pitch += deltaPitch;
	roll  += deltaRoll;
	DiagnosticCheckNaN();
	return *this;
}

FORCEINLINE Rotator Rotator::GridSnap(const Rotator& rotGrid) const
{
	return Rotator
	(
		MMath::GridSnap(pitch, rotGrid.pitch),
		MMath::GridSnap(yaw,   rotGrid.yaw),
		MMath::GridSnap(roll,  rotGrid.roll)
	);
}

FORCEINLINE Rotator Rotator::Clamp() const
{
	return Rotator(ClampAxis(pitch), ClampAxis(yaw), ClampAxis(roll));
}

FORCEINLINE float Rotator::ClampAxis(float angle)
{
	angle = MMath::Fmod(angle, 360.f);
	if (angle < 0.f)
	{
		angle += 360.f;
	}
	return angle;
}

FORCEINLINE float Rotator::NormalizeAxis(float angle)
{
	angle = ClampAxis(angle);
	if (angle > 180.f)
	{
		angle -= 360.f;
	}
	return angle;
}

FORCEINLINE uint8 Rotator::CompressAxisToByte(float angle)
{
	return MMath::RoundToInt(angle * 256.f / 360.f) & 0xFF;
}

FORCEINLINE float Rotator::DecompressAxisFromByte(uint8 angle)
{
	return (angle * 360.f / 256.f);
}

FORCEINLINE uint16 Rotator::CompressAxisToShort(float angle)
{
	return MMath::RoundToInt(angle * 65536.f / 360.f) & 0xFFFF;
}

FORCEINLINE float Rotator::DecompressAxisFromShort(uint16 angle)
{
	return (angle * 360.f / 65536.f);
}

FORCEINLINE Rotator Rotator::GetNormalized() const
{
	Rotator rot = *this;
	rot.Normalize();
	return rot;
}

FORCEINLINE Rotator Rotator::GetDenormalized() const
{
	Rotator rot = *this;
	rot.pitch = ClampAxis(rot.pitch);
	rot.yaw   = ClampAxis(rot.yaw);
	rot.roll  = ClampAxis(rot.roll);
	return rot;
}

FORCEINLINE void Rotator::Normalize()
{
	pitch = NormalizeAxis(pitch);
	yaw   = NormalizeAxis(yaw);
	roll  = NormalizeAxis(roll);
	DiagnosticCheckNaN();
}

FORCEINLINE float Rotator::GetComponentForAxis(Axis::Type axis) const
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

FORCEINLINE void Rotator::SetComponentForAxis(Axis::Type axis, float component)
{
	switch (axis)
	{
    case Axis::None:
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

FORCEINLINE Vector Rotator::GetVector() const
{
	float cp, sp, cy, sy;
	MMath::SinCos(&sp, &cp, MMath::DegreesToRadians(pitch));
	MMath::SinCos(&sy, &cy, MMath::DegreesToRadians(yaw));
	Vector v = Vector(cp * cy, cp * sy, sp);
	return v;
}

FORCEINLINE Vector Rotator::Euler() const
{
	return Vector(roll, pitch, yaw);
}

FORCEINLINE Rotator Rotator::MakeFromEuler(const Vector& euler)
{
	return Rotator(euler.x, euler.y, euler.z);
}

//FORCEINLINE Vector Rotator::RotateVector(const Vector& v) const
//{
//	return RotationMatrix(*this).TransformVector(v);
//}
//
//FORCEINLINE Vector Rotator::UnrotateVector(const Vector& v) const
//{
//	return RotationMatrix(*this).GetTransposed().TransformVector(v);
//}

FORCEINLINE void Rotator::GetWindingAndRemainder(Rotator& winding, Rotator& remainder) const
{
	remainder.yaw = NormalizeAxis(yaw);
	winding.yaw = yaw - remainder.yaw;

	remainder.pitch = NormalizeAxis(pitch);
	winding.pitch = pitch - remainder.pitch;

	remainder.roll = NormalizeAxis(roll);
	winding.roll = roll - remainder.roll;
}

FORCEINLINE std::string Rotator::ToString() const
{
	return StringUtils::Printf("P=%f Y=%f r=%f", pitch, yaw, roll);
}

FORCEINLINE bool Rotator::ContainsNaN() const
{
	return 
		!MMath::IsFinite(pitch) ||
		!MMath::IsFinite(yaw) ||
		!MMath::IsFinite(roll);
}

FORCEINLINE float Rotator::GetManhattanDistance(const Rotator & rotator) const
{
	return MMath::Abs<float>(yaw - rotator.yaw) + MMath::Abs<float>(pitch - rotator.pitch) + MMath::Abs<float>(roll - rotator.roll);
}

FORCEINLINE Rotator Rotator::GetEquivalentRotator() const
{
	return Rotator(180.0f - pitch, yaw + 180.0f, roll + 180.0f);
}

FORCEINLINE void Rotator::SetClosestToMe(Rotator& makeClosest) const
{
	Rotator otherChoice = makeClosest.GetEquivalentRotator();
	float firstDiff  = GetManhattanDistance(makeClosest);
	float secondDiff = GetManhattanDistance(otherChoice);
	if (secondDiff < firstDiff)
	{
		makeClosest = otherChoice;
	}
}

template<class U>
FORCEINLINE Rotator MMath::Lerp(const Rotator& a, const Rotator& b, const U& alpha)
{
	return a + (b - a).GetNormalized() * alpha;
}

template<class U>
FORCEINLINE Rotator MMath::LerpRange(const Rotator& a, const Rotator& b, const U& alpha)
{
	return (a * (1 - alpha) + b * alpha).GetNormalized();
}
