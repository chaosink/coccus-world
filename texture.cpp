#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

GLuint LoadTexture(const char * image_name, int *image_width, int *image_height, int force_channels) {
	unsigned char *image_buffer;
	int image_channels;
	int image_width_d, image_height_d;

	GLuint texture;
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);
	image_buffer = SOIL_load_image(image_name, &image_width_d, &image_height_d, &image_channels, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width_d, image_height_d, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_buffer);
	SOIL_free_image_data(image_buffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	if(image_width) *image_width = image_width_d;
	if(image_height) *image_height = image_height_d;

	return texture;
}
