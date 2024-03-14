#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

//

Color background = {152, 225, 95, 225};
Color foreground = {30, 46, 5, 225};

// created grid for spawning food and snake
int cellSize = 30;
int cellCount = 25;
int offset = 75;

double timeSpent = 0;
// returns true every interval of time (used to update snake movement)
bool eventTriggered(double interval)
{
    double currentTime = GetTime();

    if (currentTime - timeSpent >= interval)
    {
        timeSpent = currentTime;
        return true;
    }
    return false;
}

// returns true if element is present in queue or not,
// used to food from spawning on snake body
bool ElementInQueue(Vector2 position, deque<Vector2> snakeBody)
{
    for (int i = 0; i < (int)snakeBody.size(); i++)
    {
        if (Vector2Equals(snakeBody[i], position))
            return true;
    }
    return false;
}

// class for food
class Food
{

public:
    Vector2 position;
    Texture2D texture;

    Food(deque<Vector2> snakeBody)
    {
        Image image = LoadImage("Graphics/food.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = generateRandomPosition(snakeBody);
    };
    ~Food()
    {
        UnloadTexture(texture);
    };
    void Draw()
    {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    Vector2 generateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    Vector2 generateRandomPosition(deque<Vector2> snakeBody)
    {
        Vector2 position = generateRandomCell();
        while (ElementInQueue(position, snakeBody))
        {
            position = generateRandomCell();
        }
        return position;
    }
};

// class for Snake
class Snake
{

public:
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    bool addSegment = false;
    Texture2D texture;

    // Snake(){
    //     Image image = LoadImage("Graphics/snake.png");
    //     texture = LoadTextureFromImage(image);
    //     UnloadImage(image);
    // }
    // ~Snake(){
    //     UnloadTexture(texture);
    // }

    void Draw()
    {
        for (int i = 0; i < (int)body.size(); i++)
        {
            // DrawRectangle(body[i].x * cellSize, body[i].y * cellSize, cellSize, cellSize, WHITE);

            Rectangle rectangle = {
                offset + body[i].x * cellSize, // x
                offset + body[i].y * cellSize, // y
                (float)cellSize,               // width
                (float)cellSize                // height
            };
            // DrawTextureRec(texture, rectangle, Vector2{body[i].x * cellSize, body[i].y * cellSize}, WHITE);
            DrawRectangleRounded(rectangle, 0.5, 50, foreground);
        }
    }

    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment)
        {
            addSegment = false;
        }
        else
        {
            body.pop_back();
        }
    }

    void reset()
    {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

// class for containing all objects of game
class Game
{

public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;
    Sound eatSound;
    Sound wallSound;

    Game(){
        InitAudioDevice();
        eatSound = LoadSound("Sounds/eat.mp3");
        wallSound = LoadSound("Sounds/wall.mp3");
    }
    ~Game(){
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }


    void Draw()
    {
        food.Draw();
        snake.Draw();
    }

    void Update()
    {
        if (running)
        {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithBody(snake.body);
        }
    }

    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.body[0], food.position))
        {
            //  increase snake length
            snake.addSegment = true;
            //  generate new food position
            food.position = food.generateRandomPosition(snake.body);
            score++;
            PlaySound(eatSound);
        }
    }

    void CheckCollisionWithEdges()
    {
        Vector2 head = snake.body[0];

        if (head.x == cellCount || head.x == -1)
        {
            GameOver();
        }
        if (head.y == cellCount || head.y == -1)
        {
            GameOver();
        }
    }

    void CheckCollisionWithBody(deque<Vector2> snakeBody)
    {
        deque<Vector2> headlessBody = snakeBody;
        headlessBody.pop_front();
        if (ElementInQueue(snakeBody[0], headlessBody))
        {
            GameOver();
        }
    }

    void GameOver()
    {
        snake.reset();
        food.position = food.generateRandomPosition(snake.body);
        running = false;
        score = 0;
        PlaySound(wallSound);
    }
};

int main()
{

    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Retrosnake");
    SetTargetFPS(60);

    Game game = Game();

    while (WindowShouldClose() == false)
    {

        if (IsKeyDown(KEY_UP) && game.snake.direction.y != 1)
        {
            game.snake.direction = {0, -1};
            game.running = true;
        }
        else if (IsKeyDown(KEY_DOWN) && game.snake.direction.y != -1)
        {
            game.snake.direction = {0, 1};
            game.running = true;
        }
        else if (IsKeyDown(KEY_LEFT) && game.snake.direction.x != 1)
        {
            game.snake.direction = {-1, 0};
            game.running = true;
        }
        else if (IsKeyDown(KEY_RIGHT) && game.snake.direction.x != -1)
        {
            game.snake.direction = {1, 0};
            game.running = true;
        }

        if (eventTriggered(0.2))
        {
            game.Update();
        }

        BeginDrawing();
        ClearBackground(background);
        DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, foreground);
        DrawText("RetoSnake", offset - 5, 20, 40, foreground);
        DrawText(TextFormat("%i", game.score), offset - 5, offset + 15 + cellSize * cellCount , 40, foreground);
        game.Draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}