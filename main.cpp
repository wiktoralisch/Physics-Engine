// Dodanie głównych nagłówków OpenGL, GLFW i GLM
#include <GL/glew.h> // GLEW: ładowanie funkcji OpenGL
#include <GLFW/glfw3.h> // GLFW: obsługa okien i zdarzeń
#include <glm/glm.hpp> // GLM: wektory, macierze
#include <glm/gtc/matrix_transform.hpp> // GLM: funkcje transformacji
#include <glm/gtc/type_ptr.hpp> // GLM: dostęp do danych wektorów jako ciąg floatów
#include <vector> // std::vector: dynamiczna tablica
#include <iostream> // std::cout, std::cerr

// Źródło kodu shadera wierzchołków
const char *vertexShaderSource = R"glsl(
#version 330 core // Wersja GLSL
layout(location=0) in vec3 aPos; // Pozycja wierzchołka
uniform mat4 model; // Macierz modelu
uniform mat4 view; // Macierz widoku
uniform mat4 projection; // Macierz projekcji
out float lightIntensity; // Przekazywanie natężenia światła do fragment shadera
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0); // Transformacja pozycji wierzchołka
    vec3 worldPos = (model * vec4(aPos, 1.0)).xyz; // Pozycja w świecie
    vec3 normal = normalize(aPos); // Normalna (zakładana ze współrzędnych)
    vec3 dirToCenter = normalize(-worldPos); // Kierunek do środka układu
    lightIntensity = max(dot(normal, dirToCenter), 0.15); // Obliczenie natężenia
})glsl";

// Źródło kodu shadera fragmentów
const char *fragmentShaderSource = R"glsl(
#version 330 core // Wersja GLSL
in float lightIntensity; // Wejście z vertex shadera
out vec4 FragColor; // Kolor wyjściowy fragmentu
uniform vec4 objectColor; // Kolor obiektu
uniform bool isGrid; // Flaga siatki
uniform bool GLOW; // Flaga efektu poświaty
void main() {
    if (isGrid) {
        // If it's the grid, use the original color without lighting
        FragColor = objectColor; // Siatka - czysty kolor
    } else if(GLOW){
        FragColor = vec4(objectColor.rgb * 100000, objectColor.a); // Efekt glow
    }else {
        // If it's an object, apply the lighting effect
        float fade = smoothstep(0.0, 10.0, lightIntensity*10); // Wygładzanie
        FragColor = vec4(objectColor.rgb * fade, objectColor.a); // Kolor z oświetleniem
    }
})glsl";

// Flagi sterujące pętlą główną
bool running = true; // Czy kontynuować program?
bool pause = true; // Czy symulacja jest zatrzymana?

// Wektory kamery
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 1.0f); // Pozycja kamery
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Kierunek patrzenia
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); // Wektor "góry"

// Zmienne do obsługi myszki
float lastX = 400.0, lastY = 300.0; // Ostatnie współrzędne kursora
float yaw = -90; // Kąt poziomy
float pitch = 0.0; // Kąt pionowy

// Zmienne pomiaru czasu
float deltaTime = 0.0; // Czas pomiędzy klatkami
float lastFrame = 0.0; // Czas ostatniej klatki

// Stałe fizyczne
const double G = 6.6743e-11; // Stała grawitacyjna (m^3 kg^-1 s^-2)
const float c = 299792458.0; // Prędkość światła (m/s)
float initMass = float(pow(10, 22)); // Masa początkowa obiektu
float sizeRatio = 30000.0f; // Współczynnik skalowania rozmiaru

// Deklaracje funkcji pomocniczych
GLFWwindow *StartGLU(); // Inicjalizacja GLFW + GLEW\GLuint CreateShaderProgram(const char *vertexSource, const char *fragmentSource); // Kompilacja i linkowanie shaderów
void CreateVBOVAO(GLuint &VAO, GLuint &VBO, const float *vertices, size_t vertexCount); // Ustawienie VBO i VAOoid UpdateCam(GLuint shaderProgram, glm::vec3 cameraPos); // Aktualizacja macierzy widoku
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods); // Obsługa klawiatury
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods); // Obsługa przycisków myszy
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset); // Obsługa scrolla myszy

