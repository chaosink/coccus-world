#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <algorithm>
#include <pthread.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "shader.hpp"
#include "texture.hpp"
#include "play_wav.hpp"

#define max(a, b) ((a) >= (b) ? (a) : (b))
#define min(a, b) ((a) <  (b) ? (a) : (b))
#define PI 3.14159265358979323846

int space            = 0;

int particle_num     = 20;   // n
double max_radius    = 0.1;  // r
double total_speed   = 1.0;  // s
double recoil_speed  = 1.0;  // p
double recoil_ratio	 = 0.01; // a

int decoration       = 1;    // d
int fullscreen       = 0;    // f
int window_width     = 1024; // w
int window_height    = 768;  // w

int game             = 1;    // g
int print            = 0;    // p
int develop          = 0;    // e

const int max_particle_num = 10000;
double border = 0.05;
double spacing = 0.05;
double fps = 60.0;
double window_ratio = 768.0 / 1024.0;
double background_speed = 0.001;
int surface = 1;
int audio = 1;
char audio_collision[] = "audio/collision1.wav";
char audio_absorption_dec[] = "audio/absorption1.wav";
char audio_absorption_inc[] = "audio/absorption1.wav";
char audio_ejection[] = "audio/ejection1.wav";
char audio_bg[] = "audio/bg1.wav";

struct Particle {
	double radius;
	glm::dvec2 pos, speed;
	glm::vec4 color;
	int targeted;
	int dead;
	int operator<(const Particle& that) const {
		return this->dead < that.dead;
	}
};
Particle particles[max_particle_num];

void SortParticles() {
	int i = 1, j = particle_num - 1;
	while(i < j) {
		while(!particles[i].dead && i < particle_num) i++;
		while( particles[j].dead && j > 0) j--;
		if(i < j) {
			Particle temp;
			temp = particles[i];
			particles[i] = particles[j];
			particles[j] = temp;
			i++;
			j--;	
		}
	}
//	std::sort( particles + game, particles + particle_num);
}

int Compare(Particle a, Particle b) {
	return a.radius < b.radius;
}

void SortParticlesByRadius() {
	std::sort(particles + game, particles + particle_num, Compare);
}

void CollisionTest(int n) {
	static double time_collision = 0;
	double 
		left = (particles[n].radius + border) * window_ratio - 1, right = 1 - (border + particles[n].radius) * window_ratio,
		down = (particles[n].radius + border)                - 1, up    = 1 - (border + particles[n].radius)               ;

	if(particles[n].pos.x < left) {
		particles[n].pos.x = left * 2 - particles[n].pos.x;
		particles[n].speed.x *= -1;
		if(!n && audio) {
			double time_collision_current = glfwGetTime();
			if(time_collision_current - time_collision > 0.1) {
				play_wav_thread(audio_collision);
				time_collision = time_collision_current;
			}
		}
	}
	if(particles[n].pos.x > right) {
		particles[n].pos.x = right * 2 - particles[n].pos.x;
		particles[n].speed.x *= -1;
		if(!n && audio) {
			double time_collision_current = glfwGetTime();
			if(time_collision_current - time_collision > 0.1) {
				play_wav_thread(audio_collision);
				time_collision = time_collision_current;
			}
		}
	}
	if(particles[n].pos.y < down) {
		particles[n].pos.y = down * 2 - particles[n].pos.y;
		particles[n].speed.y *= -1;
		if(!n && audio) {
			double time_collision_current = glfwGetTime();
			if(time_collision_current - time_collision > 0.1) {
				play_wav_thread(audio_collision);
				time_collision = time_collision_current;
			}
		}
	}
	if(particles[n].pos.y > up) {
		particles[n].pos.y = up * 2 - particles[n].pos.y;
		particles[n].speed.y *= -1;
		if(!n && audio) {
			double time_collision_current = glfwGetTime();
			if(time_collision_current - time_collision > 0.1) {
				play_wav_thread(audio_collision);
				time_collision = time_collision_current;
			}
		}
	}
}

int Inside(int n) {
	double left = particles[n].radius * window_ratio - 1 + border * window_ratio, right = 1 - border * window_ratio - particles[n].radius * window_ratio, down = particles[n].radius - 1 + border, up = 1 - border - particles[n].radius;
	if(particles[n].pos.x < left || particles[n].pos.x > right || particles[n].pos.y < down || particles[n].pos.y > up) return 0;
	return 1;
}

int Inside(double x, double y, double r) {
	double left = r * window_ratio - 1 + border * window_ratio, right = 1 - border * window_ratio - r * window_ratio, down = r - 1 + border, up = 1 - border - r;
	if(x < left || x > right || y < down || y > up) return 0;
	return 1;
}

int Digit(int n) {
	int i = 0;
	while(n) {
		n /= 10;
		i++;
	}
	return i;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	window_width = width;
	window_height = height;
	window_ratio = window_height * 1.0 / window_width;
}

