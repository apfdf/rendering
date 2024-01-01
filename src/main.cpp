
#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <thread>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define BUFFER_SIZE 512
#define PI 3.14159265359

using namespace std;

GLint compileShader(const char* filename, GLenum type) {

    FILE* file = fopen(filename, "rb");

    if (file == NULL) {
        std::cerr << "Cannot open shader " << filename << std::endl;
        abort();
    }

    fseek(file, 0, SEEK_END);
    const int size = ftell(file);
    rewind(file);

    const GLchar* source = new GLchar[size+1];
    fread(const_cast<char*>(source), sizeof(char), size, file);
    const_cast<char&>(source[size]) = '\0';

    const GLint shader = glCreateShader(type);

    if (not shader) {
        std::cerr << "Cannot create a shader of type " << shader << std::endl;
        abort();
    }

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    {
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (not compiled) {
            std::cerr << "Cannot compile shader " << filename << std::endl;
            abort();
        }
    }

    return shader;

}

GLint compileShaderProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename) {

    const GLint program = glCreateProgram();

    if (not program) {
        std::cerr << "Cannot create a shader program" << std::endl;
        abort();
    }

    glAttachShader(program, compileShader(vertexShaderFilename, GL_VERTEX_SHADER));
    glAttachShader(program, compileShader(fragmentShaderFilename, GL_FRAGMENT_SHADER));

    glLinkProgram(program);

    {
        GLint linked;
        glGetShaderiv(program, GL_LINK_STATUS, &linked);
        if (not linked) {
            std::cerr << "Cannot link shader program with shaders " << vertexShaderFilename << " and " << fragmentShaderFilename << std::endl;
            abort();
        }
    }

    return program;

}

struct Camera {
    glm::vec3 p;
    float yaw;
    float pitch;
    float v;
    float rotate_v;
};


