#ifndef CAMERA_COMMON
#define CAMERA_COMMON

void SampleEmitter(CameraInfo camera, uvec2 launchIndex, inout RandomSequence randSequence, inout uint sampleIndex, uint sourceEmitterId, uint sensorId, out RayDesc outRay, out uvec2 outPixel, out float outPdf)
{
    vec2 randSample = RandomSequenceGenerateSample2D(randSequence, sampleIndex);
    vec2 viewportUV = (vec2(launchIndex.x, launchIndex.y) + randSample) * camera.viewSize.zw;
    
    vec3 origin    = camera.pos.xyz;
    vec3 direction = vec3(viewportUV.x * 2.0 - 1.0, -(viewportUV.y * 2.0 - 1.0), 1.0); // flip Y

    // clip to view space
    direction   = (camera.invProj * vec4(direction, 1.0)).xyz;
    direction.x = direction.x * direction.z;
    direction.y = direction.y * direction.z;

    // view space to world space
    direction.xyz = (camera.invView * vec4(direction, 0.0)).xyz;
    direction.xyz = normalize(direction.xyz);

    outRay.origin    = origin;
    outRay.direction = direction;
    outRay.tMin      = TMIN;
    outRay.tMax      = TMAX;

    outPixel = launchIndex.xy;
    outPdf   = 1.0;
}

#endif