int main(int argc, char** argv) {
/*--------------------------------------------------*/
// command line arguments
	int oc; // option character
	while((oc = getopt(argc, argv, "n:r:s: dfw: eghp")) != -1) {
		switch(oc) {
			case 'n':
				particle_num = atoi(optarg);
				break;
			case 'r':
				max_radius = atof(optarg);
				break;
			case 's':
				total_speed = atof(optarg);
				break;

			case 'd':
				decoration = 0;
				break;
			case 'f':
				fullscreen = 1;
				break;
			case 'w':
				window_width = atoi(optarg);
				window_height = atoi(optarg + Digit(window_width) + 1);
				window_ratio = window_height * 1.0 / window_width;
				break;

			case 'e':
				develop = 1;
				break;
			case 'g':
				game = 1;
				break;
			case 'h':
				printf(
"None\n");
				return 0;
				break;
			case 'p':
				print = 1;
				break;
		}
	}
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// initialize GLFW & GLEW and create window
	if(!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW!\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DECORATED, decoration);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(window_width, window_height, "absorption", fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
	if(window == NULL) {
		fprintf(stderr, "Failed to open GLFW window!\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glewExperimental = true; // Needed for core profile
	if(glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW!\n");
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
//	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// initialize OpenGL
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	GLuint programID_rendering = LoadShaders("vertex.glsl", "fragment.glsl");
	GLuint textureID_rendering = glGetUniformLocation(programID_rendering, "texture");

	GLuint programID_triangle = LoadShaders("vertex.glsl", "fragment.glsl", "geometry_triangle.glsl");
	GLuint window_widthID_triangle = glGetUniformLocation(programID_triangle, "window_width");
	GLuint window_heightID_triangle = glGetUniformLocation(programID_triangle, "window_height");
	GLuint textureID_primitive = glGetUniformLocation(programID_triangle, "texture");

	GLuint programID_line = LoadShaders("vertex.glsl", "fragment.glsl", "geometry_line.glsl");
	GLuint window_widthID_line = glGetUniformLocation(programID_line, "window_width");
	GLuint window_heightID_line = glGetUniformLocation(programID_line, "window_height");

	GLuint programID_dash_line = LoadShaders("vertex.glsl", "fragment.glsl", "geometry_dash_line.glsl");
	GLuint window_widthID_dash_line = glGetUniformLocation(programID_dash_line, "window_width");
	GLuint window_heightID_dash_line = glGetUniformLocation(programID_dash_line, "window_height");

	GLfloat g_vertex_buffer_data[max_particle_num][3];
	GLfloat g_color_buffer_data[max_particle_num][4];
	GLfloat g_uv_buffer_data[max_particle_num][2];

	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	GLuint color_buffer;
	glGenBuffers(1, &color_buffer);
	GLuint uv_buffer;
	glGenBuffers(1, &uv_buffer);
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// textures
	int image_width, image_height;

	GLuint texture_cursor = LoadTexture("image/cursor.png", NULL, NULL, 4);
	GLuint texture_background1 = LoadTexture("image/matrix.png", &image_width, &image_height, 4);
	int image_width1 = image_width;
	int image_height1 = image_height;
	GLuint texture_background2 = LoadTexture("image/background.png", &image_width, &image_height, 4);
	GLuint texture_coccus = LoadTexture("image/coccus.png", NULL, NULL, 4);
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// initialize particles
	srand(time(NULL));
	while(1) {
		printf("Map generating!\n");
		do {
			particles[0].radius = rand() % 2000 / 2000.0f * max_radius;
			particles[0].pos = glm::dvec2((rand() % 2000 - 1000.0f) / 1000.0f * (1 - border * window_ratio), (rand() % 2000 - 1000.0f) / 1000.0f * (1 - border));
			particles[0].radius = max_radius * 0.6;
			particles[0].pos = glm::dvec2(0.0, 0.0);
		} while(!Inside(0));
		for(int i = 1; i < particle_num; i++)
			while(1) {
				particles[i].radius = rand() % 2000 / 2000.0f * max_radius;
				particles[i].pos = glm::dvec2((rand() % 2000 - 1000.0f) / 1000.0f * (1 - border * window_ratio), (rand() % 2000 - 1000.0f) / 1000.0f * (1 - border));
				int available = 1;
				for(int j = 0; j < i; j++) {
					if(!Inside(i) || glm::distance(particles[i].pos * glm::dvec2(window_width * 1.0 / window_height, 1), particles[j].pos * glm::dvec2(window_width  * 1.0 / window_height, 1)) < particles[i].radius + particles[j].radius + (j == 0 ? spacing : 0)) {
						available = 0;
						break;
					}
				}
				if(available) break;
			}

		SortParticlesByRadius();
		int available = 1;

	if(surface) {
		double surf_sum = pow(particles[0].radius, 2); // surface
		for(int i = 1; i < particle_num; i++) {
			if(particles[i].radius <= particles[0].radius) surf_sum += pow(particles[i].radius, 2);
			else if(surf_sum < pow(particles[i].radius, 2)) {
				available = 0;
				break;
			} else surf_sum += pow(particles[i].radius, 2);
		}
	} else {
		double volu_sum = pow(particles[0].radius, 3); // volume
		for(int i = 1; i < particle_num; i++) {
			if(particles[i].radius <= particles[0].radius) volu_sum += pow(particles[i].radius, 3);
			else if(volu_sum < pow(particles[i].radius, 3)) {
				available = 0;
				break;
			} else volu_sum += pow(particles[i].radius, 3);
		}
	}

		if(available) break;
	}
	printf("Map generated!\n");

	for(int i = 0; i < particle_num; i++) {
		particles[i].speed = glm::dvec2((rand() % 2000 - 1000.0f) / 2000000.0f, (rand() % 2000 - 1000.0f) / 2000000.0f);
//		particles[i].color = glm::vec4(rand() % 2000 / 2000.0f, rand() % 2000 / 2000.0f, rand() % 2000 / 2000.0f, rand() % 2000 / 2000.0f);
		particles[i].color.b = rand() % 2000 / 2000.0f; // use blue channel as color variation phase
		particles[i].dead = 0;
		particles[particle_num].targeted = 0;
	}
	particles[0].speed = glm::dvec2(0.0, 0.0);

/*	particles[1].radius = 0.2; // head-on collision test
	particles[1].pos = glm::dvec2(-0.5, 0.0);
	particles[1].speed = glm::dvec2(0.0002, 0.0);
	particles[2].radius = 0.2;
	particles[2].pos = glm::dvec2(0.5, 0.0);
	particles[2].speed = glm::dvec2(-0.0002, 0.0);*/
/*--------------------------------------------------*/



/*--------------------------------------------------*/
//  initialize state variable
	play_wav_thread(audio_bg);

	int released_mouse_left = 1, released_mouse_right = 1, released_space = 1;
	double time_left_last = 0, time_left_current, time_right_last = 0, time_right_current;
	double recoil_ratio_d;
	int run = 1;
	double time_current;
	int frame_count = 0;
	int bad_situation = 0;
	double background1_angle = 0, background2_angle = 0;
	double time_absorption = 0;
	int drag = 0;
	glm::dvec2 drag_vector;
	int drag_target = -1;

	glfwSetTime(0);
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// keyboard
	do {
		if(glfwGetKey(window, GLFW_KEY_LEFT ) == GLFW_PRESS) total_speed = glm::clamp(total_speed - 0.025, -10.0, 10.0);
		if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) total_speed = glm::clamp(total_speed + 0.025, -10.0, 10.0);
	if(develop || particles[0].dead) {
		if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) particles[0].speed.x -= 0.0001;
		if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) particles[0].speed.x += 0.0001;
		if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) particles[0].speed.y -= 0.0001;
		if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) particles[0].speed.y += 0.0001;
	}
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// mouse
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		double xpos_clamp = xpos / window_width * 2 - 1, ypos_clamp = 1 - ypos / window_height * 2;



	/*-----------------------------------*/
	// drag 
		if(drag && drag_target != -1) {
			if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
				particles[drag_target].pos = glm::dvec2(xpos_clamp, ypos_clamp) + drag_vector;
				if(!Inside(xpos_clamp + drag_vector.x, 0, particles[drag_target].radius))
					particles[drag_target].pos.x = glm::clamp(xpos_clamp + drag_vector.x, (border + particles[drag_target].radius) * window_ratio - 1, 1 - (border + particles[drag_target].radius) * window_ratio);
				if(!Inside(0, ypos_clamp + drag_vector.y, particles[drag_target].radius))
					particles[drag_target].pos.y = glm::clamp(ypos_clamp + drag_vector.y, (border + particles[drag_target].radius)                - 1, 1 - (border + particles[drag_target].radius)               );
				drag_vector = particles[drag_target].pos - glm::dvec2(xpos_clamp, ypos_clamp);
			} else if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
				particles[drag_target].speed = 0.02 * (glm::dvec2(xpos_clamp, ypos_clamp) - particles[drag_target].pos);
			} else {
				drag = 0;
				drag_vector = glm::dvec2(0);
			}
		}
	/*-----------------------------------*/



		glm::dvec2 direction_d = glm::dvec2((particles[0].pos.x - xpos_clamp) / window_ratio, particles[0].pos.y - ypos_clamp);
		if(!direction_d.x && !direction_d.y) direction_d.y = 1.0;
		glm::dvec2 direction = glm::normalize(direction_d);
		double beta = glm::dot(direction, glm::dvec2(1.0, 0.0));



	/*-----------------------------------*/
	// left mouse button
		if(released_mouse_left && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			released_mouse_left = 0;

		/*--------------------*/
		// point selection
			glfwGetKey(window, GLFW_KEY_Z); // consume the GLRW_PRESS state
			glfwGetKey(window, GLFW_KEY_X);
			glfwGetKey(window, GLFW_KEY_C);
			if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) { // key Z, increase or decrease
				for(int i = 0; i < particle_num; i++)
					if(glm::length(glm::dvec2((xpos_clamp - particles[i].pos.x) / window_ratio, ypos_clamp - particles[i].pos.y)) < particles[i].radius) {
						particles[i].radius *= 1.1;
						break;
					}
			} else if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) { // key X, add or remove
				particles[particle_num].radius = rand() % 2000 / 2000.0f * max_radius;
				particles[particle_num].pos = glm::dvec2(xpos_clamp, ypos_clamp);
				particles[particle_num].speed = glm::dvec2((rand() % 2000 - 1000.0f) / 2000000.0f, (rand() % 2000 - 1000.0f) / 2000000.0f);
				particles[particle_num].color.b = rand() % 2000 / 2000.0f;
				particles[particle_num].dead = 0;
				particles[particle_num].targeted = 0;

				if(Inside(particle_num)) particle_num++;
			} else if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) { // key C, drag, undirected or directed
				for(int i = 0; i < particle_num; i++)
					if(glm::length(glm::dvec2((xpos_clamp - particles[i].pos.x) / window_ratio, ypos_clamp - particles[i].pos.y)) < particles[i].radius) {
						if(drag_target != -1) particles[drag_target].targeted = 0;
						drag_target = i;
						particles[i].targeted = 1;
						drag_vector = particles[i].pos - glm::dvec2(xpos_clamp, ypos_clamp);
						break;
					}
				drag = 1;
		/*--------------------*/

		/*--------------------*/
		// ejection
			} else if(!particles[0].dead) {
				play_wav_thread(audio_ejection);

				time_left_current = glfwGetTime();
				if(time_left_current - time_left_last < 0.2) recoil_ratio_d += recoil_ratio;
				else recoil_ratio_d = recoil_ratio;
				time_left_last = time_left_current;

			/*	particles[0].speed += direction * 0.0005;
				double len = glm::length(particles[0].speed);
				if(len > 10000 / 1000000.0f) particles[0].speed *= 10000 / 1000000.0f / len;*/

			if(surface) {
				double recoil_ratio_d_sqrt = sqrt(recoil_ratio_d); // surface
				particles[particle_num].radius = particles[0].radius * recoil_ratio_d_sqrt;
				particles[0].radius = particles[0].radius * sqrt(1 - recoil_ratio_d);
			} else {
				double recoil_ratio_d_cbrt = pow(recoil_ratio_d, 1.0 / 3); // volume
				particles[particle_num].radius = particles[0].radius * recoil_ratio_d_cbrt;
				particles[0].radius = particles[0].radius * pow(1 - recoil_ratio_d, 1.0 / 3);
			}

				particles[particle_num].pos = particles[0].pos - direction * (particles[0].radius + particles[particle_num].radius) * glm::dvec2(window_height * 1.0 / window_width, 1.0);
				particles[particle_num].speed = -direction * glm::dvec2(window_height * 1.0 / window_width, 1.0) * recoil_speed * 0.02;
				particles[particle_num].dead = 0;
				particles[particle_num].targeted = 0;
				particles[0].speed = particles[0].speed * (1 - recoil_ratio_d * 2) / (1 - recoil_ratio_d) - particles[particle_num].speed * recoil_ratio_d / (1 - recoil_ratio_d);

//				printf("%lf\n", particles[particle_num].radius);
				printf("particle_num after recoil    : %d\n", particle_num + 1);
				particle_num++;
			}
		}
		/*--------------------*/

		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) released_mouse_left = 1;
	/*-----------------------------------*/



	/*-----------------------------------*/
	// right mouse button
		if(released_mouse_right && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			released_mouse_right = 0;

		/*--------------------*/
		// point selection
			glfwGetKey(window, GLFW_KEY_Z);
			glfwGetKey(window, GLFW_KEY_X);
			glfwGetKey(window, GLFW_KEY_C);
			if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
				for(int i = 0; i < particle_num; i++)
					if(glm::length(glm::dvec2((xpos_clamp - particles[i].pos.x) / window_ratio, ypos_clamp - particles[i].pos.y)) < particles[i].radius) {
						particles[i].radius *= 0.9;
						break;
					}
			} else if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
				for(int i = 0; i < particle_num; i++)
					if(glm::length(glm::dvec2((xpos_clamp - particles[i].pos.x) / window_ratio, ypos_clamp - particles[i].pos.y)) < particles[i].radius) {
						particles[i].dead = 1;
						if(!i) particles[0].radius = 0;
						if(i == drag_target) drag_target = -1;
						break;
					}
			} else if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
				for(int i = 0; i < particle_num; i++)
					if(glm::length(glm::dvec2((xpos_clamp - particles[i].pos.x) / window_ratio, ypos_clamp - particles[i].pos.y)) < particles[i].radius) {
						if(drag_target != -1) particles[drag_target].targeted = 0;
						drag_target = i;
						particles[i].targeted = 1;
						drag_vector = particles[i].pos - glm::dvec2(xpos_clamp, ypos_clamp);
						break;
					}
				drag = 1;
		/*--------------------*/

		/*--------------------*/
		// empty ejection
			} else if(particles[0].dead) {
				play_wav_thread(audio_ejection);

				time_right_current = glfwGetTime();
				if(time_right_current - time_right_last < 0.2) recoil_ratio_d += recoil_ratio;
				else recoil_ratio_d = recoil_ratio;
				time_left_last = time_left_current;
				particles[0].speed += direction * 0.0005 * recoil_ratio_d / recoil_ratio;

			/*	particles[0].speed += direction * 0.0005;
				double len = glm::length(particles[0].speed);
				if(len > 10000 / 1000000.0f) particles[0].speed *= 10000 / 1000000.0f / len;*/
			}
		}
		/*--------------------*/

		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) released_mouse_right = 1;
	/*-----------------------------------*/
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// move and collision test
		for(int i = 0; i < particle_num; i++) { // if(frame_count != 0) {
			particles[i].pos += particles[i].speed * total_speed;
			CollisionTest(i);
		}
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// absorption
		double dist;
		for(int i = 0; i < particle_num; i++)
			for(int j = i + 1; j < particle_num; j++)
				if(!particles[i].dead && !particles[j].dead && (dist = glm::distance(particles[i].pos * glm::dvec2(window_width * 1.0 / window_height, 1), particles[j].pos * glm::dvec2(window_width  * 1.0 / window_height, 1))) < particles[i].radius + particles[j].radius) {
					int big = particles[i].radius > particles[j].radius ? i : j;
					int small = i + j - big;
				//	printf("radius_before_absorption: %lf %lf\n", particles[big].radius, particles[small].radius);
					double r1, r2, m1, m2;

					if(!i && audio) {
						double time_absorption_current = glfwGetTime();
						if(time_absorption_current - time_absorption > 0.5) {
							if(particles[0].radius > particles[j].radius) play_wav_thread(audio_absorption_inc);
							else play_wav_thread(audio_absorption_dec);
							time_absorption = time_absorption_current;
						}
					}

				if(surface) {
					double surf = pow(particles[small].radius, 2) + pow(particles[big].radius, 2); // surface
					r1 = sqrt(surf / 2 - pow(dist, 2) / 4) + dist / 2;
					r2 = dist - r1;
					if(dist < max(particles[i].radius, particles[j].radius)) {
						r1 = sqrt(surf);
						r2 = 0;
					}
					m1 = pow(particles[big].radius, 2);
					m2 = pow(particles[small].radius, 2) - pow(r2, 2);
				} else {
					double volu = pow(particles[small].radius, 3) + pow(particles[big].radius, 3); // volume
					r1 = sqrt(volu / 3 / dist - pow(dist, 2) / 12) + dist / 2;
					r2 = dist - r1;
					if(dist < max(particles[i].radius, particles[j].radius)) {
						r1 = pow(volu, 1.0 / 3);
						r2 = 0;
					}
					m1 = pow(particles[big].radius, 3);
					m2 = pow(particles[small].radius, 3) - pow(r2, 3);
				}

					glm::dvec2 v1 = particles[big].speed;
					glm::dvec2 v2 = particles[small].speed;
					particles[big].radius = r1;
					particles[small].radius = r2;
					particles[big].speed = (m1 * v1 + m2 * v2) / (m1 + m2);
				//	printf("radius_after_absorption : %lf %lf\n\n", particles[big].radius, particles[small].radius);

					if(particles[big].radius >= 1 - border) particles[big].dead = 1;
					if(particles[small].radius <= 0) {
						particles[small].dead = 1;
						if(game && !small) {
							printf("Failed!\n");
						//	return 0;
						}
						if(small == drag_target) {
							drag_target = big;
							drag_vector += particles[big].pos - particles[small].pos;
							particles[small].targeted = 0;
							particles[big].targeted = 1;
						}
					}
				}

		int particle_num_d = particle_num;
		for(int i = 1; i < particle_num; i++)
			if(particles[i].dead) particle_num_d--;
		if(particle_num != particle_num_d) {
			SortParticles();
			particle_num = particle_num_d;
			printf("particle_num after absorption: %d\n", particle_num - particles[0].dead);
			if(game && particle_num == 1) {
				printf("Succeeded!\n");
//				return 0;
			}
		}

