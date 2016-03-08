#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <SDL.h>
#include <SDL_video.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "main.h"
#include "config.h"

#define radian(deg) (deg/180.0f * M_PI)

typedef float mat4[4][4];

typedef struct vec3 {
    float x;
    float y;
    float z;
} vec3;

typedef struct camera {
    vec3 pos;
    vec3 target;
    vec3 up;
} camera;

struct wav_file {
    char riff[4];
    uint32_t file_size;
    char wave[4];
    char fmt[4];
    uint32_t fmt_size;
    uint16_t format;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t total;
    uint16_t something;
    uint16_t bits_per_sample;
    char data[4];
    uint32_t data_size;
};

const int window_width = 640;
const int window_height = 480;

void
mat4_multiply(mat4 m, mat4 n)
{
    static mat4 results;
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            results[j][i] = m[0][i] * n[j][0] +
                            m[1][i] * n[j][1] +
                            m[2][i] * n[j][2] +
                            m[3][i] * n[j][3];
        }
    }

    memcpy(m, results, sizeof(results));
}

void
mat4_rotateZ(mat4 m, float radians)
{
    float cosr = cos(radians);
    float sinr = sin(radians);

    mat4 rotMat = {
        {cosr, -sinr, 0.0, 0.0},
        {sinr, cosr, 0.0, 0.0},
        {0.0, 0.0, 1.0, 0.0},
        {0.0, 0.0, 0.0, 1.0}
    };

    mat4_multiply(m, rotMat);
}

void
mat4_rotateY(mat4 m, float radians)
{
    float cosr = cos(radians);
    float sinr = sin(radians);

    mat4 rotMat = {
        {cosr, 0.0, -sinr, 0.0},
        {0.0, 1.0, 0.0, 0.0},
        {sinr, 0.0, cosr, 0.0},
        {0.0, 0.0, 0.0, 1.0}
    };

    mat4_multiply(m, rotMat);
}

void
mat4_rotateX(mat4 m, float radians)
{
    float cosr = cos(radians);
    float sinr = sin(radians);

    mat4 rotMat = {
        {1.0, 0.0, 0.0, 0.0},
        {0.0, cosr, sinr, 0.0},
        {0.0, -sinr, cosr, 0.0},
        {0.0, 0.0, 0.0, 1.0}
    };

    mat4_multiply(m, rotMat);
}

void
mat4_scale(mat4 m, float x, float y, float z)
{
    mat4 scaleMat = {
        {x, 0.0, 0.0, 0.0},
        {0.0, y, 0.0, 0.0},
        {0.0, 0.0, z, 0.0},
        {0.0, 0.0, 0.0, 1.0}
    };

    mat4_multiply(m, scaleMat);
}

void
mat4_translate(mat4 m, float x, float y, float z)
{
    mat4 transMat = {
        {1.0, 0.0, 0.0, x},
        {0.0, 1.0, 0.0, y},
        {0.0, 0.0, 1.0, z},
        {0.0, 0.0, 0.0, 1.0}
    };

    mat4_multiply(m, transMat);
}

void
perspective_mat(mat4 m,
                float fov,
                float width,
                float height,
                float znear,
                float zfar)
{
    float ar = width / height;
    float zrange = zfar - znear;
    float tanfov = tanf(radian(fov) / 2.0);

    m[0][0] = 1.0f / (tanfov * ar);
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;

    m[1][0] = 0.0f;
    m[1][1] = 1.0f / tanfov;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;

    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = -(znear + zfar) / zrange;
    m[2][3] = -1.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = -2.0f * zfar * znear / zrange;
    m[3][3] = 0.0f;
}

void
vec3_cross(vec3 *result, vec3 *v, vec3 *w)
{
    result->x = v->y * w->z - v->z * w->y;
    result->y = v->z * w->x - v->x * w->z;
    result->z = v->x * w->y - v->y * w->x;
}

void
vec3_normalize(vec3 v)
{
    float mag = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

    v.x /= mag;
    v.y /= mag;
    v.z /= mag;
}

void
cam_mat4(mat4 m, struct camera *c)
{
}

