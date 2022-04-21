#version 440

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;

layout(location = 0) out vec2 v_texcoord;
layout(location = 1) out vec3 cameraDir;
layout(location = 2) out vec3 cameraPos;
layout(location = 3) out vec2 xAxisRange;
layout(location = 4) out vec2 yAxisRange;
layout(location = 5) out mat4 invModelView;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 invModelView;
    vec3 posCamera;
    vec2 xAxisRange;
    vec2 yAxisRange;
    //float aspect;
    //float cotHalfFov;
} ubo;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
    cameraPos = (ubo.invModelView * vec4(ubo.posCamera, 1.0)).xyz;
    cameraDir = mat3(ubo.invModelView) * normalize(vec3(position, 0.0) - vec3(ubo.posCamera));
    invModelView = ubo.invModelView;
    v_texcoord = texcoord;

    xAxisRange = ubo.xAxisRange;
    yAxisRange = ubo.yAxisRange;

    gl_Position = ubo.proj * vec4(position, 0.0, 1.0);
}
