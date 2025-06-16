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
Kolizje z granicami okna
```cpp
// main.cpp
// Prosta symulacja spadającego i odbijającego się koła z kolizją z krawędziami okna

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>

void drawCircle(float centerX, float centerY, float radius, int res) {
    glBegin(GL_TRIANGLE_FAN);
    // środek koła
    glVertex2f(centerX, centerY);
    // obwód koła
    for (int i = 0; i <= res; ++i) {
        float angle = 2.0f * 3.14159265359f * i / res;
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
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Spadające koło", nullptr, nullptr);
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

    // Ustaw viewport i orto-projekcję w pikselach
    glViewport(0, 0, screenWidth, screenHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    // Stan fizyczny koła
    std::vector<float> pos = { screenWidth * 0.5f, screenHeight * 0.5f }; // start w środku
    std::vector<float> vel = { 50.0f,           0.0f };       // prędkość początkowa
    const float radius = 50.0f;
    const int   res = 64;

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        // --- Obliczenia fizyki ---
        float now = glfwGetTime();
        float dt = now - lastTime;
        lastTime = now;

        // grawitacja w dół
        vel[1] -= 9.81f * dt;

        // integracja ruchu
        pos[0] += vel[0] * dt;
        pos[1] += vel[1] * dt;

        // kolizja z dolną krawędzią
        if (pos[1] - radius < 0.0f) {
            pos[1] = radius;
            vel[1] = -vel[1] * 0.95f;
        }
        // kolizja z górną krawędzią
        if (pos[1] + radius > screenHeight) {
            pos[1] = screenHeight - radius;
            vel[1] = -vel[1] * 0.95f;
        }
        // kolizja z lewą krawędzią
        if (pos[0] - radius < 0.0f) {
            pos[0] = radius;
            vel[0] = -vel[0] * 0.95f;
        }
        // kolizja z prawą krawędzią
        if (pos[0] + radius > screenWidth) {
            pos[0] = screenWidth - radius;
            vel[0] = -vel[0] * 0.95f;
        }

        // --- Render ---
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity(); // zresetuj macierz modelu

        drawCircle(pos[0], pos[1], radius, res);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

```

Przyspieszenie czasu
```cpp
// main.cpp
// Spadające i odbijające się koło – przyspieszona symulacja

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>

void drawCircle(float centerX, float centerY, float radius, int res) {
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(centerX, centerY);
        for (int i = 0; i <= res; ++i) {
            float angle = 2.0f * 3.14159265359f * i / res;
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
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Spadające koło", nullptr, nullptr);
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

    // Ustaw viewport i ortho-projekcję w pikselach
    glViewport(0, 0, screenWidth, screenHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    // Stan fizyczny koła
    std::vector<float> pos = { screenWidth * 0.5f, screenHeight * 0.8f }; // start bliżej góry
    std::vector<float> vel = {  0.0f,            0.0f            };       // początkowo bez prędkości
    const float radius = 50.0f;
    const int   res    = 64;

    const float TIME_SCALE = 2.0f;   // przyspieszenie symulacji x2
    const float GRAVITY    = 9.81f;  // przyspieszenie ziemskie

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        // --- Obliczenia fizyki ---
        float now = glfwGetTime();
        float dt  = (now - lastTime) * TIME_SCALE;
        lastTime = now;

        // grawitacja
        vel[1] -= GRAVITY * dt;

        // integracja ruchu
        pos[0] += vel[0] * dt;
        pos[1] += vel[1] * dt;

        // kolizja z dolną krawędzią
        if (pos[1] - radius < 0.0f) {
            pos[1] = radius;
            vel[1] = -vel[1] * 0.95f;
        }
        // kolizja z górną krawędzią
        if (pos[1] + radius > screenHeight) {
            pos[1] = screenHeight - radius;
            vel[1] = -vel[1] * 0.95f;
        }
        // kolizja z lewą krawędzią
        if (pos[0] - radius < 0.0f) {
            pos[0] = radius;
            vel[0] = -vel[0] * 0.95f;
        }
        // kolizja z prawą krawędzią
        if (pos[0] + radius > screenWidth) {
            pos[0] = screenWidth - radius;
            vel[0] = -vel[0] * 0.95f;
        }

        // --- Render ---
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();

        drawCircle(pos[0], pos[1], radius, res);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

```
Przyspieszenie po X
```cpp
// main.cpp
// Spadające i odbijające się koło z akceleracją w poziomie

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>

void drawCircle(float cx, float cy, float r, int res) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= res; ++i) {
        float θ = 2.0f * 3.14159265359f * i / res;
        glVertex2f(cx + r * std::cos(θ),
            cy + r * std::sin(θ));
    }
    glEnd();
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Nie udało się zainicjalizować GLFW\n";
        return -1;
    }

    const int W = 800, H = 600;
    GLFWwindow* window = glfwCreateWindow(W, H, "Koło z grawitacją i akcel. poziomą", nullptr, nullptr);
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

    // Ortograficzna projekcja 2D (piksele jako jednostki)
    glViewport(0, 0, W, H);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, W, 0, H, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    // Stan fizyczny
    std::vector<float> pos = { W * 0.5f, H * 0.8f };
    std::vector<float> vel = { 0.0f,        0.0f };
    const float radius = 50.0f;
    const int   resolution = 64;

    // Stałe symulacji
    const float TIME_SCALE = 7.0f;    // przyspieszenie dt
    const float GRAVITY = 9.81f;   // grawitacja w dół
    const float ACCEL_X = 20.0f;   // pozioma stała akceleracja (np. "wiatr")

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        // --- Fizyka ---
        float now = glfwGetTime();
        float dt = (now - lastTime) * TIME_SCALE;
        lastTime = now;

        // pionowe przyspieszenie grawitacyjne
        vel[1] -= GRAVITY * dt;
        // poziome przyspieszenie
        vel[0] += ACCEL_X * dt;

        // integracja położeń
        pos[0] += vel[0] * dt;
        pos[1] += vel[1] * dt;

        // Odbicia od krawędzi
        // dół
        if (pos[1] - radius < 0.0f) {
            pos[1] = radius;
            vel[1] = -vel[1] * 0.95f;
        }
        // góra
        if (pos[1] + radius > H) {
            pos[1] = H - radius;
            vel[1] = -vel[1] * 0.95f;
        }
        // lewo
        if (pos[0] - radius < 0.0f) {
            pos[0] = radius;
            vel[0] = -vel[0] * 0.95f;
        }
        // prawo
        if (pos[0] + radius > W) {
            pos[0] = W - radius;
            vel[0] = -vel[0] * 0.95f;
        }

        // --- Render ---
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        drawCircle(pos[0], pos[1], radius, resolution);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

```