int
main(int argc, char **argv)
{
    SDL_Window *w;
    SDL_GLContext glc;

    /* Init */

    if (SDL_Init(SDL_INIT_VIDEO))
        TBA_SDL_ERROR();

    w = SDL_CreateWindow("",
                         SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED,
                         window_width,
                         window_height,
                         SDL_WINDOW_OPENGL);
    if (!w)
        TBA_SDL_ERROR();

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    glc = SDL_GL_CreateContext(w);

    glewExperimental = GL_TRUE;

    GLenum glewError;
    if ((glewError = glewInit()) != GLEW_OK)
        TBA_GLEW_ERROR(glewError);

    /* Main */

    GLuint vbo, ibo, vao, shader_frag, shader_vert, program,
           attrib;

    const GLchar *source_vert =
        "#version 410\n"
        "layout (location = 0) in vec3 vert;\n"
        "uniform mat4 model;\n"
        "uniform mat4 camera;\n"
        "void main() {\n"
            "gl_Position = camera * model * vec4(vert, 1.0);\n"
        "}\n";

    const GLchar *source_frag =
        "#version 410\n"
        "out vec4 LFragment;\n"
        "void main() {\n"
            "LFragment = vec4(0.709804, 0.415686, 0.831373, 1.0);\n"
        "}\n";

    program = glCreateProgram();

    GLint success;
    shader_vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader_vert, 1, &source_vert, NULL);
    glCompileShader(shader_vert);
    glGetShaderiv(shader_vert, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        printf("Could not compile shader %d\n", shader_vert);
    }
    glAttachShader(program, shader_vert);

    shader_frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader_frag, 1, &source_frag, NULL);
    glCompileShader(shader_frag);
    glGetShaderiv(shader_frag, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        printf("Could not compile shader %d\n", shader_frag);
    }
    glAttachShader(program, shader_frag);

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        printf("Could not link program %d\n", program);
    }

    glUseProgram(program);

    GLfloat cube_vertices[] = {
        1.000000, -1.000000, -1.000000,
        1.000000, -1.000000, 1.000000,
        -1.000000, -1.000000, 1.000000,
        -1.000000, -1.000000, -1.000000,
        1.000000, 1.000000, -0.999999,
        0.999999, 1.000000, 1.000001,
        -1.000000, 1.000000, 1.000000,
        -1.000000, 1.000000, -1.000000,
    };

    GLuint cube_indices[] = {
        1, 2, 3,
        7, 6, 5,
        4, 5, 1,
        5, 6, 2,
        2, 6, 7,
        0, 3, 7,
        0, 1, 3,
        4, 7, 5,
        0, 4, 1,
        1, 5, 2,
        3, 2, 7,
        4, 0, 7,
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(GLfloat), 0);

    GLuint idModel = glGetUniformLocation(program, "model");
    GLuint idCamera = glGetUniformLocation(program, "camera");

    glClearColor(0, 0, 0, 0);
    glClearDepth(1.0f);

    SDL_Event ev;
    for (;;) {
        if (SDL_PollEvent(&ev))
            if (ev.type == SDL_QUIT)
                break;

        unsigned int ticks = SDL_GetTicks();
        float t = ticks / 100.0f;
        float tsin = sin(t);
        float tcos = cos(t);

        glClear(GL_COLOR_BUFFER_BIT);

        mat4 matPerspective;
        perspective_mat(matPerspective, 30.0f, window_width, window_height, 1.0f, 100.0f);

        mat4 matCamera = {
            {1.0, 0.0, 0.0, 0.0},
            {0.0, 1.0, 0.0, 0.0},
            {0.0, 0.0, 1.0, 0.0},
            {0.0, 0.0, 0.0, 1.0}
        };
        mat4_rotateY(matCamera, -M_PI * t / 60.0f);
        mat4_rotateX(matCamera, -M_PI/6);
        mat4_translate(matCamera, 0.0f, 0.0f, -10.0f);

        mat4_multiply(matCamera, matPerspective);

        glUniformMatrix4fv(idCamera, 1, GL_TRUE, &matCamera[0][0]);

        // 100 cubes!
        for (float x = -5.0; x < 5.0; x += 0.2) {
            for (float y = -5.0; y < 5.0; y += 0.2) {
                mat4 matModel = {
                    {1.0, 0.0, 0.0, 0.0},
                    {0.0, 1.0, 0.0, 0.0},
                    {0.0, 0.0, 1.0, 0.0},
                    {0.0, 0.0, 0.0, 1.0}
                };
                mat4_scale(matModel, 0.09, 0.09, 0.09);
                mat4_translate(matModel, 0.0, 0.09, 0.0);
                mat4_scale(matModel, 1.0, 2*(1.0 + sin(y + x + t)), 1.0);
                mat4_translate(matModel, x, 0.0, y);

                glUniformMatrix4fv(idModel, 1, GL_TRUE, (GLfloat *)matModel);
                glDrawElements(GL_TRIANGLES, 33, GL_UNSIGNED_INT, 0);

            }
        }
        SDL_GL_SwapWindow(w);
    }

    /* Clean up */
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);

    glUseProgram(0);

    glDetachShader(program, shader_vert);
    glDetachShader(program, shader_frag);
    glDeleteShader(shader_vert);
    glDeleteShader(shader_frag);

    glDeleteProgram(program);

    SDL_GL_DeleteContext(glc);
    SDL_DestroyWindow(w);
    SDL_Quit();
    return 0;
}

