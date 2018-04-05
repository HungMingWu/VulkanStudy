#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout(binding = 0) uniform TriangleUBO {
	mat4 mvp;
} ubo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texcoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexcoord;

void main() {
	gl_Position = ubo.mvp * vec4(position, 1.0);
	fragColor = color;
	fragTexcoord = texcoord;
}
