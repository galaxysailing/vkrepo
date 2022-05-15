#version 460

layout(location = 0) in vec3 vNormal;
layout(location = 0) out vec4 FragColor;

void main(){
    vec3 normal = vNormal * 0.5 + vec3(0.5, 0.5, 0.5);
    // normal = pow(normal, vec3(2.2,2.2,2.2));
    FragColor = vec4(normal, 1.0);
}