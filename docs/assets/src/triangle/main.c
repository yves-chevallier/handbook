#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

const int width = 800, height = 400;

const char* vert_src =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 ourColor;\n"
    "uniform float aspectRatio;\n"
    "void main() {\n"
    "    vec3 scaledPos = aPos;\n"
    "    scaledPos.x /= aspectRatio;\n"  // Appliquer la transformation
    "    gl_Position = vec4(scaledPos, 1.0);\n"
    "    ourColor = aColor;\n"
    "}\n";

const char* frag_src =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 ourColor;\n"
    "void main() { FragColor = vec4(ourColor, 1.0); }\n";

int compileShader(const char* source, GLenum type) {
   unsigned int shader = glCreateShader(type);
   glShaderSource(shader, 1, &source, NULL);
   glCompileShader(shader);

   int success;
   glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
   if (!success) {
      char infoLog[512];
      glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
      printf("Erreur de compilation du shader : %s\n", infoLog);
      return -1;
   }
   return shader;
}

unsigned int createShaderProgram(const char* vert_src, const char* frag_src) {
   unsigned int vertexShader = compileShader(vert_src, GL_VERTEX_SHADER);
   unsigned int fragmentShader = compileShader(frag_src, GL_FRAGMENT_SHADER);

   // Create Shader Program
   unsigned int shaderProgram = glCreateProgram();
   glAttachShader(shaderProgram, vertexShader);
   glAttachShader(shaderProgram, fragmentShader);
   glLinkProgram(shaderProgram);

   // Verify Link
   int success;
   glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
   if (!success) {
      char infoLog[512];
      glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
      printf("Erreur de linking du programme shader : %s\n", infoLog);
   }

   glDeleteShader(vertexShader);
   glDeleteShader(fragmentShader);

   return shaderProgram;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
   glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action,
                  int mods) {
   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(int argc, char* argv[]) {
   // Init GLFW
   int isInitiated = glfwInit();
   if (!isInitiated) {
      fprintf(stderr, "Failed to init GLFW\n");
      exit(-1);
   }

   // Set desired OpenGL version and profile
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   // Create window
   GLFWwindow* window = glfwCreateWindow(width, height, "Triangle", NULL, NULL);
   if (window == NULL) {
      fprintf(stderr, "Failed to create window\n");
      glfwTerminate();
      exit(-1);
   }
   glfwMakeContextCurrent(window);
   glfwSetKeyCallback(window, key_callback);  // When a key is pressed
   glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
   glfwSwapInterval(1);  // Enable vsync

   // Init GLEW for OpenGL context
   GLenum err = glewInit();
   if (err != GLEW_OK) {
      fprintf(stderr, "Failed to init glew\n");
      glfwTerminate();
      return -1;
   }

   unsigned int shaderProgram = createShaderProgram(vert_src, frag_src);

   // Define triangle vertices
   const float vertices[] = {
       // positions         // colors
       0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom right
       -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,  // bottom left
       0.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f   // top
   };

   // Load data into OpenGL
   unsigned int VBO = 0, VAO = 0;
   glGenVertexArrays(1, &VAO);
   glGenBuffers(1, &VBO);
   glBindVertexArray(VAO);
   glBindBuffer(GL_ARRAY_BUFFER, VBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

   // Position attribute
   const size_t stride = 6 * sizeof(float);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
   glEnableVertexAttribArray(0);

   // Color attribute
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                         (void*)(3 * sizeof(float)));
   glEnableVertexAttribArray(1);
   int aspectRatioLocation = glGetUniformLocation(shaderProgram, "aspectRatio");
   while (!glfwWindowShouldClose(window)) {
      glClearColor(.0f, .0f, .0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      glUseProgram(shaderProgram);
      glBindVertexArray(VAO);
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      float aspectRatio = (float)width / (float)height;
      glUniform1f(aspectRatioLocation, aspectRatio);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      glfwSwapBuffers(window);
      glfwPollEvents();
   }
   glDeleteVertexArrays(1, &VAO);
   glDeleteBuffers(1, &VBO);
   glfwDestroyWindow(window);
   glfwTerminate();
}
