#version 460

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec3 vCol;
layout(location = 0) out vec4 FragColor;

void main(){
    vec3 normal = vNormal * 0.5 + vec3(0.5, 0.5, 0.5);

// #define QUEUE_TEST
#ifdef QUEUE_TEST
    for(int i = 0; i < 400000; ++i){
        normal = vNormal * 0.5 + vec3(0.5, 0.5, 0.5) + normal + vCol;
    }
#endif
    normal = normalize(normal);
    FragColor = vec4(normal, 1.0);
}