#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
//     outColor = vec4(fragColor, 1.0);
//	outColor = vec4(fragTexCoord, 0.0, 1.0);
	outColor = texture(texSampler, fragTexCoord);
	// outColor = vec4(0.5, 0.2, 0.3, 1.0);

//    outColor = vec4(pow(fragColor, vec3(1/2.2)), 1.0);
}