/*		printf("%d\n", particle_num - particles[0].dead);
		for(int i = 0; i < 10; i++) printf("%d ", particles[i].dead); puts("");
		for(int i = 0; i < 10; i++) printf("%lf ", particles[i].radius); puts("");*/
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// bad situation test
		SortParticlesByRadius();
		int available = 1;

	if(surface) {
		double surf_sum = pow(particles[0].radius, 2); // surface
		for(int i = 1; i < particle_num; i++) {
			if(particles[i].radius <= particles[0].radius) surf_sum += pow(particles[i].radius, 2);
			else if(surf_sum < pow(particles[i].radius, 2)) {
				available = 0;
				break;
			} else surf_sum += pow(particles[i].radius, 2);
		}
	} else {
		double volu_sum = pow(particles[0].radius, 3); // volume
		for(int i = 1; i < particle_num; i++) {
			if(particles[i].radius <= particles[0].radius) volu_sum += pow(particles[i].radius, 3);
			else if(volu_sum < pow(particles[i].radius, 3)) {
				available = 0;
				break;
			} else volu_sum += pow(particles[i].radius, 3);
		}
	}

		if(!bad_situation && !available) {
			printf("Bad situation!\n");
			bad_situation = 1;
		}

	for(int i = 0; i < particle_num; i++) // get new drag target
		if(particles[i].targeted) drag_target = i;
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// color
		for(int i = 1; i < particle_num; i++)
			if(particles[i].radius > particles[0].radius)
				particles[i].color = glm::vec4(1.0, 0.0, particles[i].color.b + background_speed * 5, 1.0);
			else
				particles[i].color = glm::vec4(0.0, 1.0, particles[i].color.b + background_speed * 5, 1.0);
		particles[0].color = glm::vec4(0.0, 0.0, particles[0].color.b + background_speed * 5, 1.0);
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// begin rendering
		glClear(GL_COLOR_BUFFER_BIT);
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// background
		double backgrcound_vertex[4][2] = {
			-1,-1,
			 1,-1,
			-1, 1,
			 1, 1,
		};

		double border_uv[4][2] = {
			 0.0, 1.0 - 0.0,
			 1.0, 1.0 - 0.0,
			 0.0, 1.0 - 1.0,
			 1.0, 1.0 - 1.0,
		};

		for(int i = 0; i < 4; i++) {
			g_vertex_buffer_data[i][0] = backgrcound_vertex[i][0];
			g_vertex_buffer_data[i][1] = backgrcound_vertex[i][1];
			g_vertex_buffer_data[i][2] = 0;
		}

		glUseProgram(programID_rendering);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLfloat) * 3, g_vertex_buffer_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			0,        // The attribute we want to configure
			3,        // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,        // stride
			(void*)0  // array buffer offset
		);

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(textureID_rendering, 0);

		double border_x, border_y;



	/*-------------------------*/
	// background1
		background1_angle += background_speed * total_speed;
		border_x = cos(background1_angle) * 0.25 + 0.25;
		border_y = sin(background1_angle) * 0.25 + 0.25;

		for(int i = 0; i < 4; i++) {
			g_uv_buffer_data[i][0] = border_x + border_uv[i][0] / 2 * image_height1 / image_width1;
			g_uv_buffer_data[i][1] = border_y + border_uv[i][1] / 2 * window_ratio;
		}

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLfloat) * 2, g_uv_buffer_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			2,        // The attribute we want to configure
			2,        // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,        // stride
			(void*)0  // array buffer offset
		);

		glBindTexture(GL_TEXTURE_2D, texture_background1);
		glUniform1i(glGetUniformLocation(programID_rendering, "object"), 3);
		glUniform1f(glGetUniformLocation(programID_rendering, "iGlobalTime"), background2_angle);
		glUniform3f(glGetUniformLocation(programID_rendering, "iResolution"), window_width, window_height, 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	/*-------------------------*/



	/*-------------------------*/
	// background2
		background2_angle -= background_speed * 10 * total_speed;
		border_x = cos(background2_angle) / 4 + 0.25;
		border_y = sin(background2_angle) / 4 + 0.25;

		for(int i = 0; i < 4; i++) {
			g_uv_buffer_data[i][0] = border_x + border_uv[i][0] / 2;
			g_uv_buffer_data[i][1] = border_y + border_uv[i][1] / 2;
		}

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLfloat) * 2, g_uv_buffer_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			2,        // The attribute we want to configure
			2,        // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,        // stride
			(void*)0  // array buffer offset
		);

		glBindTexture(GL_TEXTURE_2D, texture_background2);
		glUniform1i(glGetUniformLocation(programID_rendering, "object"), 4);
		glUniform1f(glGetUniformLocation(programID_rendering, "iGlobalTime"), background2_angle + PI);
		glUniform3f(glGetUniformLocation(programID_rendering, "iResolution"), window_width, window_height, 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	/*-------------------------*/
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// border
/*		double border_vertex[4][2] = {
			border * window_ratio - 1, border - 1,
			1 - border * window_ratio, border - 1,
			border * window_ratio - 1, 1 - border,
			1 - border * window_ratio, 1 - border,
		};*/

		double border_vertex[10][2] = {
			border * window_ratio - 1, border - 1,
			 1,-1,
			1 - border * window_ratio, border - 1,
			 1, 1,
			1 - border * window_ratio, 1 - border,
			-1, 1,
			border * window_ratio - 1, 1 - border,
			-1,-1,
			border * window_ratio - 1, border - 1,
			 1,-1,
		};

/*		double border_uv[4][2] = {
			 0.0, 1.0 - 0.0,
			 1.0, 1.0 - 0.0,
			 0.0, 1.0 - 1.0,
			 1.0, 1.0 - 1.0,
		};*/

		for(int i = 0; i < 10; i++) {
			g_vertex_buffer_data[i][0] = border_vertex[i][0];
			g_vertex_buffer_data[i][1] = border_vertex[i][1];
			g_vertex_buffer_data[i][2] = 0;
			g_color_buffer_data [i][0] = 204.0 / 255;
			g_color_buffer_data [i][1] = 219.0 / 255;
			g_color_buffer_data [i][2] = 231.0 / 255;
			g_color_buffer_data [i][3] = 0.25;
/*			g_uv_buffer_data    [i][0] = x + border_uv[i][0] / 2;
			g_uv_buffer_data    [i][1] = y + border_uv[i][1] / 2;*/
		}

		glUseProgram(programID_rendering);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, 10 * sizeof(GLfloat) * 3, g_vertex_buffer_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			0,        // The attribute we want to configure
			3,        // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,        // stride
			(void*)0  // array buffer offset
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
		glBufferData(GL_ARRAY_BUFFER, 10 * sizeof(GLfloat) * 4, g_color_buffer_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			1,		  // The attribute we want to configure
			4,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void*)0  // array buffer offset
		);

