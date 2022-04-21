#version 440
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 v_texcoord;
layout(location = 1) in vec3 cameraDir;
layout(location = 2) in vec3 cameraPos;
layout(location = 3) in vec2 xAxisRange;
layout(location = 4) in vec2 yAxisRange;
layout(location = 5) in mat4 invModelView;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D tex;
layout(binding = 2) uniform sampler3D tex3D;

layout(push_constant) uniform pushConstant {
    vec2 xAxisRange;
    vec2 yAxisRange;
}constant;

#define PI 3.14159265

vec3 mapToVolumeTex(vec3 pos)
{
     vec3 volumeScale = normalize(vec3(201,51,219))/2;
     return (pos + volumeScale )/ 2 /volumeScale;
}

bool intersectBox(vec3 ori, vec3 dir, vec3 boxMin, vec3 boxMax, out float t0, out float t1)
{
    vec3 invDir = 1.0 / dir;
    vec3 tBot = invDir * (boxMin.xyz - ori);
    vec3 tTop = invDir * (boxMax.xyz - ori);

    vec3 tMin = min(tTop, tBot);
    vec3 tMax = max(tTop, tBot);

    vec2 temp = max(tMin.xx, tMin.yz);
    float tMinMax = max(temp.x, temp.y);
    temp = min(tMax.xx, tMax.yz);
    float tMaxMin = min(temp.x, temp.y);

    bool hit;
    if((tMinMax > tMaxMin))
        hit = false;
    else
        hit = true;

    t0 = tMinMax;
    t1 = tMaxMin;

    return hit;
}

float opacityCorrecting(float opacity)
{
    return 1 - pow((1-opacity),30/40.0);
}


vec4 raycasting(vec3 ori, vec3 dir, float t0, float t1)
{
    vec4  frag_c=vec4(0, 0, 0, 0);
    vec4  TF;
    vec3  curPos;
    float scalarVal;
    float tempGradient;
    float k;
    float stepDistance = 0.002;//(t1 - t0) / 100.0f;

    float t = t0;
    while (t<t1)
    {
        curPos = ori+dir*t;
        vec3 texPos = mapToVolumeTex(curPos);

        tempGradient = (texture(tex3D, texPos).g - yAxisRange.x) / (yAxisRange.y - yAxisRange.x);
        scalarVal = (texture(tex3D, texPos).r - xAxisRange.x) / (xAxisRange.y - xAxisRange.x);

        //tempGradient = texture(tex3D, texPos).g;
        //scalarVal = texture(tex3D, texPos).r;
        k = texture(tex3D, texPos).g / texture(tex3D, texPos).r;
        //background
        if(texture(tex, vec2(scalarVal, 1.0f-tempGradient)).a < 0.2)
            TF = vec4(0.0f, 0.0, 0.0, 0.0f);
        else //object region
        {
            /*
            if(texture(tex3D, texPos).g > 0.0f)
                TF.rgb = vec3(0.8f, 0.0f, 0.0f);
            else if(texture(tex3D, texPos).g < 0.000001f && texture(tex3D, texPos).g > -0.000001f) //when temporal gradient = 0
                TF.rgb = vec3(0.0f, 0.0f, 0.8f);
            else
                TF.rgb = vec3(0.0f, 0.8f, 0.0f);
            TF.a = 0.05;
            */
            float l = atan(k);
            if(l >= 0)
                TF.rgba = vec4(l/(PI/4), (1.0f-l/(PI/4)), 0.0f, l/(PI/4));
            else
                TF.rgba = vec4(0.0f, (1.0f+l/(PI/2)), -l/(PI/2), -l/(PI/2));
        }

        t += stepDistance;  // in order to do the follow condition check, we must step forward in advance

        frag_c.rgb += TF.rgb * TF.a * (1 - frag_c.a);
        frag_c.a += TF.a * (1 - frag_c.a);
        if (frag_c.a>=0.999)
            break;
    }
    return frag_c;
}



void main()
{
    vec3 volumeScale = normalize(vec3(201,51,219))/2;
    vec3 boxMin = -volumeScale;
    vec3 boxMax = volumeScale;

    // discard if ray-box intersection test fails
    vec3 dir = normalize(cameraDir);
    float t0, t1;
    bool hit = intersectBox(cameraPos, dir, boxMin, boxMax, t0, t1);

    if(!hit)
        discard;

    // discard if the volume is behind the camera
    t0 = max(t0, 0.0);
    if(t1 <= t0)
        discard;

    outColor = raycasting(cameraPos, dir, t0, t1);
}
