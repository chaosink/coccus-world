#version 430 core

layout(points, invocations = 2) in;
layout(triangle_strip, max_vertices = 96) out;

in VertexData {
	vec4 color;
	vec2 uv;
} vertexIn[];
out VertexData {
	vec4 color;
	vec2 uv;
} vertexOut;

uniform int window_width;
uniform int window_height;
uniform float iGlobalTime;

int vertex_num = 64;
float PI = 3.14159265358979323846;
float border = 0.0;

void main() {
	for(int i = gl_InvocationID * vertex_num / 2; i < (gl_InvocationID + 1)  * vertex_num / 2; i++) {
		gl_Position = vec4(gl_in[0].gl_Position.x + cos(PI * 2 / vertex_num * i) * (gl_in[0].gl_Position.z + border) * window_height / window_width, gl_in[0].gl_Position.y + sin(PI * 2 / vertex_num * i) * (gl_in[0].gl_Position.z + border), 0.0, 1.0);
		vertexOut.color = vec4(vertexIn[0].color.rgb, 1.0);
		vertexOut.uv = vec2(cos(PI * 2 / vertex_num * i), sin(PI * 2 / vertex_num * i)) / 2.0 + 0.5;
		EmitVertex();
		gl_Position = vec4(gl_in[0].gl_Position.x + cos(PI * 2 / vertex_num * (i + 1)) * (gl_in[0].gl_Position.z + border) * window_height / window_width, gl_in[0].gl_Position.y + sin(PI * 2 / vertex_num * (i + 1)) * (gl_in[0].gl_Position.z + border), 0.0, 1.0);
		vertexOut.color = vec4(vertexIn[0].color.rgb, 1.0);
		vertexOut.uv = vec2(cos(PI * 2 / vertex_num * (i + 1)), sin(PI * 2 / vertex_num * (i + 1))) / 2.0 + 0.5;
		EmitVertex();
		gl_Position = vec4(gl_in[0].gl_Position.x, gl_in[0].gl_Position.y, 0.0, 1.0);
		vertexOut.color = vec4(vertexIn[0].color.rgb, 0.6);
		vertexOut.uv = vec2(0.5, 0.5);
		EmitVertex();
		EndPrimitive();
	}
}
// (sin(iGlobalTime + vertexIn[0].color.a * PI * 2) * 0.1 + 0.1 + 0.8)