int main() {

    srand(time(NULL));

    if (!glfwInit()) {
        cerr << "GLFW init failed." << endl;
        abort();
    }

    int ww = 800;
    int wh = 800;
    GLFWwindow* window;
    window = glfwCreateWindow(ww, wh, "this is a window", NULL, NULL);
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        cerr << "GLEW init failed." << endl;
        abort();
    }

    const GLint shader_program = compileShaderProgram("/home/mans/Documents/episk programmering/portal/src/vertex.glsl", "/home/mans/Documents/episk programmering/portal/src/fragment.glsl");
    glUseProgram(shader_program);

    unordered_map<const char*, GLuint> uniform_locations;
    uniform_locations["vertex_amount"] = glGetUniformLocation(shader_program, "vertex_amount");
    uniform_locations["vertex_buffer"] = glGetUniformLocation(shader_program, "vertex_buffer");
    uniform_locations["z0"] = glGetUniformLocation(shader_program, "z0");
    uniform_locations["view_mat"] = glGetUniformLocation(shader_program, "view_mat");
    uniform_locations["light_amount"] = glGetUniformLocation(shader_program, "light_amount");
    uniform_locations["light_buffer"] = glGetUniformLocation(shader_program, "light_buffer");
    uniform_locations["cam_p"] = glGetUniformLocation(shader_program, "cam_p");
    uniform_locations["light_pos"] = glGetUniformLocation(shader_program, "light_pos");
    

    vector<vector<float>> height_map(8, vector<float>(8));
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            height_map[i][j] = ((float)rand() / RAND_MAX);
        }
    }

    
    vector<glm::vec3> vertices = {};
    for (int i = 0; i < 8-1; i++) {
        for (int j = 0; j < 8-1; j++) {
            vertices.insert(vertices.end(), {

                {i, height_map[i][j], j},
                {i+1, height_map[i+1][j], j},
                {i+1, height_map[i+1][j+1], j+1},

                {i, height_map[i][j], j},
                {i, height_map[i][j+1], j+1},
                {i+1, height_map[i+1][j+1], j+1}

            });
        }
    }
    

    /*
    vector<glm::vec3> vertices = {

        {5.0f, -2.0f, 5.0f},
        {5.0f, -2.0f, -5.0f},
        {-5.0f, -2.0f, -5.0f},

        {5.0f, -2.0f, 5.0f},
        {-5.0f, -2.0f, 5.0f},
        {-5.0f, -2.0f, -5.0f},

        {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 1.0f},

        {3.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {2.0f, 0.0f, 1.0f}

    };
    */
    

    glm::vec3 vertex_buffer[BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE; i++) {
        vertex_buffer[i] = glm::vec3(0.0f, 0.0f, 0.0f);
    }
    for (int i = 0; i < vertices.size(); i++) {
        vertex_buffer[i] = vertices[i];
    }

    vector<glm::vec3> lights = {
        {2.0f, 2.0f, 2.0f}
    };
    glm::vec3 light_buffer[BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE; i++) {
        light_buffer[i] = glm::vec3(0.0f, 0.0f, 0.0f);
    }
    for (int i = 0; i < lights.size(); i++) {
        light_buffer[i] = lights[i];
    }

    float fov = 0.75 * PI;
    float z0 = cos(fov/2) / sin(fov/2);

    glUniform1i(uniform_locations["vertex_amount"], vertices.size());
    glUniform3fv(uniform_locations["vertex_buffer"], BUFFER_SIZE, glm::value_ptr(vertex_buffer[0]));
    glUniform3f(uniform_locations["light_pos"], lights[0].x, lights[0].y, lights[0].z);
    glUniform1i(uniform_locations["light_amount"], lights.size());
    glUniform3fv(uniform_locations["light_buffer"], BUFFER_SIZE, glm::value_ptr(light_buffer[0]));
    glUniform1f(uniform_locations["z0"], z0);

    Camera cam = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        0.0f,
        0.0f,
        1.0f,
        PI
    };


    double time_when_fps = glfwGetTime();
    int updates_since_fps = 0;
    float dt = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        
        float time_start = glfwGetTime();

        glfwPollEvents();

        glm::mat4 forward_mat = glm::rotate(glm::mat4(1.0f), cam.yaw + (float)(PI / 2), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 sideways_mat = glm::rotate(glm::mat4(1.0f), cam.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec4 unit_forward_4 = forward_mat * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec4 unit_sideways_4 = sideways_mat * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 unit_forward = glm::vec3(unit_forward_4.x, unit_forward_4.y, unit_forward_4.z);
        glm::vec3 unit_sideways = glm::vec3(unit_sideways_4.x, unit_sideways_4.y, unit_sideways_4.z);

        if (glfwGetKey(window, GLFW_KEY_W)) {
            cam.p -= dt * cam.v * unit_forward;
        }
        if (glfwGetKey(window, GLFW_KEY_S)) {
            cam.p += dt * cam.v * unit_forward;
        }
        if (glfwGetKey(window, GLFW_KEY_A)) {
            cam.p -= dt * cam.v * unit_sideways;
        }
        if (glfwGetKey(window, GLFW_KEY_D)) {
            cam.p += dt * cam.v * unit_sideways;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE)) {
            cam.p += dt * cam.v * glm::vec3(0.0f, 1.0f, 0.0f);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
            cam.p -= dt * cam.v * glm::vec3(0.0f, 1.0f, 0.0f);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT)) {
            cam.yaw -= dt * cam.rotate_v;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
            cam.yaw += dt * cam.rotate_v;
        }
        if (glfwGetKey(window, GLFW_KEY_UP)) {
            cam.pitch -= dt * cam.rotate_v;
            if (cam.pitch < -(PI / 2)) {
                cam.pitch = -(PI / 2);
            }
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN)) {
            cam.pitch += dt * cam.rotate_v;
            if (cam.pitch > PI / 2) {
                cam.pitch = PI / 2;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            break;
        }

        glm::mat4 view_mat = glm::mat4(1.0f);
        view_mat = glm::rotate(view_mat, -cam.pitch, glm::vec3(1.0f, 0.0f, 0.0f));
        view_mat = glm::rotate(view_mat, -cam.yaw, glm::vec3(0.0f, 1.0f, 0.0f));

        for (int i = 0; i < vertices.size(); i++) {
            glm::vec4 v4 = view_mat * glm::vec4(vertices[i] - cam.p, 0.0f);
            vertex_buffer[i] = glm::vec3(v4.x, v4.y, v4.z);
        }
        for (int i = 0; i < lights.size(); i++) {
            glm::vec4 l4 = view_mat * glm::vec4(lights[i] - cam.p, 0.0f);
            light_buffer[i] = glm::vec3(l4.x, l4.y, l4.z);
        }

        glm::vec4 light_pos = view_mat * glm::vec4(lights[0] - cam.p, 0.0f);

        glUniform3fv(uniform_locations["vertex_buffer"], BUFFER_SIZE, glm::value_ptr(vertex_buffer[0]));
        glUniform3f(uniform_locations["light_pos"], light_buffer[0].x, light_buffer[0].y, light_buffer[0].z);

        /*
        glm::mat4 view_mat = glm::mat4(1.0f);
        view_mat = glm::rotate(view_mat, -cam.pitch, glm::vec3(1.0f, 0.0f, 0.0f));
        view_mat = glm::rotate(view_mat, -cam.yaw, glm::vec3(0.0f, 1.0f, 0.0f));

        glUniformMatrix4fv(uniform_locations["view_mat"], 1, false, glm::value_ptr(view_mat));
        glUniform3f(uniform_locations["cam_p"], cam.p.x, cam.p.y, cam.p.z);
        */

        glClearColor(255, 255, 255, 255);
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_TRIANGLES);

        glVertex3f(-1.0f, -1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 0.0f);
        glVertex3f(1.0f, 1.0f, 0.0f);
        
        glVertex3f(-1.0f, -1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 0.0f);
        glVertex3f(1.0f, 1.0f, 0.0f);
        
        glEnd();
        
        glfwSwapBuffers(window);

        updates_since_fps++;
        if (glfwGetTime() - time_when_fps >= 1.0) {
            cout << "FPS: " << floor(updates_since_fps / (glfwGetTime() - time_when_fps)) << endl;
            time_when_fps = glfwGetTime();
            updates_since_fps = 0;
        }

        dt = glfwGetTime() - time_start;

    }

    glfwTerminate();

}
