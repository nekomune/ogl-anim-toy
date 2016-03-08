
GL_FLAGS=$(shell pkg-config --cflags --libs gl glew)
SDL_FLAGS=$(shell pkg-config --cflags --libs sdl2)
OTHER_FLAGS=-lm

all:
	cc main.c $(OTHER_FLAGS) $(SDL_FLAGS) $(GL_FLAGS)

