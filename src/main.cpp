#include <SFML/Graphics.hpp>
#include <iostream>
#include <set>

const int N_PARTICLES = 5000;

// Coefficient of restitution
float CR = 0.75f;

// Strength of attraction on LMB click
float CLICK_ATTRACTION = 0.2f;

// Gravitational force
float GRAVITY = 0.5f;

const float PARTICLE_RADIUS = 2.0f;
const float PARTICLE_VELOCITY = 0.0f;

sf::Vector2u const WINDOW_SIZE = { 1920, 1080 };
sf::Vector2u const SIMULATION_SIZE = { 1920, 880 };

const int gridDimX = 500;
const int gridDimY = 500;

typedef struct Particle Particle;

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
};

struct Slider {
    sf::Vector2f LABEL_SIZE{ 300.0f, 20.0f };
    sf::Vector2f BACK_SIZE{ 200.0f, 20.0f };
    sf::Vector2f FRONT_SIZE{ 20.0f, 18.0f };
    sf::Vector2f VALUESTRING_SIZE{ 50.0f, 20.0f };

    sf::Vector2f LABEL_POSITION{ 0.0f, 0.0f };
    sf::Vector2f BACK_POSITION{ LABEL_SIZE.x, 0.0f };
    sf::Vector2f FRONT_POSITION{ LABEL_SIZE.x + BACK_SIZE.x / 2, 1.0f };
    sf::Vector2f VALUESTRING_POSITION{ LABEL_SIZE.x + BACK_SIZE.x + 20.0f, 0.0f };

    sf::RenderWindow* window;

    sf::RectangleShape back;
    sf::RectangleShape front;
    sf::Text label;
    sf::Text valueString;

    bool moving;

    sf::Vector2f position;
    float* value;
    float min, max;

    Slider::Slider(sf::RenderWindow *window, const sf::Font &font, sf::Vector2f position, std::string valueName, float* value, const float min, const float max) {
        this->window = window;
        this->position = position;
        this->value = value;
        this->min = min;
        this->max = max;

        *(this->value) = (min + max) / 2;
        *(this->value) = std::roundf(*(this->value) * 100) / 100.0;

        this->moving = false;

        // Create background
        this->back = sf::RectangleShape(BACK_SIZE);
        back.setPosition(this->position + BACK_POSITION);
        back.setFillColor(sf::Color::White);

        // Create movable button
        this->front= sf::RectangleShape(FRONT_SIZE);
        front.setPosition(this->position + FRONT_POSITION);
        front.setFillColor(sf::Color::Red);

        // Create main label
        this->label.setPosition(this->position + LABEL_POSITION);
        this->label.setFont(font);
        this->label.setString(valueName + ":");
        this->label.setCharacterSize(16);
        this->label.setFillColor(sf::Color::White);

        // Create value label
        this->valueString.setPosition(this->position + VALUESTRING_POSITION);
        this->valueString.setFont(font);
        this->valueString.setString(std::to_string(*(this->value)).substr(0, 4));
        this->valueString.setCharacterSize(16);
        this->valueString.setFillColor(sf::Color::White);

        (*window).draw(this->back);
        (*window).draw(this->front);
        (*window).draw(this->label);
        (*window).draw(this->valueString);
    }

    void Slider::checkForMousePress() {
        sf::Vector2f mousePosition = static_cast<sf::Vector2f>(sf::Mouse::getPosition(*this->window));
        sf::Vector2f difference = mousePosition - this->front.getPosition();

        if (difference.x >= 0 && difference.x < this->front.getSize().x && 
            difference.y >= 0 && difference.y < this->front.getSize().y) {
            this->moving = true;
        }
    }

    void Slider::checkForMouseRelease() {
        if (this->moving == true) {
            this->moving = false;
        }
    }

    void Slider::update() {
        if (this->moving) {
            float mouseX = (float)sf::Mouse::getPosition(*this->window).x;
            this->front.setPosition({ std::min(std::max(mouseX, this->back.getPosition().x), this->back.getPosition().x + this->back.getSize().x - this->front.getSize().x), this->front.getPosition().y });

            *(this->value) = min + (this->front.getPosition().x - this->back.getPosition().x) / (this->back.getSize().x - this->front.getSize().x) * (max - min);
            *(this->value) = std::roundf(*(this->value) * 100) / 100.0;

            this->valueString.setString(std::to_string(*(this->value)).substr(0, 4));
        }

        (*window).draw(this->back);
        (*window).draw(this->front);
        (*window).draw(this->label);
        (*window).draw(this->valueString);
    }
};

