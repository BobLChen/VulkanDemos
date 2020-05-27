#ifndef DIRECTIONAL_LIGHT
#define DIRECTIONAL_LIGHT

void DirectionalLightGenerateLightRay(RayDesc ray, int lightId, vec3 lightUV, out RayDesc outLightRay)
{
	outLightRay.origin    = ray.origin;
	outLightRay.tMin      = TMIN;
	outLightRay.direction = normalize(lightUV);
	outLightRay.tMax      = TMAX;
}

void DirectionalLightPdfLight(RayDesc ray, int lightId, vec3 lightUV, out float outPdf)
{
	outPdf = 0.0;
}

void DirectionalLightEvalLight(int lightId, vec3 lightUV, RayDesc ray, out vec3 outRadiance)
{
	outRadiance = GetColor(lightId);
}

void DirectionalLightSampleLight(RayDesc ray, RayPayloadInfo payLoad, vec3 randSample, int lightId, out vec3 outLightUV, out float outPdf)
{
	outPdf = 1.0;
	outLightUV = GetNormal(lightId);
}

#endif