#version 430 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;

out VertexData {
	vec4 color;
	vec2 uv;
} vertexOut;

void main() {
	gl_Position = vec4(vertexPosition_modelspace, 1.0);
	vertexOut.color = color;
	vertexOut.uv = uv;
}
