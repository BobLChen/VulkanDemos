#version 450

layout (set = 0, binding = 1) uniform sampler2D samplerColorMap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV0;

layout (location = 0) out vec4 outFragColor;

layout (constant_id = 0) const int   LIGHTING_MODEL = 0;
layout (constant_id = 1) const float INTENSITY      = 1.0f;

void main() 
{
    switch (LIGHTING_MODEL) {
		case 0: // Phong			
		{
			vec3 ambient = vec3(0.25);
			vec3 N = normalize(inNormal);
			vec3 L = normalize(vec3(0, 1, -1));
			vec3 V = normalize(vec3(0, 0, -1));
			vec3 R = reflect(-L, N);
			vec3 diffuse  = max(dot(N, L), 0.0) * texture(samplerColorMap, inUV0).rgb;
			vec3 specular = pow(max(dot(R, V), 0.0), 32.0) * vec3(0.75);
			outFragColor  = vec4(ambient + diffuse * INTENSITY + specular, 1.0);		
			break;
		}
		case 1: // Rim
		{
            vec3 ambient = vec3(0.25);
			vec3 N = normalize(inNormal);
			vec3 V = normalize(vec3(0, 0, -1));
            vec3 L = normalize(vec3(0, 1, -1));
			vec3 diffuse  = max(dot(N, L), 0.0) * texture(samplerColorMap, inUV0).rgb;
            vec3 rimColor = max(dot(N, V), 0.0) * vec3(1, 0, 0);
            outFragColor  = vec4(ambient + diffuse + rimColor * INTENSITY, 1.0);
			break;
		}
		case 2: // Textured
		{
			vec4 color   = texture(samplerColorMap, inUV0).bgra;
			vec3 ambient = color.rgb * vec3(0.25);
			vec3 N = normalize(inNormal);
			vec3 V = normalize(vec3(0, 0, -1));
            vec3 L = normalize(vec3(0, 1, -1));
			vec3 R = reflect(-L, N);
			vec3 diffuse   = max(dot(N, L), 0.0) * color.rgb;
			float specular = pow(max(dot(R, V), 0.0), 32.0) * color.a;
			outFragColor   = vec4(ambient + diffuse * INTENSITY + vec3(specular), 1.0);		
			break;
		}
    }
}