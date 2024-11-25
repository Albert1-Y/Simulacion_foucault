#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <vector>

#define PI 3.14159265358979323846

class FoucaultPendulum {
  private:
    // parametros fisicos
    const double l = 2.0;
    const double g = 9.80;
    const double w = (0.07);
    const double angulo = 44;
    const double dt = (1.0 / 60.0);

    double x, y, vx, vy, ax, ay;
    double time;
    double timeScale;

    std::vector<sf::Vertex> trail;

    std::chrono::steady_clock::time_point lastUpdate;

    std::string formatTime(double seconds) const {
        int hours = static_cast<int>(seconds) / 3600;
        int minutes = (static_cast<int>(seconds) % 3600) / 60;
        double secs = std::fmod(seconds, 60.0);

        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << hours << ":"
            << std::setfill('0') << std::setw(2) << minutes << ":"
            << std::fixed << std::setprecision(1) << std::setfill('0') << std::setw(4) << secs;
        return ss.str();
    }
  public:
    FoucaultPendulum() :
        x(150), y(150), vx(0), vy(0),
        ax(-g * (x / l)), ay(-g * (y / l)),
        time(0), timeScale(1.0),
        lastUpdate(std::chrono::steady_clock::now()) {
            trail.push_back(sf::Vertex(sf::Vector2f(x, y), sf::Color(100, 100, 255, 100)));
        }

    void update() {
        auto currentTime = std::chrono::steady_clock::now();
        double realDt = std::chrono::duration<double>(currentTime - lastUpdate).count();
        lastUpdate = currentTime;

        double scaledDt = realDt * timeScale;

        int steps = static_cast<int>(scaledDt / dt);
        double remainingDt = scaledDt - (steps * dt);

        for (int i = 0; i < steps; ++i) {
            updatePhysics(dt);
        }
        if (remainingDt > 0) {
            updatePhysics(remainingDt);
        }
    }

    void updatePhysics(double deltaTime) {
        vx += ax * deltaTime;
        vy += ay * deltaTime;
        x += vx * deltaTime;
        y += vy * deltaTime;
        time += deltaTime;

        ax = -(g / l) * x + 2 * w * std::sin(angulo * PI / 180.0) * vy;
        ay = -(g / l) * y - 2 * w * std::sin(angulo * PI / 180.0) * vx;

        trail.push_back(sf::Vertex(sf::Vector2f(x, y), sf::Color(100, 100, 255, 100)));
    }

    void adjustTimeScale(double factor) {
        timeScale *= factor;
        if (timeScale < 0.1) timeScale = 0.1;
        if (timeScale > 200.0) timeScale = 200.0;
    }

    void draw(sf::RenderWindow& window, const sf::Vector2f& offset) {
        std::vector<sf::Vertex> displayTrail = trail;
        for (auto& vertex : displayTrail) {
            vertex.position += offset;
        }
        window.draw(&displayTrail[0], displayTrail.size(), sf::LineStrip);

        // pendulo
        sf::CircleShape bob(10);
        bob.setFillColor(sf::Color::Red);
        bob.setPosition(x + offset.x - 10, y + offset.y - 10);
        window.draw(bob);

        sf::VertexArray line(sf::Lines, 2);
        line[0].position = sf::Vector2f(offset.x, offset.y);
        line[1].position = sf::Vector2f(x + offset.x, y + offset.y);
        line[0].color = line[1].color = sf::Color::White;
        window.draw(line);
    }

    std::string getStats() const {
        std::stringstream ss;
        ss << "Tiempo: " << formatTime(time) << "\n"
            << "Velocidad: x" << std::fixed << std::setprecision(1) << timeScale << "\n"
            << "Posición: (" << std::fixed << std::setprecision(2) << x << ", " << y << ")\n"
            << "Velocidad: (" << std::fixed << std::setprecision(2) << vx << ", " << vy << ")\n"
            << "Aceleración: (" << std::fixed << std::setprecision(2) << ax << ", " << ay << ")\n"
            << "Puntos en trayectoria: " << trail.size() << "\n"
            << "\nControles:\n"
            << "+ : Acelerar tiempo (x1.5)\n"
            << "- : Desacelerar tiempo (÷1.5)\n"
            << "R : Reiniciar simulación";
        return ss.str();
    }

    void reset() {
        x = 150;
        y = 150;
        vx = 0;
        vy = 0;
        ax = -g * (x / l);
        ay = -g * (y / l);
        time = 0;
        timeScale = 1.0;

        trail.clear();
        trail.push_back(sf::Vertex(sf::Vector2f(x, y), sf::Color(100, 100, 255, 100)));
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(1200, 800), "Péndulo de Foucault");
    window.setFramerateLimit(60);

    FoucaultPendulum pendulum;

    sf::Font font;
    if (!font.loadFromFile("resources/tuffy.ttf")) {
        return -1;
    }

    sf::Text stats;
    stats.setFont(font);
    stats.setCharacterSize(20);
    stats.setFillColor(sf::Color::White);
    stats.setPosition(10, 10);

    sf::Vector2f center(window.getSize().x / 2.f, window.getSize().y / 2.f);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                case sf::Keyboard::Add:
                case sf::Keyboard::Equal:
                    pendulum.adjustTimeScale(1.5);
                    break;
                case sf::Keyboard::Subtract:
                case sf::Keyboard::Dash:
                    pendulum.adjustTimeScale(1.0 / 1.5);
                    break;
                case sf::Keyboard::R:
                    pendulum.reset();
                    break;
                }
            }
        }

        pendulum.update();

        window.clear(sf::Color(30, 30, 30));
        stats.setString(pendulum.getStats());
        pendulum.draw(window, center);
        window.draw(stats);
        window.display();
    }

    return 0;
}