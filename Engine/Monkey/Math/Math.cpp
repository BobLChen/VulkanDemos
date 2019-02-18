#include "Math.h"
#include "Common/Common.h"
#include "Utils/StringUtils.h"

#include "Math/IntPoint.h"
#include "Math/Vector2.h"
#include "Math/Vector.h"
#include "Math/IntVector.h"

const IntPoint IntPoint::ZeroValue(0,  0);
const IntPoint IntPoint::NoneValue(-1, -1);
const IntVector IntVector::ZeroValue(0, 0, 0);
const IntVector IntVector::NoneValue(-1, -1, -1);

const Vector2 Vector2::ZeroVector(0.0f, 0.0f);
const Vector2 Vector2::UnitVector(1.0f, 1.0f);

const Vector Vector::ZeroVector(0.0f, 0.0f, 0.0f);
const Vector Vector::OneVector(1.0f, 1.0f, 1.0f);
const Vector Vector::UpVector(0.0f, 1.0f, 0.0f);
const Vector Vector::ForwardVector(0.0f, 0.0f, 1.0f);
const Vector Vector::RightVector(1.0f, 0.0f, 0.0f);

const uint32 MMath::BitFlag[32] =
{
	(1U << 0),	(1U << 1),	(1U << 2),	(1U << 3),
	(1U << 4),	(1U << 5),	(1U << 6),	(1U << 7),
	(1U << 8),	(1U << 9),	(1U << 10),	(1U << 11),
	(1U << 12),	(1U << 13),	(1U << 14),	(1U << 15),
	(1U << 16),	(1U << 17),	(1U << 18),	(1U << 19),
	(1U << 20),	(1U << 21),	(1U << 22),	(1U << 23),
	(1U << 24),	(1U << 25),	(1U << 26),	(1U << 27),
	(1U << 28),	(1U << 29),	(1U << 30),	(1U << 31),
};

float MMath::FInterpConstantTo(float current, float target, float deltaTime, float interpSpeed)
{
	const float dist = target - current;
	if (MMath::Square(dist) < SMALL_NUMBER)
	{
		return target;
	}
	const float step = interpSpeed * deltaTime;
	return current + MMath::Clamp<float>(dist, -step, step);
}

float MMath::FInterpTo(float current, float target, float deltaTime, float interpSpeed)
{
	if (interpSpeed <= 0.f)
	{
		return target;
	}

	const float dist = target - current;
	if (MMath::Square(dist) < SMALL_NUMBER)
	{
		return target;
	}

	const float deltaMove = dist * MMath::Clamp<float>(deltaTime * interpSpeed, 0.f, 1.f);
	return current + deltaMove;
}

void MMath::WindRelativeAnglesDegrees(float inAngle0, float& inOutAngle1)
{
	const float diff = inAngle0 - inOutAngle1;
	const float absDiff = Abs(diff);
	if (absDiff > 180.0f)
	{
		inOutAngle1 += 360.0f * Sign(diff) * FloorToFloat((absDiff / 360.0f) + 0.5f);
	}
}

float MMath::TruncateToHalfIfClose(float value)
{
	float valueToFudgeIntegralPart = 0.0f;
	float valueToFudgeFractionalPart = MMath::Modf(value, &valueToFudgeIntegralPart);
	if (value < 0.0f)
	{
		return valueToFudgeIntegralPart + ((MMath::IsNearlyEqual(valueToFudgeFractionalPart, -0.5f)) ? -0.5f : valueToFudgeFractionalPart);
	}
	else
	{
		return valueToFudgeIntegralPart + ((MMath::IsNearlyEqual(valueToFudgeFractionalPart, 0.5f)) ? 0.5f : valueToFudgeFractionalPart);
	}
}

