#include <SFML/Graphics.hpp>

#include <iostream>

#define INIT_SCREEN_SIZE Vector2i(1920, 1920)
#define INIT_VIEW_SIZE Vector2f(1920, 1920)
#define INIT_VIEW_ORIGIN Vector2f(0, 0)

using namespace std;
using namespace sf;

enum struct Dir {
    Left,
    Right,
    Up,
    Down,

    Same
};

struct Entity {
    Sprite sprite;
    Vector2i pos;
};

struct Snake : Entity {
    static Dir dir;
    static Dir q_dir;
};

struct Apple : Entity {};

Dir Snake::dir = Dir::Right;
Dir Snake::q_dir = Dir::Right;

Dir Inv(Dir dir) {
    if (dir == Dir::Down) return Dir::Up;
    if (dir == Dir::Left) return Dir::Right;
    if (dir == Dir::Up) return Dir::Down;

    return Dir::Left;
}

int Mod(const int& a, const unsigned int& m) {
    const int r = a % m;

    return r >= 0 ? r : m - r;
}

Dir GetInputDir() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) return Dir::Down;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) return Dir::Left;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) return Dir::Up;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) return Dir::Right;

    return Dir::Same;
}

Vector2i DirToVec(Dir dir) {
    if (dir == Dir::Down) return Vector2i(0, -1);
    if (dir == Dir::Left) return Vector2i(-1, 0);
    if (dir == Dir::Up) return Vector2i(0, 1);
    
    return Vector2i(1, 0);
}

IntRect GetBodyTextureRect(Dir f, Dir b) {
    if (f > b) swap(f, b);

    if (f == Dir::Left && b == Dir::Right) return IntRect(64, 0, 64, 64);
    if (f == Dir::Left && b == Dir::Up) return IntRect(128, 0, 64, 64);
    if (f == Dir::Left && b == Dir::Down) return IntRect(128, 128, 64, 64);
    if (f == Dir::Right && b == Dir::Up) return IntRect(0, 0, 64, 64);
    if (f == Dir::Right && b == Dir::Down) return IntRect(0, 64, 64, 64);
    
    return IntRect(128, 64, 64, 64);
}

IntRect GetHeadTailTextureRect(Dir dir, bool is_head) {
    const Vector2i offset = is_head ? Vector2i(192, 0) : Vector2i(192, 128);

    if (dir == Dir::Right) return IntRect(64 + offset.x, offset.y, 64, 64);
    if (dir == Dir::Up) return IntRect(64 + offset.x, 64 + offset.y, 64, 64);
    if (dir == Dir::Down) return IntRect(offset.x, offset.y, 64, 64);
    
    return IntRect(offset.x, 64 + offset.y, 64, 64);
}

Dir GetDir(sf::Vector2i& f, Vector2i& t) {
    Dir dir = f.x == t.x ? f.y < t.y ? Dir::Up : Dir::Down : f.x < t.x ? Dir::Right : Dir::Left;

    return ((dir == Dir::Left || dir == Dir::Right) && abs(t.x - f.x) > 1) || ((dir == Dir::Down || dir == Dir::Up) && abs(t.y - f.y) > 1) ? Inv(dir) : dir;
}

void SnakeUpdateTexture(Snake* snake, int snake_size, const Vector2f& tile_size) {
    for (int i = 1; i < snake_size - 1; i++) {
        Dir f_dir = GetDir(snake[i].pos, snake[i - 1].pos);
        Dir b_dir = GetDir(snake[i].pos, snake[i + 1].pos);

        snake[i].sprite.setTextureRect(GetBodyTextureRect(f_dir, b_dir));
        snake[i].sprite.setPosition(Vector2f(tile_size.x * snake[i].pos.x, tile_size.y * snake[i].pos.y));
    }

    snake[0].sprite.setTextureRect(GetHeadTailTextureRect(Snake::dir, true));
    snake[0].sprite.setPosition(Vector2f(tile_size.x * snake[0].pos.x, tile_size.y * snake[0].pos.y));

    snake[snake_size - 1].sprite.setTextureRect(GetHeadTailTextureRect(GetDir(snake[snake_size - 1].pos, snake[snake_size - 2].pos), false));
    snake[snake_size - 1].sprite.setPosition(Vector2f(tile_size.x * snake[snake_size - 1].pos.x, tile_size.y * snake[snake_size - 1].pos.y));
}

