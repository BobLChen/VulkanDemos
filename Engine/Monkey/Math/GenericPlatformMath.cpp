#include "GenericPlatformMath.h"
#include "PlatformMath.h"
#include "Math.h"

static int32 G_SRandSeed;

void GenericPlatformMath::SRandInit(int32 seed)
{
	G_SRandSeed = seed;
}

int32 GenericPlatformMath::GetRandSeed()
{
	return G_SRandSeed;
}

float GenericPlatformMath::SRand()
{
	G_SRandSeed = (G_SRandSeed * 196314165) + 907633515;

	union 
	{ 
		float f; 
		int32 i;
	} result;

	union 
	{ 
		float f; 
		int32 i;
	} temp;

	const float randTemp = 1.0f;
	temp.f   = randTemp;
	result.i = (temp.i & 0xff800000) | (G_SRandSeed & 0x007fffff);

	return PlatformMath::Fractional(result.f);
}

float GenericPlatformMath::Atan2(float y, float x)
{
	const float absX = Abs(x);
	const float absY = Abs(y);
	const bool yAbsBigger = (absY > absX);
	float t0 = yAbsBigger ? absY : absX;
	float t1 = yAbsBigger ? absX : absY;

	if (t0 == 0.f) {
		return 0.f;
	}

	float t3 = t1 / t0;
	float t4 = t3 * t3;

	static const float c[7] = {
		+7.2128853633444123e-03f,
		-3.5059680836411644e-02f,
		+8.1675882859940430e-02f,
		-1.3374657325451267e-01f,
		+1.9856563505717162e-01f,
		-3.3324998579202170e-01f,
		+1.0f
	};

	t0 = c[0];
	t0 = t0 * t4 + c[1];
	t0 = t0 * t4 + c[2];
	t0 = t0 * t4 + c[3];
	t0 = t0 * t4 + c[4];
	t0 = t0 * t4 + c[5];
	t0 = t0 * t4 + c[6];
	t3 = t0 * t3;

	t3 = yAbsBigger ? (0.5f * PI) - t3 : t3;
	t3 = (x < 0.0f) ? PI - t3 : t3;
	t3 = (y < 0.0f) ? -t3 : t3;

	return t3;
}