void mouse_callback(GLFWwindow *window, double xpos, double ypos); // Ruch myszy
glm::vec3 sphericalToCartesian(float r, float theta, float phi); // Konwersja współrzędnych sferycznych
void DrawGrid(GLuint shaderProgram, GLuint gridVAO, size_t vertexCount); // Rysowanie siatki

// Klasa reprezentująca obiekt fizyczny
class Object
{
public:
    GLuint VAO, VBO; // Identyfikatory VAO i VBO w OpenGL
    glm::vec3 position = glm::vec3(400, 300, 0); // Pozycja początkowa
    glm::vec3 velocity = glm::vec3(0, 0, 0); // Prędkość początkowa
    size_t vertexCount; // Ilość współrzędnych w VBO
    glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // Domyślny kolor (czerwony)

    bool Initalizing = false; // Czy w trakcie inicjalizacji
    bool Launched = false; // Czy został wystrzelony
    bool target = false; // Czy jest celem

    float mass; // Masa obiektu
    float density; // Gęstość (kg/m^3)
    float radius; // Promień obliczany z masy i gęstości

    glm::vec3 LastPos = position; // Ostatnia zapamiętana pozycja
    bool glow; // Czy ma efekt glow

    // Konstruktor inicjalizujący wszystkie pola
    Object(glm::vec3 initPosition, glm::vec3 initVelocity, float mass, float density = 3344, glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), bool Glow = false)
    {
        this->position = initPosition; // Ustaw pozycję
        this->velocity = initVelocity; // Ustaw prędkość
        this->mass = mass; // Ustaw masę
        this->density = density; // Ustaw gęstość
        this->radius = pow(((3 * this->mass / this->density) / (4 * 3.14159265359)), (1.0f / 3.0f)) / sizeRatio; // Oblicz promień
        this->color = color; // Ustaw kolor
        this->glow = Glow; // Ustaw flagę glow

        // generate vertices (centered at origin)
        std::vector<float> vertices = Draw(); // Wygeneruj wierzchołki sfery
        vertexCount = vertices.size(); // Zapamiętaj liczbę

        CreateVBOVAO(VAO, VBO, vertices.data(), vertexCount); // Utwórz VAO i VBO
    }

    // Generowanie wierzchołków sfery
    std::vector<float> Draw()
    {
        std::vector<float> vertices; // Kontener na współrzędne
        int stacks = 10; // Ilość poziomów
        int sectors = 10; // Ilość segmentów

        // generate circumference points using integer steps
        for (float i = 0.0f; i <= stacks; ++i)
        {
            float theta1 = (i / stacks) * glm::pi<float>(); // Kąt pionowy startowy
            float theta2 = (i + 1) / stacks * glm::pi<float>(); // Kąt pionowy kolejny
            for (float j = 0.0f; j < sectors; ++j)
            {
                float phi1 = j / sectors * 2 * glm::pi<float>(); // Kąt poziomy startowy
                float phi2 = (j + 1) / sectors * 2 * glm::pi<float>(); // Kąt poziomy kolejny
                glm::vec3 v1 = sphericalToCartesian(this->radius, theta1, phi1); // Punkt 1
                glm::vec3 v2 = sphericalToCartesian(this->radius, theta1, phi2); // Punkt 2
                glm::vec3 v3 = sphericalToCartesian(this->radius, theta2, phi1); // Punkt 3
                glm::vec3 v4 = sphericalToCartesian(this->radius, theta2, phi2); // Punkt 4

                // Triangle 1: v1-v2-v3
                vertices.insert(vertices.end(), {v1.x, v1.y, v1.z}); // Wierzchołek 1
                vertices.insert(vertices.end(), {v2.x, v2.y, v2.z}); // Wierzchołek 2
                vertices.insert(vertices.end(), {v3.x, v3.y, v3.z}); // Wierzchołek 3

                // Triangle 2: v2-v4-v3
                vertices.insert(vertices.end(), {v2.x, v2.y, v2.z}); // Wierzchołek 2
                vertices.insert(vertices.end(), {v4.x, v4.y, v4.z}); // Wierzchołek 4
                vertices.insert(vertices.end(), {v3.x, v3.y, v3.z}); // Wierzchołek 3
            }
        }
        return vertices; // Zwróć tablicę współrzędnych
    }

    // Aktualizacja pozycji na podstawie prędkości
    void UpdatePos()
    {
        this->position[0] += this->velocity[0] / 94; // Przesunięcie x
        this->position[1] += this->velocity[1] / 94; // Przesunięcie y
        this->position[2] += this->velocity[2] / 94; // Przesunięcie z
        this->radius = pow(((3 * this->mass / this->density) / (4 * 3.14159265359)), (1.0f / 3.0f)) / sizeRatio; // Aktualizacja promienia
    }
    // Aktualizacja bufora wierzchołków
    void UpdateVertices()
    {
        // generate new vertices with current radius
        std::vector<float> vertices = Draw(); // Wygeneruj nowe

        // update VBO with new vertex data
        glBindBuffer(GL_ARRAY_BUFFER, VBO); // Wybierz VBO
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW); // Załaduj dane
    }
    // Zwraca pozycję obiektu
    glm::vec3 GetPos() const
    {
        return this->position; // Zwróć wektor pozycji
    }
    // Dodaje przyspieszenie
    void accelerate(float x, float y, float z)
    {
        this->velocity[0] += x / 96; // Zmiana prędkości x
        this->velocity[1] += y / 96; // Zmiana prędkości y
        this->velocity[2] += z / 96; // Zmiana prędkości z
    }
    // Sprawdza kolizję z innym obiektem
    float CheckCollision(const Object &other)
    {
        float dx = other.position[0] - this->position[0]; // Różnica x
        float dy = other.position[1] - this->position[1]; // Różnica y
        float dz = other.position[2] - this->position[2]; // Różnica z
        float distance = std::pow(dx * dx + dy * dy + dz * dz, (1.0f / 2.0f)); // Odległość
        if (other.radius + this->radius > distance)
        {
            return -0.2f; // Kolizja: zmniejszenie prędkości
        }
        return 1.0f; // Brak kolizji: brak zmiany
    }
};