double MMath::TruncateToHalfIfClose(double value)
{
	double valueToFudgeIntegralPart = 0.0;
	double valueToFudgeFractionalPart = MMath::Modf(value, &valueToFudgeIntegralPart);
	if (value < 0.0)
	{
		return valueToFudgeIntegralPart + ((MMath::IsNearlyEqual(valueToFudgeFractionalPart, -0.5)) ? -0.5 : valueToFudgeFractionalPart);
	}
	else
	{
		return valueToFudgeIntegralPart + ((MMath::IsNearlyEqual(valueToFudgeFractionalPart, 0.5)) ? 0.5 : valueToFudgeFractionalPart);
	}
}

float MMath::RoundHalfToEven(float value)
{
	value = MMath::TruncateToHalfIfClose(value);

	const bool bIsNegative = value < 0.0f;
	const bool bValueIsEven = static_cast<uint32>(FloorToFloat(((bIsNegative) ? -value : value))) % 2 == 0;
	if (bValueIsEven)
	{
		return (bIsNegative) ? FloorToFloat(value + 0.5f) : CeilToFloat(value - 0.5f);
	}
	else
	{
		return (bIsNegative) ? CeilToFloat(value - 0.5f) : FloorToFloat(value + 0.5f);
	}
}

double MMath::RoundHalfToEven(double value)
{
	value = MMath::TruncateToHalfIfClose(value);

	const bool bIsNegative = value < 0.0f;
	const bool bValueIsEven = static_cast<uint32>(FloorToFloat(((bIsNegative) ? -value : value))) % 2 == 0;
	if (bValueIsEven)
	{
		return (bIsNegative) ? FloorToFloat(value + 0.5f) : CeilToFloat(value - 0.5f);
	}
	else
	{
		return (bIsNegative) ? CeilToFloat(value - 0.5f) : FloorToFloat(value + 0.5f);
	}
}

float MMath::RoundHalfFromZero(float value)
{
	value = MMath::TruncateToHalfIfClose(value);
	return (value < 0.0f) ? CeilToFloat(value - 0.5f) : FloorToFloat(value + 0.5f);
}

double MMath::RoundHalfFromZero(double value)
{
	value = MMath::TruncateToHalfIfClose(value);
	return (value < 0.0) ? CeilToDouble(value - 0.5) : FloorToDouble(value + 0.5);
}

float MMath::RoundHalfToZero(float value)
{
	value = MMath::TruncateToHalfIfClose(value);
	return (value < 0.0f) ? FloorToFloat(value + 0.5f) : CeilToFloat(value - 0.5f);
}

double MMath::RoundHalfToZero(double value)
{
	value = MMath::TruncateToHalfIfClose(value);
	return (value < 0.0) ? FloorToDouble(value + 0.5) : CeilToDouble(value - 0.5);
}

float MMath::PerlinNoise1D(const float value)
{
	const int32 b = 256;
	static int32 p[b + b + 2];
	static float g[b + b + 2];
	const int32 bm = 255;

	static bool bIsFirstCall = true;
	if (bIsFirstCall)
	{
		bIsFirstCall = false;

		int32 i;
		for (i = 0; i < b; i++)
		{
			p[i] = i;

			const int32 Random1 = MMath::RandRange(0, 0x3fffffff);

			g[i] = (float)((Random1 % (b + b)) - b) / b;
		}

		while (--i)
		{
			const int32 k = p[i];

			const int32 Random = MMath::RandRange(0, 0x3fffffff);

			const int32 j = Random % b;
			p[i] = p[j];
			p[j] = k;
		}

		for (i = 0; i < b + 2; i++)
		{
			p[b + i] = p[i];
			g[b + i] = g[i];
		}
	}

	const int32 n = 4096;
	const float t = value + n;
	const int32 bx0 = ((int32)t) & bm;
	const int32 bx1 = (bx0 + 1) & bm;
	const float rx0 = t - (int32)t;
	const float rx1 = rx0 - 1.;
	const float sx = (rx0 * rx0 * (3. - 2. * rx0));
	const float u = rx0 * g[p[bx0]];
	const float v = rx1 * g[p[bx1]];

	return 2.0f * (u + sx * (v - u));
}