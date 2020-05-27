#ifndef SKY_LIGHT
#define SKY_LIGHT

void SkyLightGenerateLightRay(RayDesc ray, int lightId, vec3 lightUV, out RayDesc outLightRay)
{
	outLightRay.origin = ray.origin;
	outLightRay.direction = lightUV;
	outLightRay.tMin = TMIN;
	outLightRay.tMax = TMAX;
}

void SkyLightPdfLight(RayDesc ray, int lightId, vec3 lightUV, out float outPdf)
{
	outPdf = 1.0 / (2.0 * PI);
}

void SkyLightEvalLight(int lightId, vec3 lightUV, RayDesc ray, out vec3 outRadiance)
{
    outRadiance = lights.datas[lightId].color.rgb;
}

void SkyLightSampleLight(RayDesc ray, RayPayloadInfo payload, vec4 randSample, int lightId, out vec3 outLightUV, out float outPdf)
{
    vec4 directionTangent = UniformSampleHemisphere(randSample.yz);
	outLightUV = TangentToWorld(directionTangent.xyz, payload.worldNormal);
	outPdf = directionTangent.w;
}

#endif