std::vector<Object> objs = {}; // Kontener obiektów sceny

// Deklaracje funkcji do siatki
std::vector<float> CreateGridVertices(float size, int divisions, const std::vector<Object> &objs);
std::vector<float> UpdateGridVertices(std::vector<float> vertices, const std::vector<Object> &objs);

GLuint gridVAO, gridVBO; // VAO i VBO dla siatki

int main()
{
    GLFWwindow *window = StartGLU(); // Inicjalizacja okna i kontekstu OpenGL
    GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource); // Kompilacja shaderów

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model"); // Lokalizacja uniformu model
    GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor"); // Lokalizacja uniformu color
    glUseProgram(shaderProgram); // Użycie programu shaderów

    glfwSetCursorPosCallback(window, mouse_callback); // Ustaw callback ruchu myszy
    glfwSetScrollCallback(window, scroll_callback); // Callback scroll
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Ukrycie kursora

    // projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 750000.0f); // Macierz projekcji
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection"); // Lokalizacja uniformu projection
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection)); // Przesłanie macierzy do GPU
    cameraPos = glm::vec3(0.0f, 1000.0f, 5000.0f); // Ustawienie początkowej pozycji kamery

    objs = {
        Object(glm::vec3(-5000, 650, -350), glm::vec3(0, 0, 1500), 5.97219 * pow(10, 22), 5515, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)), // Pierwszy obiekt
        Object(glm::vec3(5000, 650, -350), glm::vec3(0, 0, -1500), 5.97219 * pow(10, 22), 5515, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)), // Drugi obiekt
        Object(glm::vec3(0, 0, -350), glm::vec3(0, 0, 0), 1.989 * pow(10, 25), 5515, glm::vec4(1.0f, 0.929f, 0.176f, 1.0f), true), // Obiekt glow
    };
    std::vector<float> gridVertices = CreateGridVertices(20000.0f, 25, objs); // Generuj wierzchołki siatki
    CreateVBOVAO(gridVAO, gridVBO, gridVertices.data(), gridVertices.size()); // Utwórz VAO/VBO siatki

    while (!glfwWindowShouldClose(window) && running == true) // Główna pętla
    {
        float currentFrame = glfwGetTime(); // Pobierz czas od startu
        deltaTime = currentFrame - lastFrame; // Oblicz deltaTime
        lastFrame = currentFrame; // Zaktualizuj lastFrame

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Wyczyść bufor koloru i głębi

        glfwSetKeyCallback(window, keyCallback); // Callback klawiatury
        glfwSetMouseButtonCallback(window, mouseButtonCallback); // Callback myszy
        UpdateCam(shaderProgram, cameraPos); // Zaktualizuj widok kamery

        if (!objs.empty() && objs.back().Initalizing) // Jeśli ostatni obiekt jest inicjalizowany
        {
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) // Prawy przycisk
            {
                // increase mass by 1% per second
                objs.back().mass *= 1.0 + 1.0 * deltaTime; // Zwiększ masę
                // update radius based on new mass
                objs.back().radius = pow((3 * objs.back().mass / objs.back().density) / (4 * 3.14159265359f), 1.0f / 3.0f) / sizeRatio; // Zaktualizuj promień
                // update vertex data
                objs.back().UpdateVertices(); // Przeładuj wierzchołki
            }
        }

        // Draw the grid
        glUseProgram(shaderProgram); // Użyj programu shaderów
        glUniform4f(objectColorLoc, 1.0f, 1.0f, 1.0f, 0.25f); // Ustaw kolor siatki
        glUniform1i(glGetUniformLocation(shaderProgram, "isGrid"), 1); // Flaga siatki
        glUniform1i(glGetUniformLocation(shaderProgram, "GLOW"), 0); // Wyłącz glow
        gridVertices = UpdateGridVertices(gridVertices, objs); // Zaktualizuj wierzchołki siatki
        glBindBuffer(GL_ARRAY_BUFFER, gridVBO); // Wybierz bufor siatki
        glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_DYNAMIC_DRAW); // Załaduj nowe dane
        DrawGrid(shaderProgram, gridVAO, gridVertices.size()); // Narysuj siatkę

        // Draw the triangles / sphere
        for (auto &obj : objs) // Iteracja po obiektach
        {
            glUniform4f(objectColorLoc, obj.color.r, obj.color.g, obj.color.b, obj.color.a); // Ustaw kolor obiektu

            for (auto &obj2 : objs) // Iteracja po parach obiektów
            {
                if (&obj2 != &obj && !obj.Initalizing && !obj2.Initalizing) // Pomijaj ten sam obiekt
                {
                    float dx = obj2.GetPos()[0] - obj.GetPos()[0]; // Różnica x
                    float dy = obj2.GetPos()[1] - obj.GetPos()[1]; // Różnica y
                    float dz = obj2.GetPos()[2] - obj.GetPos()[2]; // Różnica z
                    float distance = sqrt(dx * dx + dy * dy + dz * dz); // Odległość

                    if (distance > 0) // Jeśli nie nachodzą na siebie
                    {
                        std::vector<float> direction = {dx / distance, dy / distance, dz / distance}; // Kierunek normalizowany
                        distance *= 1000; // Konwersja do metrów
                        double Gforce = (G * obj.mass * obj2.mass) / (distance * distance); // Siła grawitacji

                        float acc1 = Gforce / obj.mass; // Przyspieszenie
                        std::vector<float> acc = {direction[0] * acc1, direction[1] * acc1, direction[2] * acc1}; // Wektor przyspieszenia
                        if (!pause) // Jeśli nie pauza
                        {
                            obj.accelerate(acc[0], acc[1], acc[2]); // Zastosuj przyspieszenie
                        }

                        // collision
                        obj.velocity *= obj.CheckCollision(obj2); // Sprawdzenie i reakcja na kolizję
                        std::cout << "radius: " << obj.radius << std::endl; // Debug: promień
                    }
                }
            }
            if (obj.Initalizing) // Jeśli inicjalizacja obiektu
            {
                obj.radius = pow(((3 * obj.mass / obj.density) / (4 * 3.14159265359)), (1.0f / 3.0f)) / 1000000; // Mały promień podczas inicjalizacji
                obj.UpdateVertices(); // Przeładuj wierzchołki
            }

            // update positions
            if (!pause) // Jeśli nie pauza
            {
                obj.UpdatePos(); // Zaktualizuj pozycję
            }

            glm::mat4 model = glm::mat4(1.0f); // Identity matrix
            model = glm::translate(model, obj.position); // apply position
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Przesyłanie macierzy modelu
            glUniform1i(glGetUniformLocation(shaderProgram, "isGrid"), 0); // Wyłącz siatkę dla rysowania obiektu
            if (obj.glow) // Jeśli glow
            {
                glUniform1i(glGetUniformLocation(shaderProgram, "GLOW"), 1); // Włącz glow
            }
            else
            {
                glUniform1i(glGetUniformLocation(shaderProgram, "GLOW"), 0); // Wyłącz glow
            }

            glBindVertexArray(obj.VAO); // Wybierz VAO obiektu
            glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount / 3); // Narysuj trójkąty
        }

        glfwSwapBuffers(window); // Zamiana buforów
        glfwPollEvents(); // Obsługa zdarzeń
    }

    // Cleanup: usuwanie VAO i VBO wszystkich obiektów
    for (auto &obj : objs)
    {
        glDeleteVertexArrays(1, &obj.VAO);
        glDeleteBuffers(1, &obj.VBO);
    }

    // Cleanup siatki
    glDeleteVertexArrays(1, &gridVAO); // Usuń VAO siatki
    glDeleteBuffers(1, &gridVBO); // Usuń VBO siatki

    glDeleteProgram(shaderProgram); // Usuń program shaderów
    glfwTerminate(); // Zakończ GLFW

    return 0; // Zwróć kod wyjścia 0
}

