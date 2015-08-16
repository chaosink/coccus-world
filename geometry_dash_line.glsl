#version 430 core

layout(lines, invocations = 9) in;
layout(line_strip, max_vertices = 120) out;

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

int vertex_num = 40;
float PI = 3.14159265358979323846;
float step_length = 0.0125;
float offset_speed = 0.05;
float arrow_angle = PI * 5 / 6;

void main() {
	vec4 direction = normalize(gl_in[1].gl_Position - gl_in[0].gl_Position);
	float step_length_d = length(direction * vec4(step_length, step_length * window_width / window_height, 1.0, 1.0));
	vec4 step_vector = direction * step_length_d;
	float len = length(gl_in[1].gl_Position - gl_in[0].gl_Position);
	int segment_num = int(len / step_length_d);

	float offset = fract(iGlobalTime * offset_speed / (step_length_d * 2)) * step_length_d * 2;
	vec4 offset_vector = direction * offset;

	vec4 direction_d, direction_scale, direction_d_scale;
	float beta, cos_beta_scale, sin_beta_scale;
	vec4 step_vector_d;
	vec3 cross_vector;

	beta = arrow_angle;
	direction_d = direction * mat4(
		 cos(beta),-sin(beta), 0, 0,
		 sin(beta), cos(beta), 0, 0,
		 0, 0, 1, 0,
		 0, 0, 0, 1);
	direction_scale = direction * vec4(1.0, 1.0 * window_width / window_height, 1.0, 1.0);
	direction_d_scale = direction_d * vec4(1.0, 1.0 * window_width / window_height, 1.0, 1.0);
	cos_beta_scale = dot(normalize(direction_scale), normalize(direction_d_scale));
	cross_vector = cross(normalize(direction_scale).xyz, normalize(direction_d_scale).xyz);
	sin_beta_scale = length(cross_vector) * (cross_vector.z > 0 ? 1 : -1);
	direction_d = direction * mat4(
		 cos_beta_scale,-sin_beta_scale, 0, 0,
		 sin_beta_scale, cos_beta_scale, 0, 0,
		 0, 0, 1, 0,
		 0, 0, 0, 1);
	vec4 step_vector_d1 = direction_d * length(direction_d * vec4(step_length / 2, step_length / 2 * window_width / window_height, 1.0, 1.0));

	beta = PI * 2 - arrow_angle;
	direction_d = direction * mat4(
		 cos(beta),-sin(beta), 0, 0,
		 sin(beta), cos(beta), 0, 0,
		 0, 0, 1, 0,
		 0, 0, 0, 1);
	direction_scale = direction * vec4(1.0, 1.0 * window_width / window_height, 1.0, 1.0);
	direction_d_scale = direction_d * vec4(1.0, 1.0 * window_width / window_height, 1.0, 1.0);
	cos_beta_scale = dot(normalize(direction_scale), normalize(direction_d_scale));
	cross_vector = cross(normalize(direction_scale).xyz, normalize(direction_d_scale).xyz);
	sin_beta_scale = length(cross_vector) * (cross_vector.z > 0 ? 1 : -1);
	direction_d = direction * mat4(
		 cos_beta_scale,-sin_beta_scale, 0, 0,
		 sin_beta_scale, cos_beta_scale, 0, 0,
		 0, 0, 1, 0,
		 0, 0, 0, 1);
	vec4 step_vector_d2 = direction_d * length(direction_d * vec4(step_length / 2, step_length / 2 * window_width / window_height, 1.0, 1.0));

	for(int i = gl_InvocationID * vertex_num; i < min(gl_InvocationID * vertex_num + vertex_num, segment_num); i += 2) 
		if(length(step_vector * (i + 1) + offset_vector) < len) {
			gl_Position = gl_in[0].gl_Position + step_vector * i + offset_vector;
			vertexOut.color = vertexIn[0].color;
			EmitVertex();
			gl_Position = gl_in[0].gl_Position + step_vector * (i + 1) + offset_vector;
			vertexOut.color = vertexIn[1].color;
			EmitVertex();
			EndPrimitive();

			gl_Position = gl_in[0].gl_Position + offset_vector + step_vector * (i + 1);
			vertexOut.color = vertexIn[1].color;
			EmitVertex();
			gl_Position = gl_in[0].gl_Position + offset_vector + step_vector * (i + 1) + step_vector_d1;// * vec4(window_height * 1.0 / window_width, 1.0, 1.0, 1.0);
			vertexOut.color = vertexIn[0].color;
			EmitVertex();
			EndPrimitive();

			gl_Position = gl_in[0].gl_Position + offset_vector + step_vector * (i + 1);
			vertexOut.color = vertexIn[1].color;
			EmitVertex();
			gl_Position = gl_in[0].gl_Position + offset_vector + step_vector * (i + 1) + step_vector_d2;// * vec4(window_height * 1.0 / window_width, 1.0, 1.0, 1.0);
			vertexOut.color = vertexIn[0].color;
			EmitVertex();
			EndPrimitive();
		} else if(length(step_vector * i + offset_vector) < len) {
			float len_d = length(step_vector * i + offset_vector);

			gl_Position = gl_in[0].gl_Position + step_vector * i + offset_vector;
			vertexOut.color = vertexIn[0].color;
			EmitVertex();
			gl_Position = gl_in[1].gl_Position;
			vertexOut.color = vertexIn[1].color;
			EmitVertex();
			EndPrimitive();

			gl_Position = gl_in[1].gl_Position;
			vertexOut.color = vertexIn[1].color;
			EmitVertex();
			gl_Position = gl_in[1].gl_Position + step_vector_d1 * (len - len_d) / step_length_d;// * vec4(window_height * 1.0 / window_width, 1.0, 1.0, 1.0);
			vertexOut.color = vertexIn[0].color;
			EmitVertex();
			EndPrimitive();

			gl_Position = gl_in[1].gl_Position;
			vertexOut.color = vertexIn[1].color;
			EmitVertex();
			gl_Position = gl_in[1].gl_Position + step_vector_d2 * (len - len_d) / step_length_d;// * vec4(window_height * 1.0 / window_width, 1.0, 1.0, 1.0);
			vertexOut.color = vertexIn[0].color;
			EmitVertex();
			EndPrimitive();
		}

	if(gl_InvocationID * vertex_num <= segment_num && segment_num < gl_InvocationID * vertex_num + vertex_num && offset > step_length_d) {
		gl_Position = gl_in[0].gl_Position;
		vertexOut.color = vertexIn[0].color;
		EmitVertex();
		gl_Position = gl_in[0].gl_Position + direction * (offset - step_length_d);
		vertexOut.color = vertexIn[1].color;
		EmitVertex();
		EndPrimitive();

		gl_Position = gl_in[0].gl_Position + direction * (offset - step_length_d);
		vertexOut.color = vertexIn[1].color;
		EmitVertex();
		gl_Position = gl_in[0].gl_Position + direction * (offset - step_length_d) + step_vector_d1 * (offset - step_length_d) / step_length_d;// * vec4(window_height * 1.0 / window_width, 1.0, 1.0, 1.0);
		vertexOut.color = vertexIn[0].color;
		EmitVertex();

		gl_Position = gl_in[0].gl_Position + direction * (offset - step_length_d);
		vertexOut.color = vertexIn[1].color;
		EmitVertex();
		gl_Position = gl_in[0].gl_Position + direction * (offset - step_length_d) + step_vector_d2 * (offset - step_length_d) / step_length_d;// * vec4(window_height * 1.0 / window_width, 1.0, 1.0, 1.0);
		vertexOut.color = vertexIn[0].color;
		EmitVertex();
	}

	if(gl_InvocationID * vertex_num <= segment_num && segment_num < gl_InvocationID * vertex_num + vertex_num && segment_num % 2 == 0) {
		if(length(step_vector * segment_num + offset_vector) < len) {
			float len_d = length(step_vector * segment_num + offset_vector);

			gl_Position = gl_in[0].gl_Position + step_vector * segment_num + offset_vector;
			vertexOut.color = vertexIn[0].color;
			EmitVertex();
			gl_Position = gl_in[1].gl_Position;
			vertexOut.color = vertexIn[1].color;
			EmitVertex();
			EndPrimitive();

			gl_Position = gl_in[1].gl_Position;
			vertexOut.color = vertexIn[1].color;
			EmitVertex();
			gl_Position = gl_in[1].gl_Position + step_vector_d1 * (len - len_d) / step_length_d;// * vec4(window_height * 1.0 / window_width, 1.0, 1.0, 1.0);
			vertexOut.color = vertexIn[0].color;
			EmitVertex();

			gl_Position = gl_in[1].gl_Position;
			vertexOut.color = vertexIn[1].color;
			EmitVertex();
			gl_Position = gl_in[1].gl_Position + step_vector_d2 * (len - len_d) / step_length_d;// * vec4(window_height * 1.0 / window_width, 1.0, 1.0, 1.0);
			vertexOut.color = vertexIn[0].color;
			EmitVertex();
		}
	}
}