float dot(sf::Vector2f& left, sf::Vector2f& right) {
    return left.x * right.x + left.y * right.y;
}

float norm(sf::Vector2f& vec) {
    return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

sf::Vector2f scaleVectorToLength(sf::Vector2f& vec, float length) {
    return length / norm(vec) * vec;
}

bool collisionDetected(Particle& p1, Particle& p2) {
    return norm(p1.position - p2.position) <= 2 * PARTICLE_RADIUS;
}

void handleCollision(Particle& p1, Particle& p2) {
    sf::Vector2f x1 = p1.position;
    sf::Vector2f x2 = p2.position;

    sf::Vector2f v1 = p1.velocity;
    sf::Vector2f v2 = p2.velocity;

    p1.velocity = v1 - (1 + CR) / 2 * dot(v1 - v2, x1 - x2) / std::powf(std::max(norm(x1 - x2), 1e-6f), 2) * (x1 - x2);
    p2.velocity = v2 - (1 + CR) / 2 * dot(v2 - v1, x2 - x1) / std::powf(std::max(norm(x2 - x1), 1e-6f), 2) * (x2 - x1);

    if (x1.x == x2.x && x1.y == x2.y) {
        p1.position += sf::Vector2f{ 1e-2f, 1e-2f };
        p2.position -= sf::Vector2f{ 1e-2f, 1e-2f };
    }
    else {
        p1.position += scaleVectorToLength(x1 - x2, (2 * PARTICLE_RADIUS - norm(x1 - x2)) / 2.0f);
        p2.position += scaleVectorToLength(x2 - x1, (2 * PARTICLE_RADIUS - norm(x2 - x1)) / 2.0f);
    }
}

std::pair<int, int> getGridIndices(Particle& p) {
    int x = std::min((int)(p.position.x / ((float) SIMULATION_SIZE.x / gridDimX)), gridDimX - 1);
    int y = std::min((int)(p.position.y / ((float) SIMULATION_SIZE.y / gridDimY)), gridDimY - 1);

    return { x, y };
}

int main()
{
    // Initialize SFML window
    auto window = sf::RenderWindow{ { WINDOW_SIZE.x, WINDOW_SIZE.y }, "CMake SFML Project" };
    window.setFramerateLimit(144);

    // Load the font
    sf::Font font;
    if (!font.loadFromFile("res/cour.ttf"))
    {
        std::cerr << "Error loading font cour.ttf" << std::endl;
    }

    // Sliders for modifiable parameters
    Slider crSlider = Slider(&window, font, sf::Vector2f{ 200.0f, 930.0f }, "Coefficient of restitution", &CR, 0.5f, 1.0f);
    Slider caSlider = Slider(&window, font, sf::Vector2f{ 200.0f, 980.0f }, "Click attraction", &CLICK_ATTRACTION, 0.0f, 0.5f);
    Slider gravSlider = Slider(&window, font, sf::Vector2f{ 200.0f, 1030.0f }, "Gravitational force", &GRAVITY, 0.0f, 1.0f);

    std::vector<Slider> sliders = { crSlider, caSlider, gravSlider };

    // Random seed
    std::srand(time(NULL));

    // Initialize particles
    std::vector<Particle> particles{};
    std::vector<sf::CircleShape> shapes{};

    for (int i = 0; i < N_PARTICLES; i++) {
        // Generate random position
        float pos_x = std::rand() % int(SIMULATION_SIZE.x - PARTICLE_RADIUS);
        float pos_y = std::rand() % int(SIMULATION_SIZE.y - PARTICLE_RADIUS);

        // Generate random initial velocity
        float alpha = std::rand() % 360;
        float vel_x = std::cos(alpha) * PARTICLE_VELOCITY;
        float vel_y = std::sin(alpha) * PARTICLE_VELOCITY;

        // Create the particle object
        particles.push_back(Particle{ {pos_x, pos_y}, {vel_x, vel_y} });

        // Create a corresponding shape
        shapes.push_back(sf::CircleShape(PARTICLE_RADIUS));
        shapes[i].setFillColor(sf::Color(255, 28, 206));
        shapes[i].setPosition(particles[i].position.x, particles[i].position.y);

        // Draw that shape to the screen
        window.draw(shapes[i]);
    }

    // Initialize optimization containers: grid & map
    std::vector<std::vector<std::set<int>>> particleGrid{};
    std::map<int, std::pair<int, int>> particleMap{};

    for (int i = 0; i < gridDimY; i++) {
        particleGrid.push_back({});
        for (int j = 0; j < gridDimX; j++) {
            particleGrid[i].push_back(std::set<int>{});
        }
    }
    
    for (int i = 0; i < N_PARTICLES; i++) {
        std::pair<int, int> indices = getGridIndices(particles[i]);
        particleGrid[indices.second][indices.first].insert(i);
        particleMap[i] = indices;
    }
    
    // Mainloop
    while (window.isOpen())
    {
        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    for (Slider &s : sliders) {
                        s.checkForMousePress();
                    }
                }
            }
            else if (event.type == sf::Event::MouseButtonReleased) {
                for (Slider &s : sliders) {
                    s.checkForMouseRelease();
                }
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            sf::Vector2f mousePosition = { (float) sf::Mouse::getPosition(window).x, (float)sf::Mouse::getPosition(window).y };

            if (mousePosition.x <= SIMULATION_SIZE.x && mousePosition.y <= SIMULATION_SIZE.y) {
                for (int i = 0; i < N_PARTICLES; i++) {
                    particles[i].velocity += scaleVectorToLength(mousePosition - particles[i].position, CLICK_ATTRACTION);
                }
            }            
        }

        for (int i = 0; i < N_PARTICLES; i++) {

            // Calculate new position
            particles[i].position.x += particles[i].velocity.x;
            particles[i].position.y += particles[i].velocity.y;

            // Vertical collisions
            if (particles[i].position.y < PARTICLE_RADIUS) {
                particles[i].position.y = PARTICLE_RADIUS;
                particles[i].velocity.y = (1 + CR) / 2 * - particles[i].velocity.y;
            }

            if (particles[i].position.y > (SIMULATION_SIZE.y - PARTICLE_RADIUS)) {
                particles[i].position.y = SIMULATION_SIZE.y - PARTICLE_RADIUS;
                particles[i].velocity.y = (1 + CR) / 2 * - particles[i].velocity.y;
            }

            // Horizontal collisions
            if (particles[i].position.x < PARTICLE_RADIUS) {
                particles[i].position.x = PARTICLE_RADIUS;
                particles[i].velocity.x = (1 + CR) / 2 * - particles[i].velocity.x;
            }

            if (particles[i].position.x > (SIMULATION_SIZE.x - PARTICLE_RADIUS)) {
                particles[i].position.x = SIMULATION_SIZE.x - PARTICLE_RADIUS;
                particles[i].velocity.x = (1 + CR) / 2 * - particles[i].velocity.x;
            }

            // Update particleGrid and particleMap
            std::pair<int, int> indices = getGridIndices(particles[i]);

            if (indices.first != particleMap[i].first ||
                indices.second != particleMap[i].second) {
                // If particle moved to different grid cell, we update it
                std::pair<int, int> oldIndices = particleMap[i];
                particleGrid[oldIndices.second][oldIndices.first].erase(i);

                particleMap[i] = indices;
                particleGrid[indices.second][indices.first].insert(i);
            }

            // Particle-to-particle collisions
            if (particleGrid[indices.second][indices.first].size() > 1) {
                for (int j : particleGrid[indices.second][indices.first]) {
                    if (j < i && collisionDetected(particles[i], particles[j])) {
                        handleCollision(particles[i], particles[j]);
                    }
                }
            }
            

            // Gravitational force
            particles[i].velocity.y += 0.1 * GRAVITY;
        }

        for (int i = 0; i < N_PARTICLES; i++) {
            // Draw new shape
            shapes[i].setPosition(particles[i].position.x, particles[i].position.y);
            window.draw(shapes[i]);
        }

        // Update sliders
        for (Slider &s : sliders) {
            s.update();
        }
        
        window.display();
        window.clear();
    }
}