// Funkcja inicjalizująca GLFW i GLEW oraz tworząca okno
GLFWwindow *StartGLU()
{
    if (!glfwInit()) // Inicjalizacja GLFW
    {
        std::cout << "Failed to initialize GLFW, panic" << std::endl; // Błąd
        return nullptr; // Zwróć nullptr
    }
    GLFWwindow *window = glfwCreateWindow(800, 600, "3D_TEST", NULL, NULL); // Utwórz okno
    if (!window) // Jeśli nie udało się utworzyć
    {
        std::cerr << "Failed to create GLFW window." << std::endl; // Błąd
        glfwTerminate(); // Zakończ GLFW
        return nullptr; // Zwróć nullptr
    }
    glfwMakeContextCurrent(window); // Ustaw kontekst

    glewExperimental = GL_TRUE; // Wymuś użycie nowoczesnych funkcji
    if (glewInit() != GLEW_OK) // Inicjalizacja GLEW
    {
        std::cerr << "Failed to initialize GLEW." << std::endl; // Błąd
        glfwTerminate(); // Zakończ GLFW
        return nullptr; // Zwróć nullptr
    }

    glEnable(GL_DEPTH_TEST); // Włącz test głębi
    glViewport(0, 0, 800, 600); // Ustaw viewport
    glEnable(GL_BLEND); // Włącz blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Funkcja mieszania przezroczystości

    return window; // Zwróć utworzone okno
}

