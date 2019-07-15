#version 450

layout (set = 0, binding = 1) uniform UBOParam 
{
	float omega;
	float k;
	float cutoff;
	float padding;
} params;

layout (location = 0) in vec2 inUV0;

layout (location = 0) out vec4 outFragColor;

float PreCompute(float omega, float k, float CutOff, float u, float v)
{
	float left      = 0.36 * cos(omega * v) + 0.1;
	float inArea1   = max(sign(left - u), 0.0);
	float inArea3   = max(sign(k * (u - CutOff) - v), 0.0);
	float inArea2   = 1.0 - min(inArea1 + inArea3, 1.0);
	float value3    = 1;//0.5*u+0.38;
	float right     = v / k + CutOff;
	float amplitude = 0.5;//0.5 * (0.5 * right + 0.38);
	float omeg      = 3.1415926 / (right - left);
	float value2    = amplitude + amplitude * sin(omeg * (u - left - 0.5 * (right - left)));
	return (inArea2 * value2 + inArea3 * value3);
}

void main() 
{
	float bias = PreCompute(params.omega, params.k, params.cutoff, inUV0.x, inUV0.y);
	outFragColor.rgba = vec4(bias, bias, bias, 1.0);
}