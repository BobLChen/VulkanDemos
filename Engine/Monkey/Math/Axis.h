#pragma once

namespace Axis
{
	enum Type
	{
		Axis_None,
		X,
		Y,
		Z,
	};
}

namespace AxisList
{
	enum Type
	{
		Axis_None = 0,
		X = 1,
		Y = 2,
		Z = 4,

		Screen = 8,
		XY = X | Y,
		XZ = X | Z,
		YZ = Y | Z,
		XYZ = X | Y | Z,
		All = XYZ | Screen,

		ZRotation = YZ,
		Rotate2D = Screen,
	};
}