// Funkcja kompilująca i linkująca shadery
GLuint CreateShaderProgram(const char *vertexSource, const char *fragmentSource)
{
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // Utwórz shader
    glShaderSource(vertexShader, 1, &vertexSource, nullptr); // Przypisz źródło
    glCompileShader(vertexShader); // Skompiluj shader

    GLint success; // Flaga powodzenia
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success); // Sprawdź status kompilacji
    if (!success) // Jeśli błąd
    {
        char infoLog[512]; // Bufor na logi
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog); // Pobierz logi
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl; // Wypisz błąd
    }

    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Utwórz shader
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr); // Przypisz źródło
    glCompileShader(fragmentShader); // Skompiluj shader

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success); // Sprawdź status kompilacji
    if (!success) // Jeśli błąd
    {
        char infoLog[512]; // Bufor na logi
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog); // Pobierz logi
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl; // Wypisz błąd
    }

    // Shader program
    GLuint shaderProgram = glCreateProgram(); // Utwórz program
    glAttachShader(shaderProgram, vertexShader); // Dołącz vertex shader
    glAttachShader(shaderProgram, fragmentShader); // Dołącz fragment shader
    glLinkProgram(shaderProgram); // Zlinkuj program

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success); // Sprawdź status linkowania
    if (!success) // Jeśli błąd
    {
        char infoLog[512]; // Bufor na logi
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog); // Pobierz logi
        std::cerr << "Shader program linking failed: " << infoLog << std::endl; // Wypisz błąd
    }

    glDeleteShader(vertexShader); // Usuń vertex shader
    glDeleteShader(fragmentShader); // Usuń fragment shader

    return shaderProgram; // Zwróć ID programu
}

