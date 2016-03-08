#define WIDTH 640
#define HEIGHT 480

#define TBA_SDL_ERROR() fprintf(stderr, "SDL error: %s\n", SDL_GetError())
#define TBA_GLEW_ERROR(errno) fprintf(stderr, "GLEW error: %s\n", glewGetErrorString(errno))

