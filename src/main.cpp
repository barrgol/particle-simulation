#include <SFML/Graphics.hpp>
#include <iostream>

typedef struct Particle Particle;

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
};

const int N_PARTICLES = 200;
const int PARTICLE_RADIUS = 10;
const float PARTICLE_VELOCITY = 2.0f;

float dot(sf::Vector2f& left, sf::Vector2f& right) {
    return left.x * right.x + left.y * right.y;
}

float norm(sf::Vector2f& vec) {
    return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

bool collisionDetected(Particle p1, Particle p2) {
    return norm(p1.position - p2.position) <= 2 * PARTICLE_RADIUS;
}

void handleCollision(Particle& p1, Particle& p2) {
    sf::Vector2f x1 = p1.position;
    sf::Vector2f x2 = p2.position;

    sf::Vector2f v1 = p1.velocity;
    sf::Vector2f v2 = p2.velocity;

    p1.velocity = v1 - dot(v1 - v2, x1 - x2) / std::powf(norm(x1 - x2), 2) * (x1 - x2);
    p2.velocity = v2 - dot(v2 - v1, x2 - x1) / std::powf(norm(x2 - x1), 2) * (x2 - x1);
}

int main()
{
    sf::Vector2u const WINDOW_SIZE = { 1920, 1080 };
    
    auto window = sf::RenderWindow{ { WINDOW_SIZE.x, WINDOW_SIZE.y }, "CMake SFML Project" };
    window.setFramerateLimit(144);

    std::srand(time(NULL));

    // Initialize particles
    Particle particles[N_PARTICLES];
    sf::CircleShape shapes[N_PARTICLES];

    for (int i = 0; i < N_PARTICLES; i++) {
        // Generate random position
        float pos_x = std::rand() % int(WINDOW_SIZE.x - PARTICLE_RADIUS);
        float pos_y = std::rand() % int(WINDOW_SIZE.y - PARTICLE_RADIUS);

        // Generate random initial velocity
        float alpha = std::rand() % 360;
        float vel_x = std::cos(alpha) * PARTICLE_VELOCITY;
        float vel_y = std::sin(alpha) * PARTICLE_VELOCITY;

        // Create the particle object
        particles[i] = Particle{ {pos_x, pos_y}, {vel_x, vel_y} };

        // Create a corresponding shape
        shapes[i] = sf::CircleShape(PARTICLE_RADIUS);
        shapes[i].setFillColor(sf::Color::Green);
        shapes[i].setPosition(particles[i].position.x, particles[i].position.y);

        // Draw that shape to the screen
        window.draw(shapes[i]);
    }

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
        }

        for (int i = 0; i < N_PARTICLES; i++) {

            // Vertical collisions
            if (particles[i].position.y <= PARTICLE_RADIUS || particles[i].position.y >= (WINDOW_SIZE.y - PARTICLE_RADIUS)) {
                particles[i].velocity.y = -particles[i].velocity.y;
            }

            // Horizontal collisions
            if (particles[i].position.x <= PARTICLE_RADIUS || particles[i].position.x >= (WINDOW_SIZE.x - PARTICLE_RADIUS)) {
                particles[i].velocity.x = -particles[i].velocity.x;
            }

            // Particle-to-particle collisions
            for (int j = 0; j < i; j++) {
                if (collisionDetected(particles[i], particles[j])) {
                    handleCollision(particles[i], particles[j]);
                }
            }

            // Calculate new position
            particles[i].position.x += particles[i].velocity.x;
            particles[i].position.y += particles[i].velocity.y;

            // Draw new shape
            shapes[i].setPosition(particles[i].position.x, particles[i].position.y);
            window.draw(shapes[i]);
        }
        
        window.display();
        window.clear();
    }
}