void SnakeInit(Snake* snake, int snake_size, int max_snake_size, Texture& texture, const Vector2f& tile_size) {
    for (int i = 0; i < max_snake_size; i++) {
        snake[i].sprite.setTexture(texture);
        snake[i].sprite.scale(tile_size.x / 64.f, tile_size.y / 64.f);
    }

    for (int i = 0; i < snake_size; i++) {
        snake[i].pos = sf::Vector2i(snake_size - i - 1, 0);
    }

    SnakeUpdateTexture(snake, snake_size, tile_size);
}

void SnakeUpdateMovement(Snake* snake, int snake_size, const Vector2i grid_size) {
    Vector2i temp_pos[snake_size - 1];

    for (int i = 1; i < snake_size; i++) {
        temp_pos[i - 1] = snake[i - 1].pos;
    }

    for (int i = 1; i < snake_size; i++) {
        snake[i].pos = temp_pos[i - 1];
    }

    snake[0].pos += DirToVec(Snake::dir);

    snake[0].pos.x = Mod(snake[0].pos.x, grid_size.x);
    snake[0].pos.y = Mod(snake[0].pos.y, grid_size.y);
}

bool SnakeCollided(Snake* snake, int snake_size) {
    for (int i = 1; i < snake_size; i++) {
        if (snake[0].pos == snake[i].pos) return true;
    }

    return false;
}

void AppleRespawn(Apple& apple, const Vector2f& tile_size, const Vector2i& grid_size) {
    srand(time(nullptr));

    apple.pos = Vector2i(rand() % grid_size.x, rand() % grid_size.y);
    apple.sprite.setPosition(apple.pos.x * tile_size.x, apple.pos.y * tile_size.y);
}

void AppleInit(Apple& apple, Texture& texture, const Vector2f& tile_size, const Vector2i& grid_size) {
    apple.sprite.setTexture(texture);
    apple.sprite.scale(tile_size.x / 64.f, tile_size.y / 64.f);
    apple.sprite.setTextureRect(IntRect(0, 192, 64, 64));

    AppleRespawn(apple, tile_size, grid_size);
}

bool AteApple(Snake* snake, Apple& apple) {
    return snake[0].pos == apple.pos;
}

int main() {
    RenderWindow window(sf::VideoMode(INIT_SCREEN_SIZE.x, INIT_SCREEN_SIZE.y), "Snake", Style::Default & (~Style::Resize));
    View view(INIT_VIEW_ORIGIN, INIT_VIEW_SIZE);

    window.setFramerateLimit(60);
    
    Vector2f screen_resolution = Vector2f(INIT_SCREEN_SIZE.x, INIT_SCREEN_SIZE.y);
    
    Event event;
    Font font;
    font.loadFromFile("alpha.ttf");
    Texture texture;
    texture.loadFromFile("snake.png");
    Shader shader;
    shader.loadFromFile("test.vert", "test.frag");

    const sf::Vector2i grid_size(16, 16);
    const sf::Vector2f tile_size = sf::Vector2f(view.getSize().x / grid_size.x, view.getSize().y / grid_size.y);

    Snake snake[grid_size.x * grid_size.y];
    int snake_size = 3;

    Apple apple;
    AppleInit(apple, texture, tile_size, grid_size);

    SnakeInit(snake, snake_size, grid_size.x * grid_size.y, texture, tile_size);

    float move_time = .1f;
    float c_move_time = 0;
    float delta;

    Clock clock;
    
    while (window.isOpen()) {
        while (window.pollEvent(event)) {
            switch (event.type) {
                case Event::Closed:
                    window.close();

                    break;
                case Event::Resized:
                    Vector2f ratio = (Vector2f)window.getSize();
                    ratio.x /= screen_resolution.x;
                    ratio.y /= screen_resolution.y;
                    screen_resolution = (Vector2f)window.getSize();
                    sf::Vector2f view_size = view.getSize();
                    view_size.x *= ratio.x;
                    view_size.y *= ratio.y;
                    view.setSize(view_size);
                    
                    break;
            }
        }

        delta = clock.restart().asSeconds();
        c_move_time += delta;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) continue;

        Dir dir = GetInputDir();
        if (dir != Dir::Same && dir != Inv(Snake::dir)) Snake::q_dir = dir;

        if (c_move_time >= move_time) {
            c_move_time = 0;

            Snake::dir = Snake::q_dir;

            if (AteApple(snake, apple)) {
                AppleRespawn(apple, tile_size, grid_size);

                snake_size++;
            }

            SnakeUpdateMovement(snake, snake_size, grid_size);
            SnakeUpdateTexture(snake, snake_size, tile_size);

            if (SnakeCollided(snake, snake_size)) window.close();
        }

        window.clear();

        window.draw(apple.sprite);
        
        for (int i = 0; i < snake_size; i++) {
            window.draw(snake[i].sprite);
        }

        window.display();
    }
    
    return 0;
}