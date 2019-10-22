#version 450

layout (binding = 2) uniform samplerCube environmentMap;

layout (set = 0, location = 0) in vec3 inUVW;

layout (set = 0, location = 0) out vec4 outColor;

#define PI 3.14159265359

void main() 
{
    vec3 irradiance = vec3(0, 0, 0);

    // tangent space
    vec3 N = normalize(inUVW);
    vec3 up = vec3(0, 1, 0);
    vec3 right = normalize(cross(up, N));
    up = cross(N, right);

    float sampleDelta = 0.025;
    float sampleCount = 0;

    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// tangent spcae to world space
			vec3 sampleDir = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			sampleDir = sampleDir.x * right + sampleDir.y * up + sampleDir.z * N;
			// irradiance
			irradiance  += texture(environmentMap, sampleDir).xyz * cos(theta) * sin(theta);
			sampleCount += 1;
		}
	}
    
    irradiance = (irradiance / sampleCount) / PI;
	outColor   = vec4(irradiance, 1.0);
}