#ifndef MATERIAL
#define MATERIAL

float GlossyPdf(vec3 incomingDirection, vec3 outgoingDirection, RayPayloadInfo payload)
{
	vec3 N = payload.worldNormal;
	vec3 V = normalize(-incomingDirection);
	vec3 L = normalize(outgoingDirection);

	float diffuseRatio  = 0.5 * (1.0 - payload.metallic);
	float specularRatio = 1.0 - diffuseRatio;

	vec3 H = normalize(L + V);
	float cosTheta = abs(dot(H, N));

	float a = max(0.001, payload.roughness);
	float pdfGTR2 = GTR2(cosTheta, a) * cosTheta;

	float pdfSpec = pdfGTR2 / (4.0 * abs(dot(L, H)));
	float pdfDiff = abs(dot(L, N)) * (1.0 / PI);

	return diffuseRatio * pdfDiff + specularRatio * pdfSpec;
}

vec3 GlossySample(vec3 incomingDirection, RayPayloadInfo payload, vec3 randSample)
{
	vec3 N = payload.worldNormal;
	vec3 V = normalize(-incomingDirection);

	float probability  = randSample.x;
	float diffuseRatio = 0.5 * (1.0 - payload.metallic);

	float r1 = randSample.y;
	float r2 = randSample.z;

	vec3 UpVector = abs(N.z) < 0.999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
	vec3 TangentX = normalize(cross(UpVector, N));
	vec3 TangentY = cross(N, TangentX);

	vec3 dir;

	if (probability < diffuseRatio)
	{
		dir = CosineSampleHemisphere(r1, r2);
		dir = TangentX * dir.x + TangentY * dir.y + N * dir.z;
	}
	else
	{
		float a = max(0.001, payload.roughness);
		float phi = r1 * 2.0 * PI;
		float cosTheta = sqrt((1.0 - r2) / (1.0 + (a * a - 1.0) * r2));
		float sinTheta = clamp(sqrt(1.0 - (cosTheta * cosTheta)), 0.0, 1.0);
		float sinPhi = sin(phi);
		float cosPhi = cos(phi);

		vec3 halfVec = vec3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
		halfVec = TangentX * halfVec.x + TangentY * halfVec.y + N * halfVec.z;

		dir = 2.0 * dot(V, halfVec) * halfVec - V;
	}

	return dir;
}

vec3 GlossyEval(vec3 incomingDirection, vec3 outgoingDirection, RayPayloadInfo payload)
{
	vec3 N = payload.worldNormal;
	vec3 V = normalize(-incomingDirection);
	vec3 L = normalize(outgoingDirection);

	float NDotL = dot(N, L);
	float NDotV = dot(N, V);

	if (NDotL <= 0.0 || NDotV <= 0.0) {
		return vec3(0.0);
	}

	vec3 H = normalize(L + V);
	float NDotH = dot(N, H);
	float LDotH = dot(L, H);
	
	// specular	
	float specular = 0.5;
	vec3 specularCol = mix(vec3(1.0) * 0.08 * specular, payload.baseColor.xyz, payload.metallic);
	float a  = max(0.001, payload.roughness);
	float Ds = GTR2(NDotH, a);
	float FH = SchlickFresnel(LDotH);
	vec3 Fs  = mix(specularCol, vec3(1.0), FH);

	float roughg = (payload.roughness * 0.5 + 0.5);
	roughg = roughg * roughg;

	float Gs = SmithG_GGX(NDotL, roughg) * SmithG_GGX(NDotV, roughg);

	return (payload.baseColor.xyz / PI) * (1.0 - payload.metallic) + Gs * Fs*Ds;
}

void EvalMaterial(vec3 incomingDirection, vec3 outgoingDirection, RayPayloadInfo payload, out vec3 outThroughput, out float outPdf)
{
	outThroughput = GlossyEval(incomingDirection, outgoingDirection, payload);
	outPdf = GlossyPdf(incomingDirection, outgoingDirection, payload);
}

void SampleMaterial(vec3 rayDirection, RayPayloadInfo payload, vec3 randSample, out vec3 outDirection, out vec3 outThroughput, out float outPdf, out float outPositionBiasSign)
{
	outPositionBiasSign = 1;

	outDirection  = GlossySample(rayDirection, payload, randSample.xyz);
	outPdf        = GlossyPdf(rayDirection, outDirection, payload);
	
	outThroughput = GlossyEval(rayDirection, outDirection, payload) * abs(dot(payload.worldNormal, outDirection)) / outPdf;
}

#endif