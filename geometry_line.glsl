#version 430 core

layout(points, invocations = 10) in;
layout(line_strip, max_vertices = 65) out;

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

int vertex_num = 64;
float PI = 3.14159265358979323846;
float step_length = 0.002;
float radiance_radius = 0.1;

void main() {
//	if(gl_in[0].gl_Position.z < radiance_radius) {	// small circles
	if(true) { // all circles
		vec3 color_rgb;
		if     (vertexIn[0].color.r == 1) color_rgb = vec3(168.0 / 255, 26.0 / 255, 123.0 / 255);
		else if(vertexIn[0].color.g == 1) color_rgb = vec3(26.0 / 255, 168.0 / 255, 123.0 / 255);
		else                              color_rgb = vec3(26.0 / 255, 123.0 / 255, 168.0 / 255);
	
		for(int i = 0; i < vertex_num + 1; i++) {
			gl_Position = vec4(gl_in[0].gl_Position.x + cos(PI * 2 / vertex_num * i) * (gl_in[0].gl_Position.z + step_length * (gl_InvocationID - 5)) * window_height / window_width, gl_in[0].gl_Position.y + sin(PI * 2 / vertex_num * i) * (gl_in[0].gl_Position.z + step_length * (gl_InvocationID - 5)), 0.0, 1.0);
//			vertexOut.color = vec4(color_rgb, 0.75 / radiance_radius * (gl_in[0].gl_Position.z < radiance_radius ? (radiance_radius - gl_in[0].gl_Position.z) * 0.75 + 0.025 : 0.05) - 0.15 * abs((gl_InvocationID - 5))); // small cirles
//			vertexOut.color = vec4(color_rgb, 0.75 / radiance_radius * (radiance_radius - gl_in[0].gl_Position.z) - 0.15 * abs((gl_InvocationID - 5))); // small cirles
			vertexOut.color = vec4(color_rgb, 0.75 / radiance_radius * 0.05 - 0.15 * abs((gl_InvocationID - 5))); // all circles
			if(gl_in[0].gl_Position.z < 0.01) vertexOut.color.a += 0.4; // enhance alpha of tiny circles
			EmitVertex();
		}
		EndPrimitive();
	}
}