void CreateVBOVAO(GLuint &VAO, GLuint &VBO, const float *vertices, size_t vertexCount)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void UpdateCam(GLuint shaderProgram, glm::vec3 cameraPos)
{
    glUseProgram(shaderProgram);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    float cameraSpeed = 10000.0f * deltaTime;
    bool shiftPressed = (mods & GLFW_MOD_SHIFT) != 0;
    Object &lastObj = objs[objs.size() - 1];

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraPos += cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraPos -= cameraSpeed * cameraFront;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        cameraPos += cameraSpeed * cameraUp;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        cameraPos -= cameraSpeed * cameraUp;
    }

    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        pause = true;
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE)
    {
        pause = false;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        glfwTerminate();
        glfwWindowShouldClose(window);
        running = false;
    }

    // init arrows pos up down left right
    if (!objs.empty() && objs[objs.size() - 1].Initalizing)
    {
        if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            if (!shiftPressed)
            {
                objs[objs.size() - 1].position[1] += objs[objs.size() - 1].radius * 0.2;
            }
        };
        if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            if (!shiftPressed)
            {
                objs[objs.size() - 1].position[1] -= objs[objs.size() - 1].radius * 0.2;
            }
        }
        if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            objs[objs.size() - 1].position[0] += objs[objs.size() - 1].radius * 0.2;
        };
        if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            objs[objs.size() - 1].position[0] -= objs[objs.size() - 1].radius * 0.2;
        };
        if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            objs[objs.size() - 1].position[2] += objs[objs.size() - 1].radius * 0.2;
        };

        if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            objs[objs.size() - 1].position[2] -= objs[objs.size() - 1].radius * 0.2;
        }
    };
};
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            objs.emplace_back(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0f, 0.0f, 0.0f), initMass);
            objs[objs.size() - 1].Initalizing = true;
        };
        if (action == GLFW_RELEASE)
        {
            objs[objs.size() - 1].Initalizing = false;
            objs[objs.size() - 1].Launched = true;
        };
    };
    if (!objs.empty() && button == GLFW_MOUSE_BUTTON_RIGHT && objs[objs.size() - 1].Initalizing)
    {
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {
            objs[objs.size() - 1].mass *= 1.2;
        }
        std::cout << "MASS: " << objs[objs.size() - 1].mass << std::endl;
    }
};
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    float cameraSpeed = 250000.0f * deltaTime;
    if (yoffset > 0)
    {
        cameraPos += cameraSpeed * cameraFront;
    }
    else if (yoffset < 0)
    {
        cameraPos -= cameraSpeed * cameraFront;
    }
}

