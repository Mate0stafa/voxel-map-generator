#include "Lib/Glad/include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Resources/Classes/Shader.h"
#include "Resources/Classes/Camera.h"
#include "Resources/Classes/World.h"
#include "Resources/Classes/WorldGeneration.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>

void glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                    GLsizei length, const GLchar* message, const void* userParam) {
    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        std::cerr << "OpenGL ERROR: " << message << std::endl;
    }
}

int main() {
    // 1. Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 2. Configure GLFW window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // 3. Create Window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Voxel Engine", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Disable V-Sync for uncapped FPS measurement
    glfwSwapInterval(0);

    // 4. Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set initial viewport (it is 0x0 by default until you do this)
    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);

    glEnable(GL_DEPTH_TEST);
    // Enable GPU backface culling to skip triangles facing away from the camera
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);



    // 6. Initialize engine components
    try {
        Shader shader("Resources/Shaders/vertex.glsl", "Resources/Shaders/fragment.glsl");
        World world;
        Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

        // Provide camera to callbacks and enable mouse-look
        glfwSetWindowUserPointer(window, &camera);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
            static bool firstMouse = true;
            static double lastX = xpos, lastY = ypos;
            if (firstMouse) { firstMouse = false; lastX = xpos; lastY = ypos; return; }
            double xoffset = xpos - lastX;
            double yoffset = lastY - ypos; // invert Y
            lastX = xpos; lastY = ypos;
            if (auto cam = static_cast<Camera*>(glfwGetWindowUserPointer(window))) {
                cam->ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
            }
        });

        // Set up viewport callback
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
        });

        // 7. Main render loop
        while (!glfwWindowShouldClose(window)) {
            // Input
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, true);
            }

            // Clear screen
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Update and render
            world.update(camera.Position);
            float currentFrame = glfwGetTime();
            static float lastFrame = 0.0f;
            static int frames = 0;
            static float fpsTimer = 0.0f;
            static float noiseTime = 0.0f;
            float deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // Advance noise time slowly (terrain evolves conceptually)
            noiseTime += deltaTime * 0.25f;

            // FPS counter
            frames++;
            fpsTimer += deltaTime;
            if (fpsTimer >= 1.0f) {
                int fps = frames;
                frames = 0;
                fpsTimer = 0.0f;
                std::string title = "Voxel Engine - FPS: " + std::to_string(fps);
                glfwSetWindowTitle(window, title.c_str());
            }

            // Process keyboard input for camera movement
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);

            // Regenerate chunks using current noise state when 'R' is pressed
            static bool regenPressedLast = false;
            bool regenPressed = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);
            if (regenPressed && !regenPressedLast) {
                WorldGeneration::setAnimationTime(noiseTime);
                world.regenerateAllChunks();
            }
            regenPressedLast = regenPressed;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
                camera.ProcessKeyboard(Camera_Movement::UP, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
                camera.ProcessKeyboard(Camera_Movement::DOWN, deltaTime);

            // Create projection matrix from current framebuffer size
            int fbw = 800, fbh = 600;
            glfwGetFramebufferSize(window, &fbw, &fbh);
            float aspect = (fbh > 0) ? (static_cast<float>(fbw) / static_cast<float>(fbh)) : (800.0f/600.0f);
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 500.0f);

            // Get view matrix from camera
            glm::mat4 view = camera.GetViewMatrix();

            // Use shader and pass matrices as uniforms
            shader.use();
            shader.setMat4("projection", projection);
            shader.setMat4("view", view);
            shader.setMat4("model", glm::mat4(1.0f));

            // Now render world
            world.render();

            // Swap buffers
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwTerminate();
    return 0;
}