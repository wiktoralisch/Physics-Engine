Generowanie Koła
```cpp
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

int main() {
    if (!glfwInit()) {
        std::cerr << "Nie udało się zainicjalizować GLFW\n";
        return -1;
    }

    const int screenWidth  = 800;
    const int screenHeight = 600;

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Test OpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "Nie udało się utworzyć okna GLFW\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Nie udało się zainicjalizować GLEW\n";
        return -1;
    }

    // ustaw viewport
    glViewport(0, 0, screenWidth, screenHeight);

    // --- ORTOGRAFICZNA PROJEKCJA W PIKSELACH ---
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // left, right, bottom, top, near, far
    glOrtho(0.0,                 screenWidth,
            0.0,                 screenHeight,
           -1.0,                  1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // -------------------------------------------

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    // parametry koła w pikselach
    float centerX = screenWidth  / 2.0f;
    float centerY = screenHeight / 2.0f;
    float radius  = 50.0f;
    int   res     = 100;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_TRIANGLE_FAN);
            // środek koła
            glVertex2f(centerX, centerY);
            // obwód
            for (int i = 0; i <= res; ++i) {
                float angle = 2.0f * 3.1415926f * i / res;
                float x = centerX + radius * std::cos(angle);
                float y = centerY + radius * std::sin(angle);
                glVertex2f(x, y);
            }
        glEnd();

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
```

Ruch Kuli 2D w dół
```cpp
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>

void drawCircle(float centerX, float centerY, float radius, int res) {
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(centerX, centerY);
        for (int i = 0; i <= res; ++i) {
            float angle = 2.0f * 3.1415926f * i / res;
            float x = centerX + radius * std::cos(angle);
            float y = centerY + radius * std::sin(angle);
            glVertex2f(x, y);
        }
    glEnd();
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Nie udało się zainicjalizować GLFW\n";
        return -1;
    }

    const int screenWidth  = 800;
    const int screenHeight = 600;
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Test OpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "Nie udało się utworzyć okna GLFW\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Nie udało się zainicjalizować GLEW\n";
        return -1;
    }

    glViewport(0, 0, screenWidth, screenHeight);

    // ortho w pikselach
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    std::vector<float> pos = {400.0f, 300.0f};
    const float radius = 50.0f;
    const int   res    = 100;
    float speed        = 100.0f; // px na sekundę

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float now = glfwGetTime();
        float dt  = now - lastTime;
        lastTime  = now;

        // ruszamy w dół
        pos[1] -= speed * dt;

        glClear(GL_COLOR_BUFFER_BIT);

        // reset modelu
        glLoadIdentity();

        // rysuj
        drawCircle(pos[0], pos[1], radius, res);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

```
Grawitacja w 2D
```cpp
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>

void drawCircle(float centerX, float centerY, float radius, int res) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(centerX, centerY);
    for (int i = 0; i <= res; ++i) {
        float angle = 2.0f * 3.1415926f * i / res;
        float x = centerX + radius * std::cos(angle);
        float y = centerY + radius * std::sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Nie udało się zainicjalizować GLFW\n";
        return -1;
    }

    const int screenWidth = 800;
    const int screenHeight = 600;
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Test OpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "Nie udało się utworzyć okna GLFW\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Nie udało się zainicjalizować GLEW\n";
        return -1;
    }

    glViewport(0, 0, screenWidth, screenHeight);

    // ortho w pikselach
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    std::vector<float> pos = { 400.0f, 300.0f };
	std::vector<float> vel = { 0.0f, 0.0f };
    const float radius = 50.0f;
    const int   res = 100;
    float speed = 100.0f; // px na sekundę

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float now = glfwGetTime();
        float dt = now - lastTime;
        lastTime = now;

        // ruszamy w dół
        pos[1] -= speed * dt;

        glClear(GL_COLOR_BUFFER_BIT);

        // reset modelu
        glLoadIdentity();

        // rysuj
        drawCircle(pos[0], pos[1], radius, res);

        pos[0] += vel[0];
		pos[1] += vel[1];
        vel[1] += -9.81f * dt; // przyspieszenie grawitacyjne

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
```