glm::vec3 sphericalToCartesian(float r, float theta, float phi)
{
    float x = r * sin(theta) * cos(phi);
    float y = r * cos(theta);
    float z = r * sin(theta) * sin(phi);
    return glm::vec3(x, y, z);
};
void DrawGrid(GLuint shaderProgram, GLuint gridVAO, size_t vertexCount)
{
    glUseProgram(shaderProgram);
    glm::mat4 model = glm::mat4(1.0f); // Identity matrix for the grid
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(gridVAO);
    glPointSize(5.0f);
    glDrawArrays(GL_LINES, 0, vertexCount / 3);
    glBindVertexArray(0);
}
std::vector<float> CreateGridVertices(float size, int divisions, const std::vector<Object> &objs)
{
    std::vector<float> vertices;
    float step = size / divisions;
    float halfSize = size / 2.0f;

    // x axis
    for (int yStep = 3; yStep <= 3; ++yStep)
    {
        float y = -halfSize * 0.3f + yStep * step;
        for (int zStep = 0; zStep <= divisions; ++zStep)
        {
            float z = -halfSize + zStep * step;
            for (int xStep = 0; xStep < divisions; ++xStep)
            {
                float xStart = -halfSize + xStep * step;
                float xEnd = xStart + step;
                vertices.push_back(xStart);
                vertices.push_back(y);
                vertices.push_back(z);
                vertices.push_back(xEnd);
                vertices.push_back(y);
                vertices.push_back(z);
            }
        }
    }
    // zaxis
    for (int xStep = 0; xStep <= divisions; ++xStep)
    {
        float x = -halfSize + xStep * step;
        for (int yStep = 3; yStep <= 3; ++yStep)
        {
            float y = -halfSize * 0.3f + yStep * step;
            for (int zStep = 0; zStep < divisions; ++zStep)
            {
                float zStart = -halfSize + zStep * step;
                float zEnd = zStart + step;
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(zStart);
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(zEnd);
            }
        }
    }

    return vertices;
}
std::vector<float> UpdateGridVertices(std::vector<float> vertices, const std::vector<Object> &objs)
{

    // centre of mass calc
    float totalMass = 0.0f;
    float comY = 0.0f;
    for (const auto &obj : objs)
    {
        if (obj.Initalizing)
            continue;
        comY += obj.mass * obj.position.y;
        totalMass += obj.mass;
    }
    if (totalMass > 0)
        comY /= totalMass;

    float originalMaxY = -std::numeric_limits<float>::infinity();
    for (int i = 0; i < vertices.size(); i += 3)
    {
        originalMaxY = std::max(originalMaxY, vertices[i + 1]);
    }

    float verticalShift = comY - originalMaxY;
    std::cout << "vertical shift: " << verticalShift << " |         comY: " << comY << "|            originalmaxy: " << originalMaxY << std::endl;

    for (int i = 0; i < vertices.size(); i += 3)
    {

        // mass bending space
        glm::vec3 vertexPos(vertices[i], vertices[i + 1], vertices[i + 2]);
        glm::vec3 totalDisplacement(0.0f);
        for (const auto &obj : objs)
        {
            // f (obj.Initalizing) continue;

            glm::vec3 toObject = obj.GetPos() - vertexPos;
            float distance = glm::length(toObject);
            float distance_m = distance * 1000.0f;
            float rs = (2 * G * obj.mass) / (c * c);

            float dz = 2 * sqrt(rs * (distance_m - rs));
            totalDisplacement.y += dz * 2.0f;
        }
        vertices[i + 1] = totalDisplacement.y + -abs(verticalShift);
    }

    return vertices;
}