/*		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
		glBufferData(GL_ARRAY_BUFFER, 10 * sizeof(GLfloat) * 2, g_uv_buffer_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			2,        // The attribute we want to configure
			2,        // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,        // stride
			(void*)0  // array buffer offset
		);*/

		glUniform1i(glGetUniformLocation(programID_rendering, "object"), 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 10);
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// cue
		glm::dvec2 pos1 = glm::dvec2(particles[0].pos.x, particles[0].pos.y);
		glm::dvec2 pos2 = glm::dvec2(xpos_clamp, ypos_clamp);
		glm::dvec2 cue_direction = pos1 - pos2;
		glm::dvec2 pos1_border;
		glm::dvec2 pos2_border;
		double x, y;

		if(!cue_direction.x && !cue_direction.y) {
			pos1_border.x = xpos_clamp;
			pos1_border.y = 1 - border;
			pos2_border.x = xpos_clamp;
			pos2_border.y = border - 1;
		} else if (std::abs(cue_direction.y) > std::abs(cue_direction.x)) {
			y = 1 - border;
			x = (y - pos1.y) * (pos1.x - pos2.x) / (pos1.y - pos2.y) + pos1.x;
			if(x > 1 - border * window_ratio) {
				x = 1 - border * window_ratio;
				y = (x - pos1.x) * (pos1.y - pos2.y) / (pos1.x - pos2.x) + pos1.y;
			} else if(x < border * window_ratio - 1) {
				x = border * window_ratio - 1;
				y = (x - pos1.x) * (pos1.y - pos2.y) / (pos1.x - pos2.x) + pos1.y;
			}
			pos1_border = glm::dvec2(x, y);

			y = border - 1;
			x = (y - pos1.y) * (pos1.x - pos2.x) / (pos1.y - pos2.y) + pos1.x;
			if(x > 1 - border * window_ratio) {
				x = 1 - border * window_ratio;
				y = (x - pos1.x) * (pos1.y - pos2.y) / (pos1.x - pos2.x) + pos1.y;
			} else if(x < border * window_ratio - 1) {
				x = border * window_ratio - 1;
				y = (x - pos1.x) * (pos1.y - pos2.y) / (pos1.x - pos2.x) + pos1.y;
			}
			pos2_border = glm::dvec2(x, y);
		} else {
			x = 1 - border * window_ratio;
			y = (x - pos1.x) * (pos1.y - pos2.y) / (pos1.x - pos2.x) + pos1.y;
			if(y > 1 - border) {
				y = 1 - border;
				x = (y - pos1.y) * (pos1.x - pos2.x) / (pos1.y - pos2.y) + pos1.x;
			} else if(y < border - 1) {
				y = border - 1;
				x = (y - pos1.y) * (pos1.x - pos2.x) / (pos1.y - pos2.y) + pos1.x;
			}
			pos1_border = glm::dvec2(x, y);
			
			x = border * window_ratio - 1;
			y = (x - pos1.x) * (pos1.y - pos2.y) / (pos1.x - pos2.x) + pos1.y;
			if(y > 1 - border) {
				y = 1 - border;
				x = (y - pos1.y) * (pos1.x - pos2.x) / (pos1.y - pos2.y) + pos1.x;
			} else if(y < border - 1) {
				y = border - 1;
				x = (y - pos1.y) * (pos1.x - pos2.x) / (pos1.y - pos2.y) + pos1.x;
			}
			pos2_border = glm::dvec2(x, y);
		}

		if(glm::dot(direction, pos2_border - pos1_border) < 0) {
			glm::dvec2 temp = pos1_border;
			pos1_border = pos2_border;
			pos2_border = temp;
		} // long

