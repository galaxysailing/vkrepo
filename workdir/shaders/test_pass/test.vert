#version 460

layout(location = 0)in vec3 aPos;
layout(location = 1)in vec3 aNormal;
layout(location = 0)out vec3 vNormal;
layout(location = 1)out vec3 vCol;

layout(binding = 0) uniform UniformBufferObjects {
    mat4 proj_view_model;
    mat3 trans_inv_model;
    vec3 col;
} ubo;

void main(){
    // vNormal = normalize(ubo.trans_inv_model * aNormal);
    gl_Position = ubo.proj_view_model * vec4(aPos, 1.0);
    vNormal = aNormal;
    vCol = ubo.col;
}   