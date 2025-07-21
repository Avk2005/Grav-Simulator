#include <GLFW/glfw3.h>
#include <GLUT/glut.h>
#include <cmath>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>

#define GL_SILENCE_DEPRECATION

const int WIDTH = 800;
const int HEIGHT = 800;
const int NUM_STARS = 200;
const int ASTEROIDS_PER_ORBIT = 10;

struct Planet {
    float radius;
    float orbitRadius;
    float orbitSpeed;
    float angle;
    float r, g, b;
};

struct Star {
    float x, y;
    float baseBrightness;
    float twinklePhase;
};

struct Asteroid {
    float orbitRadius;
    float orbitSpeed;
    float angle;
    float x, y;
};

GLFWwindow* window;

std::vector<Star> stars;
std::vector<Planet> planets = {
    {12,  48,  0.02f, 0, 0.6f, 0.6f, 0.6f},   // Mercury
    {15,  72,  0.015f, 0, 1.0f, 0.5f, 0.3f},  // Venus
    {15,  96,  0.012f, 0, 0.0f, 0.5f, 1.0f},  // Earth
    {13.5, 120, 0.009f, 0, 1.0f, 0.2f, 0.2f}, // Mars
    {27,  168, 0.006f, 0, 1.0f, 0.8f, 0.2f},  // Jupiter
    {22.5, 216, 0.005f, 0, 0.9f, 0.9f, 0.6f}, // Saturn
    {18,  264, 0.004f, 0, 0.6f, 0.8f, 1.0f},  // Uranus
    {18,  312, 0.003f, 0, 0.2f, 0.5f, 1.0f},  // Neptune
};

std::vector<std::string> planetNames = {
    "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"
};

std::vector<std::vector<Asteroid>> asteroidsPerOrbit;

// Draw circle
void drawCircle(float x, float y, float radius, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 100; ++i) {
        float angle = i * 2.0f * M_PI / 100;
        glVertex2f(x + radius * cos(angle), y + radius * sin(angle));
    }
    glEnd();
}

// Draw orbit ring
void drawOrbit(float radius) {
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; ++i) {
        float angle = i * 2.0f * M_PI / 100;
        glVertex2f(radius * cos(angle), radius * sin(angle));
    }
    glEnd();
}

// Draw text at x,y
void drawText(float x, float y, const char* text) {
    glColor3f(1, 1, 1);
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

// Draw title centered at top
void drawTitle() {
    int len = (int)strlen("SOLAR SYSTEM SIMULATOR");
    float textWidth = len * 20.5f;
    float x = -textWidth / 2;
    float y = HEIGHT / 2 - 40;
    drawText(x, y, "SOLAR SYSTEM SIMULATOR");
}

// Draw star with twinkle
void drawStar(const Star& star, float time) {
    float brightness = star.baseBrightness + 0.5f * sin(time + star.twinklePhase);
    brightness = std::max(0.3f, std::min(1.0f, brightness));
    glColor3f(brightness, brightness, brightness);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    glVertex2f(star.x, star.y);
    glEnd();
}

// Draw asteroid as white dot
void drawAsteroid(float x, float y) {
    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    if (!glfwInit()) return -1;
    window = glfwCreateWindow(WIDTH, HEIGHT, "Solar System Simulator", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Setup orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WIDTH/2, WIDTH/2, -HEIGHT/2, HEIGHT/2, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Generate stars
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (int i = 0; i < NUM_STARS; ++i) {
        Star star;
        star.x = (float)(std::rand() % WIDTH) - WIDTH/2.0f;
        star.y = (float)(std::rand() % HEIGHT) - HEIGHT/2.0f;
        star.baseBrightness = 0.5f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX/(0.5f)));
        star.twinklePhase = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX/(2.0f * M_PI));
        stars.push_back(star);
    }

    // Generate asteroids for each orbit with random initial angle and speeds (faster than planets for visual)
    asteroidsPerOrbit.resize(planets.size());
    for (size_t i = 0; i < planets.size(); ++i) {
        asteroidsPerOrbit[i].clear();
        for (int a = 0; a < ASTEROIDS_PER_ORBIT; ++a) {
            float angle = static_cast<float>(std::rand()) / RAND_MAX * 2.0f * M_PI;
            float speed = planets[i].orbitSpeed * (1.5f + static_cast<float>(std::rand()) / RAND_MAX); // some variation, a bit faster than planet
            asteroidsPerOrbit[i].push_back({planets[i].orbitRadius, speed, angle, 0.0f, 0.0f});
        }
    }

    double startTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        double elapsed = currentTime - startTime;

        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();

        // Draw stars
        for (const auto& star : stars)
            drawStar(star, (float)elapsed * 5.0f);

        // Draw orbits
        glLineWidth(2.0f);
        for (const auto& p : planets)
            drawOrbit(p.orbitRadius);

        // Update & draw asteroids
        for (size_t i = 0; i < asteroidsPerOrbit.size(); ++i) {
            for (Asteroid& a : asteroidsPerOrbit[i]) {
                a.angle += a.orbitSpeed;
                if (a.angle > 2.0f * M_PI) a.angle -= 2.0f * M_PI;
                a.x = a.orbitRadius * cos(a.angle);
                a.y = a.orbitRadius * sin(a.angle);
                drawAsteroid(a.x, a.y);
            }
        }

        // Glowing Sun (stronger pulse)
        float sunBrightness = 0.8f + 0.2f * sin(elapsed * 5.0f);
        drawCircle(0, 0, 35, sunBrightness, sunBrightness, 0.0f);

        // Draw planets and names
        for (size_t i = 0; i < planets.size(); ++i) {
            Planet& p = planets[i];
            float x = p.orbitRadius * cos(p.angle);
            float y = p.orbitRadius * sin(p.angle);
            drawCircle(x, y, p.radius, p.r, p.g, p.b);
            drawText(x + 5, y + 5, planetNames[i].c_str());
            p.angle += p.orbitSpeed;
        }

        // Draw title
        drawTitle();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
