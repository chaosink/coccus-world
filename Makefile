GLFW = `pkg-config --cflags glfw3` `pkg-config --libs --static glfw3`
GLEW = `pkg-config --cflags glew` `pkg-config --libs glew`
LIB = -lSOIL -lasound

COMMON = shader.cpp texture.cpp play_wav.cpp
TARGET = coccus-world

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	clang++ $(TARGET).cpp -o $(TARGET) $(COMMON) $(GLFW) $(GLEW) $(LIB)
