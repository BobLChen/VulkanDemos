#version 450

/*
* Shaders based on https://github.com/jdupuy/tessComp01
*/

struct PnPatch
{
	float b210;
	float b120;
	float b021;
	float b012;
	float b102;
	float b201;
	float b111;
	float n110;
	float n011;
	float n101;
};

layout (binding = 1) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

layout (binding = 0) uniform TessParam 
{
    vec4 level;
} tessParam;

layout (triangles, equal_spacing, ccw) in;

layout (location = 0) in vec3 inNormal[];
layout (location = 3) in vec2 inUV0[];
layout (location = 6) in PnPatch inPnPatch[];

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV0;
layout (location = 2) out vec3 outColor;

in gl_PerVertex 
{
    vec4 gl_Position;
} gl_in[];

void main()
{
	vec3 uvwSquared = gl_TessCoord * gl_TessCoord;
    vec3 uvwCubed   = uvwSquared * gl_TessCoord;

    // extract control points
    vec3 b210 = vec3(inPnPatch[0].b210, inPnPatch[1].b210, inPnPatch[2].b210);
    vec3 b120 = vec3(inPnPatch[0].b120, inPnPatch[1].b120, inPnPatch[2].b120);
    vec3 b021 = vec3(inPnPatch[0].b021, inPnPatch[1].b021, inPnPatch[2].b021);
    vec3 b012 = vec3(inPnPatch[0].b012, inPnPatch[1].b012, inPnPatch[2].b012);
    vec3 b102 = vec3(inPnPatch[0].b102, inPnPatch[1].b102, inPnPatch[2].b102);
    vec3 b201 = vec3(inPnPatch[0].b201, inPnPatch[1].b201, inPnPatch[2].b201);
    vec3 b111 = vec3(inPnPatch[0].b111, inPnPatch[1].b111, inPnPatch[2].b111);
	
    // extract control normals
    vec3 n110 = normalize(vec3(inPnPatch[0].n110, inPnPatch[1].n110, inPnPatch[2].n110));
    vec3 n011 = normalize(vec3(inPnPatch[0].n011, inPnPatch[1].n011, inPnPatch[2].n011));
    vec3 n101 = normalize(vec3(inPnPatch[0].n101, inPnPatch[1].n101, inPnPatch[2].n101));

	// compute texcoords
    outUV0 = gl_TessCoord[2] * inUV0[0] + gl_TessCoord[0] * inUV0[1] + gl_TessCoord[1] * inUV0[2];
	
    // normal
    // Barycentric normal
    vec3 barNormal = gl_TessCoord[2] * inNormal[0] + gl_TessCoord[0] * inNormal[1] + gl_TessCoord[1] * inNormal[2];
    vec3 pnNormal  = inNormal[0] * uvwSquared[2] + inNormal[1] * uvwSquared[0] + 
					 inNormal[2] * uvwSquared[1] + n110 * gl_TessCoord[2] * gl_TessCoord[0] + 
					 n011 * gl_TessCoord[0] * gl_TessCoord[1]+ n101 * gl_TessCoord[2] * gl_TessCoord[1];
	
	mat3 nrmMat33 = transpose(inverse(mat3(uboMVP.modelMatrix)));
	vec3 normal   = normalize(nrmMat33 * (tessParam.level.y * pnNormal + (1.0 - tessParam.level.y) * barNormal));

    outNormal = normal;
	
    // compute interpolated pos
    vec3 barPos = gl_TessCoord[2] * gl_in[0].gl_Position.xyz
                + gl_TessCoord[0] * gl_in[1].gl_Position.xyz
                + gl_TessCoord[1] * gl_in[2].gl_Position.xyz;

    // save some computations
    uvwSquared *= 3.0;

    // compute PN position
    vec3 pnPos  = gl_in[0].gl_Position.xyz * uvwCubed[2]
                + gl_in[1].gl_Position.xyz * uvwCubed[0]
                + gl_in[2].gl_Position.xyz * uvwCubed[1]
                + b210 * uvwSquared[2] * gl_TessCoord[0]
                + b120 * uvwSquared[0] * gl_TessCoord[2]
                + b201 * uvwSquared[2] * gl_TessCoord[1]
                + b021 * uvwSquared[0] * gl_TessCoord[1]
                + b102 * uvwSquared[1] * gl_TessCoord[2]
                + b012 * uvwSquared[1] * gl_TessCoord[0]
                + b111 * 6.0 * gl_TessCoord[0] * gl_TessCoord[1] * gl_TessCoord[2];

    // final position and normal
    vec3 finalPos = (1.0 - tessParam.level.y) * barPos + tessParam.level.y * pnPos;
 	gl_Position   = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * vec4(finalPos,1.0);
}