//		pos1_border = pos2; pos2_border = pos1; // short

		g_vertex_buffer_data[0][0] = pos1_border.x;
		g_vertex_buffer_data[0][1] = pos1_border.y;
		g_vertex_buffer_data[0][2] = 0;
		g_color_buffer_data [0][0] = 0.1;
		g_color_buffer_data [0][1] = 0.1;
		g_color_buffer_data [0][2] = 0.1;
		g_color_buffer_data [0][3] = 0.2;

		g_vertex_buffer_data[1][0] = pos2_border.x;
		g_vertex_buffer_data[1][1] = pos2_border.y;
		g_vertex_buffer_data[1][2] = 0;
		g_color_buffer_data [1][0] = 0.1;
		g_color_buffer_data [1][1] = 0.1;
		g_color_buffer_data [1][2] = 0.1;
		g_color_buffer_data [1][3] = 0.2;

		glUseProgram(programID_dash_line);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat) * 3, g_vertex_buffer_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			0,        // The attribute we want to configure
			3,        // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,        // stride
			(void*)0  // array buffer offset
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat) * 4, g_color_buffer_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			1,		  // The attribute we want to configure
			4,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void*)0  // array buffer offset
		);

		glUniform1i(window_widthID_dash_line, window_width);
		glUniform1i(window_heightID_dash_line, window_height);
		glUniform1f(glGetUniformLocation(programID_dash_line, "iGlobalTime"), background1_angle * 20);

		glUniform1i(glGetUniformLocation(programID_rendering, "object"), 0);

		glDrawArrays(GL_LINES, 0, 2);
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// circles
		for(int i = 0; i < particle_num; i++) {
			g_vertex_buffer_data[i][0] = particles[i].pos.x;
			g_vertex_buffer_data[i][1] = particles[i].pos.y;
			g_vertex_buffer_data[i][2] = particles[i].radius;
			g_color_buffer_data [i][0] = particles[i].color.r;
			g_color_buffer_data [i][1] = particles[i].color.g;
			g_color_buffer_data [i][2] = particles[i].color.b;
			g_color_buffer_data [i][3] = particles[i].color.a;
		}

		glUseProgram(programID_triangle);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, particle_num * sizeof(GLfloat) * 3, g_vertex_buffer_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			0,        // The attribute we want to configure
			3,        // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,        // stride
			(void*)0  // array buffer offset
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
		glBufferData(GL_ARRAY_BUFFER, particle_num * sizeof(GLfloat) * 4, g_color_buffer_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			1,		  // The attribute we want to configure
			4,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void*)0  // array buffer offset
		);

		glUniform1i(window_widthID_triangle, window_width);
		glUniform1i(window_heightID_triangle, window_height);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_coccus);
		glUniform1i(textureID_primitive, 0);
		glUniform1i(glGetUniformLocation(programID_triangle, "object"), 2);
		glUniform1f(glGetUniformLocation(programID_rendering, "iGlobalTime"), background1_angle * 20);

		glDrawArrays(GL_POINTS, 0, particle_num);
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// radiance
		for(int particle_num_d = 0; particle_num_d < particle_num; particle_num_d++)
			if(particles[particle_num_d].radius >= 0.1) break; // small circles
		particle_num_d = particle_num; // all circles

		glUseProgram(programID_line);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, particle_num_d * sizeof(GLfloat) * 3, g_vertex_buffer_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			0,        // The attribute we want to configure
			3,        // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,        // stride
			(void*)0  // array buffer offset
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
		glBufferData(GL_ARRAY_BUFFER, particle_num_d * sizeof(GLfloat) * 4, g_color_buffer_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			1,		  // The attribute we want to configure
			4,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void*)0  // array buffer offset
		);

		glUniform1i(window_widthID_line, window_width);
		glUniform1i(window_heightID_line, window_height);

		glUniform1i(glGetUniformLocation(programID_triangle, "object"), 0);

		glDrawArrays(GL_POINTS, 0, particle_num_d);
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// spacing around charactor
	if(space) {
		for(int i = 0; i < 64 + 1; i++) {
			g_vertex_buffer_data[i][0] = particles[0].pos.x + cos(PI * 2 / 64 * i) * (particles[0].radius + spacing) * window_ratio;
			g_vertex_buffer_data[i][1] = particles[0].pos.y + sin(PI * 2 / 64 * i) * (particles[0].radius + spacing);
			g_vertex_buffer_data[i][2] = 0;
			g_color_buffer_data [i][0] = particles[0].color.r;
			g_color_buffer_data [i][1] = particles[0].color.g;
			g_color_buffer_data [i][2] = particles[0].color.b;
			g_color_buffer_data [i][3] = particles[0].color.a;
		}

		glUseProgram(programID_rendering);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, 65 * sizeof(GLfloat) * 3, g_vertex_buffer_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			0,        // The attribute we want to configure
			3,        // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,        // stride
			(void*)0  // array buffer offset
		);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
		glBufferData(GL_ARRAY_BUFFER, 65 * sizeof(GLfloat) * 4, g_color_buffer_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			1,        // The attribute we want to configure
			4,        // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,        // stride
			(void*)0  // array buffer offset
		);

		glUniform1i(glGetUniformLocation(programID_triangle, "object"), 0);

		glDrawArrays(GL_LINE_STRIP, 0, 65);
	}
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// cursor_new
		double cursor_vertex[4][2] = {
			-0.05,-0.05,
			 0.05,-0.05,
			-0.05, 0.05,
			 0.05, 0.05,
		};
		
		double cursor_uv[4][2] = {
			 0.0, 1.0 - 0.0,
			 1.0, 1.0 - 0.0,
			 0.0, 1.0 - 1.0,
			 1.0, 1.0 - 1.0,
		};

		for(int i = 0; i < 4; i++) {
			glm::dvec2 rotated_cursor_vertex = glm::dvec2(xpos_clamp / window_ratio, ypos_clamp) + glm::rotate(glm::dvec2(cursor_vertex[i][0], cursor_vertex[i][1]), double((acos(beta) * (direction.y > 0 ? 1 : -1)) / PI * 180));
			g_vertex_buffer_data[i][0] = rotated_cursor_vertex.x * window_ratio;
			g_vertex_buffer_data[i][1] = rotated_cursor_vertex.y;
			g_vertex_buffer_data[i][2] = 0;
			g_color_buffer_data [i][0] = 0.5;
			g_color_buffer_data [i][1] = 0.5;
			g_color_buffer_data [i][2] = 0.5;
			g_color_buffer_data [i][3] = 1.0;
			g_uv_buffer_data    [i][0] = cursor_uv[i][0];
			g_uv_buffer_data    [i][1] = cursor_uv[i][1];
		}

		glUseProgram(programID_rendering);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLfloat) * 3, g_vertex_buffer_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			0,        // The attribute we want to configure
			3,        // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,        // stride
			(void*)0  // array buffer offset
		);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLfloat) * 2, g_uv_buffer_data, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			2,        // The attribute we want to configure
			2,        // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,        // stride
			(void*)0  // array buffer offset
		);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_cursor);
		glUniform1i(textureID_rendering, 0);
		glUniform1i(glGetUniformLocation(programID_rendering, "object"), 1);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// finish rendering
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glfwSwapBuffers(window);
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// smoothen refresh rate
		time_current = glfwGetTime();
		double time_accurate = (++frame_count) / fps;
		double time_delta = time_accurate - time_current;
		if(time_delta < 0) time_delta = 0;
		if(print) printf("frame_count:%d time_accurate:%lf time_current:%lf time_delta:%lf\n", frame_count, time_accurate, time_current, time_delta);
		usleep(time_delta * 1000000);
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// pause and continue and other events
		while(!run) {
			if(released_space && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
				released_space = 0;
				run = 1;
				glfwSetTime(time_current);
			}
			if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) released_space = 1;
			glfwPollEvents();
		}
		if(run && released_space && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			released_space = 0;
			run = 0;
		}
		if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) released_space = 1;
		glfwPollEvents();
	} while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && !glfwWindowShouldClose(window));
/*--------------------------------------------------*/



/*--------------------------------------------------*/
// clean and return
	glDeleteBuffers(1, &vertex_buffer);
	glDeleteBuffers(1, &color_buffer);
	glDeleteBuffers(1, &uv_buffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID_triangle);

	glfwTerminate();

	return 0;
/*--------------------------------------------------*/
}
