#include "includes.h"

struct Shape
{
    v2f position;
    uint32_t color;
    float currentAngle;
    virtual void Draw(Window& window, DrawMode drawMode = DrawMode::Normal) { return; }
    virtual void Rotate(float angle) { return; }
    virtual void SetRotation(float angle)
    {
        if(currentAngle != angle)
        {
            Rotate(angle - currentAngle);
        }
    }
};

struct Rect : Shape
{
    int width, height;
    v2f vertices[4];
    Rect() = default;
    Rect(float x, float y, float width, float height, uint32_t color) : width(width), height(height) 
    {
        position.x = x;
        position.y = y;
        this->color = color;
        currentAngle = 0;
    }
    void Rotate(float angle) override 
    {
        currentAngle += angle;
        vertices[0] = rotate(currentAngle, v2f(-width * 0.5, -height * 0.5));
        vertices[1] = rotate(currentAngle, v2f(width * 0.5, -height * 0.5));
        vertices[2] = rotate(currentAngle, v2f(-width * 0.5, height * 0.5));
        vertices[3] = rotate(currentAngle, v2f(width * 0.5, height * 0.5));
    }
    void Draw(Window& window, DrawMode drawMode = DrawMode::Normal) override
    {
        if(currentAngle == 0)
            window.DrawRect(color, position.x-width*0.5, position.y-height*0.5, position.x+width*0.5, position.y+height*0.5, drawMode);
        else
        {
            window.DrawTriangle(color, vertices[0].x + position.x, vertices[0].y + position.y, vertices[1].x + position.x,
            vertices[1].y + position.y, vertices[2].x + position.x, vertices[2].y + position.y, drawMode);
            window.DrawTriangle(color, vertices[1].x + position.x, vertices[1].y + position.y, vertices[2].x + position.x,
            vertices[2].y + position.y, vertices[3].x + position.x, vertices[3].y + position.y, drawMode);
        }
    }
};

struct Circle : Shape
{
    int radius;
    Circle() = default;
    Circle(float x, float y, float radius, uint32_t color) : radius(radius) 
    {
        position.y = y;
        position.x = x;
        this->color = color;
    }
    void Rotate(float angle) override
    {
        return;
    }
    void Draw(Window& window, DrawMode drawMode = DrawMode::Normal) override
    {
        window.DrawCircle(color, position.x, position.y, radius, drawMode);
    }
    void SetRotation(float angle) override
    {
        return;
    }
};

struct Triangle : Shape
{
    v2f vertices[3];
    v2f cVertices[3];
    Triangle() = default;
    Triangle(const v2f v1, const v2f v2, const v2f v3, v2f offset, uint32_t color) 
    {
        cVertices[0] = vertices[0] = v1;
        cVertices[1] = vertices[1] = v2;
        cVertices[2] = vertices[2] = v3;
        position = offset;
        this->color = color;
        currentAngle = 0;
    }
    void Draw(Window& window, DrawMode drawMode = DrawMode::Normal) override
    {
        window.DrawTriangle(color, cVertices[0].x + position.x,
        cVertices[0].y + position.y, cVertices[1].x + position.x,
        cVertices[1].y + position.y, cVertices[2].x + position.x,
        cVertices[2].y + position.y, drawMode);
    }
    void Rotate(float angle) override
    {
        currentAngle += angle;
        cVertices[0] = rotate(currentAngle, vertices[0]);
        cVertices[1] = rotate(currentAngle, vertices[1]);
        cVertices[2] = rotate(currentAngle, vertices[2]);
    }
};

enum class pShape
{
    Rect,
    Circle,
    Triangle,
    Pixel
};

struct Enemy
{
    float velocity;
    Shape* shape;
    float health;
    bool remove;
    int cooldown;
    pShape data;
};

struct Player
{
    float velocity;
    Rect rect;
    int health;
    int cooldown;
};

struct Missile
{
    float velocity;
    Triangle triangle;
    float angle;
    float distance_current;
    float distance_max;
    bool remove;
};

enum class pMode
{
    Normal,
    Replay
};

enum class pBehaviour
{
    Sinusoidal,
    Directional
};

struct particle
{
    uint32_t color;
    float rotation;
    float size;
    float velocity;
    float gravity;
    float distance_max;
    int frame_current = 0;
    int frame_max;
    bool dead;
    pMode mode;
    pShape shape;
    pBehaviour behaviour;
    v2f pos_start;
    v2f pos_current;
};

struct pData
{
    rect rect;
    float size_max;
    float size_min;
    float angle_max;
    float angle_min;
    float speed_max;
    float speed_min;
    std::vector<uint32_t> colors;
};

auto equilateral = [](Triangle& triangle, float size)
{
    const float m = 0.577350269f;
    triangle.cVertices[0] = triangle.vertices[0] = v2f(0.0f, m * size);
    triangle.cVertices[1] = triangle.vertices[1] = v2f(size * 0.5f, -m * size);
    triangle.cVertices[2] = triangle.vertices[2] = v2f(-size * 0.5f, -m * size);
};

struct pSystem
{
    std::vector<particle> particles;
    bool bPause = false;
    v2f position;
    pSystem() = default;
    pSystem(int x, int y) 
    {
        bPause = true;
        position.x = x;
        position.y = y;
    }
    inline void Generate(pData& data, int size, pMode mode, pShape shape, pBehaviour behaviour, float gravity, float distance, int frame)
    {
        for(int i = 0; i < size; i++)
        {
            particles.push_back({
                data.colors[rand(0, (int)data.colors.size())],
                rand(data.angle_min, data.angle_max),
                rand(data.size_min, data.size_max),
                rand(data.speed_min, data.speed_max),
                gravity, distance, 0, frame,
                false, mode, shape, behaviour,
                {
                    rand(data.rect.sx, data.rect.ex) + position.x,
                    rand(data.rect.sy, data.rect.ey) + position.y
                },
            });
            particles.back().pos_current = particles.back().pos_start;
        }
    }
    inline void Update(int replayAmount)
    {
        if(bPause) return;
        for(auto& p : particles)
        {
            switch(p.mode)
            {
                case pMode::Replay:
                {
                    if(p.distance_max <= 0.0f && p.frame_current % p.frame_max == 0)
                        p.pos_current = p.pos_start;
                    p.dead = (p.frame_current == p.frame_max * replayAmount);
                }
                break;
                case pMode::Normal:
                {
                    p.dead = p.frame_current > p.frame_max;
                }
                break;
            }

            switch(p.behaviour)
            {
                case pBehaviour::Directional:
                {
                    p.pos_current.x += cos(p.rotation) * p.velocity;
                    p.pos_current.y += sin(p.rotation) * p.velocity;
                }
                break;
                case pBehaviour::Sinusoidal:
                {
                    p.pos_current.x += cos(p.rotation) * p.velocity;
                    p.pos_current.y += sin(p.rotation) * p.velocity;
                    p.pos_current.x += cos(p.frame_current * 0.2f) * p.velocity;
                    p.pos_current.y += sin(p.frame_current * 0.2f) * p.velocity;
                }
                break;
            }

            p.frame_current++;

            float x = p.pos_current.x - p.pos_start.x;
            float y = p.pos_current.y - p.pos_start.y;

            if(p.distance_max > 0.0f && std::hypot(x, y) > p.distance_max)
            {
                if(p.mode != pMode::Normal)
                    p.pos_current = p.pos_start;
                else
                    p.dead = true;
            }   
                          
            p.pos_current.y += p.gravity;
        }

        particles.erase(std::remove_if(particles.begin(), particles.end(), [&](particle& p){return p.dead;}), particles.end());
    }
    inline void Draw(Window& window, DrawMode drawMode = DrawMode::Normal)
    {
        if(!bPause)
            for(auto& p : particles)
                switch(p.shape)
                {
                    case pShape::Rect:
                    {
                        Rect rect;
                        rect.color = p.color;
                        rect.height = rect.width = p.size;
                        rect.position.x = p.pos_current.x;
                        rect.position.y = p.pos_current.y;
                        rect.Rotate(p.velocity);
                        rect.Draw(window, drawMode);
                    }
                    break;
                    case pShape::Circle:
                    {
                        Circle circle;
                        circle.color = p.color;
                        circle.radius = p.size;
                        circle.position.x = p.pos_current.x;
                        circle.position.y = p.pos_current.y;
                        circle.Draw(window, drawMode);
                    }
                    break;
                    case pShape::Triangle:
                    {
                        Triangle triangle;
                        triangle.color = p.color;
                        triangle.position.x = p.pos_current.x;
                        triangle.position.y = p.pos_current.y;
                        equilateral(triangle, p.size);
                        triangle.Rotate(p.velocity);
                        triangle.Draw(window, drawMode);
                    }
                    case pShape::Pixel: 
                    {
                        window.SetPixel(p.color, p.pos_current.x,
                        p.pos_current.y, drawMode);
                    }
                    break;
                }
    }
};

struct seed
{
    v2f position;
    bool remove;
};

struct MouseState
{
    uint32_t buttons;
    int x, y;
};

enum class GameState
{
    MainMenu,
    GameLoop,
    EndSuccess,
    EndFail,
    Stats
};

uint32_t enemy_colors[8] = {
    0xFF820C58, 0xFF7347FE, 0xFF8DD6F6, 0xFFA1308F,
    0xFF0C1B59, 0xFF0000FF, 0xFFABCFC1, 0xFFD5C6AC
};

inline void TakeScreenShot(Window& window, const std::string& file)
{
    const int w = window.GetWidth();
    const int h = window.GetHeight();
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ABGR8888);
    memcpy(surface->pixels, window.drawTargets[window.currentDrawTarget].data.data(), 4*w*h);
    IMG_SavePNG(surface, file.c_str());
    SDL_FreeSurface(surface);
}

struct Stats
{
    int SeedsCollected;
    int EnemiesKilled;
    int EnemiesSpawned;
    int MissilesFired;
    int MissilesHit;
    int PlayerDeaths; 
};

struct Captures
{
    int count;
    std::string prefix;
    std::string directory;
};

class Game
{
private:
    Stats stats;
    Window window;
    Player player;
    pSystem ps;
    pData explosion;
    pData kill;
    pData smoke;
    std::vector<Enemy> enemies;
    std::vector<Missile> missiles;
    std::vector<seed> seeds;
    GameState currentState;
    Captures captures;
    Button start, retry, home, stat, back;
    DataNode savefile;
public:
    inline void Start()
    {
        currentState = GameState::MainMenu;
        
        srand(time(0));

        window.Init("Window", 800, 600);

        player = {5.0f, Rect(60, 60, 30, 30, 0xFF00FF00), 20};

        ps = pSystem(0, 0);

        start = Button("assets\\start.png");
        retry = Button("assets\\retry.png");
        home = Button("assets\\home.png");
        back = Button("assets\\back.png");
        stat = Button("assets\\stats.png");
        start.position = v2f(250, 400);
        stat.position = v2f(550, 400);
        back.position = v2f(700, 550);
        retry.position = v2f(250, 400);
        home.position = v2f(550, 400);

        back.size = 5;
        stat.size = start.size = retry.size = home.size = 10;

        Deserialize(savefile, "datafile.txt");

        stats.EnemiesKilled = GetInt(savefile, "Enemies->Killed");
        stats.EnemiesSpawned = GetInt(savefile, "Enemies->Spawned");
        stats.MissilesFired = GetInt(savefile, "Missiles->Fired");
        stats.MissilesHit = GetInt(savefile, "Missiles->Hit");
        stats.PlayerDeaths = GetInt(savefile, "Player Deaths");
        stats.SeedsCollected = GetInt(savefile, "Seeds Collected");
        captures.count = GetInt(savefile, "Captures->Count");
        captures.directory = GetString(savefile, "Captures->Directory");
        captures.prefix = GetString(savefile, "Captures->Prefix");

        smoke.colors.push_back(0xFFD8D8D8);
        smoke.colors.push_back(0xFFB1B1B1);
        smoke.colors.push_back(0xFF7E7E7E);
        smoke.colors.push_back(0xFF474747);
        smoke.colors.push_back(0xFF1A1A1A);
        smoke.colors.push_back(0xFFFFFFFF);
        smoke.rect.ex = smoke.rect.sx = 0;
        smoke.rect.ey = smoke.rect.sy = 0;
        smoke.speed_max = 1;
        smoke.speed_min = 0;

        explosion.colors.push_back(0xFFD8D8D8);
        explosion.colors.push_back(0xFFB1B1B1);
        explosion.colors.push_back(0xFF7E7E7E);
        explosion.colors.push_back(0xFF474747);
        explosion.colors.push_back(0xFF1A1A1A);
        explosion.colors.push_back(0xFF0000FF);
        explosion.colors.push_back(0xFF005AFF);
        explosion.colors.push_back(0xFF009AFF);
        explosion.colors.push_back(0xFF00CCFF);
        explosion.colors.push_back(0xFF08E8FF);
        explosion.rect.ey = explosion.rect.ex = 5.0f;
        explosion.rect.sy = explosion.rect.sx = -5.0f;
        explosion.angle_min = 0;
        explosion.angle_max = 360;
        explosion.size_min = 3;
        explosion.size_max = 6;
        explosion.speed_min = 4;
        explosion.speed_max = 5;

        kill.colors.resize(1);
        kill.rect.ey = kill.rect.ex = 25.0f;
        kill.rect.sy = kill.rect.sx = -25.0f;
        kill.angle_min = 0;
        kill.angle_max = 360;
        kill.size_min = 6;
        kill.size_max = 7;
        kill.speed_min = 2;
        kill.speed_max = 4;

        ps.bPause = false;

        player.cooldown = 0;

        Restart();
    }
    inline void Restart()
    {
        for(auto& enemy : enemies)
        {
            delete enemy.shape;
            enemy.shape = nullptr;
        }
        enemies.clear();
        seeds.clear();
        missiles.clear();
        ps.particles.clear();
        const int size = seeds.size();
        for(int i = 0; i < 50 - size; i++)
        {
            seeds.push_back({
                {
                    rand(10.0f, window.GetWidth() - 10.0f),
                    rand(10.0f, window.GetHeight() - 10.0f)
                }, false
            });
        }
        player.cooldown = 0;
        player.health = 20.0f;
        player.rect.width = 30;
        player.rect.height = 30;
        player.velocity = 5.0f;
        player.rect.position.x = 60.0f;
        player.rect.position.y = 60.0f;
    }
    inline void SpawnMissile(const v2f start, const v2f destination, uint32_t color, float distance_max)
    {
        Triangle tri;
        equilateral(tri, 20);
        tri.color = color;
        v2f dist = destination - start;
        tri.position = start;
        missiles.push_back(Missile{
            10.0f, tri, atan2(dist.y, dist.x),
            0.0f, distance_max, false
        });
    }
    inline void ExplodeMissile(Missile& m)
    {
        if(m.remove) return;
        ps.position = m.triangle.position;
        ps.Generate(explosion, 18, pMode::Normal,
        pShape::Circle, pBehaviour::Directional,
        -1.8f, 80.0f, 40);
        m.remove = true;
    }
    inline void SpawnEnemy(pShape shape, v2f start_pos)
    {
        stats.EnemiesSpawned++;
        uint32_t color = enemy_colors[rand(0, 8)];
        enemies.push_back({
            6.0f, nullptr,
            20.0f, false, 
            100, shape
        });
        switch(shape)
        {
            case pShape::Circle:
            {
                enemies.back().shape = new Circle(start_pos.x, start_pos.y, 10.0f, color);
            }
            break;
            case pShape::Rect:
            {
                enemies.back().shape = new Rect(start_pos.x, start_pos.y, 20.0f, 20.0f, color);
            }
            break;
            case pShape::Triangle:
            {
                const float m = 0.577350269f;
                enemies.back().shape = new Triangle(v2f(0.0f, m * 20.0f), v2f(10.0f, -m * 20.0f), v2f(-10.0f, -m * 20.0f), start_pos, color);
            }
            break;
        };
    }
    inline void UpdateAndDraw(const uint8_t* keystates, const MouseState& mouseState)
    {
        switch(currentState)
        {
            case GameState::MainMenu: MainMenu(keystates, mouseState); break;
            case GameState::GameLoop: GameLoop(keystates, mouseState); break;
            case GameState::EndSuccess: EndSuccess(keystates, mouseState); break;
            case GameState::EndFail: EndFail(keystates, mouseState); break;
            case GameState::Stats: StatsScreen(mouseState); break;
        }
    }
    inline void MainMenu(const uint8_t* keystates, const MouseState& state)
    {
        if(start.clicked(state.x, state.y, state.buttons & SDL_BUTTON(1)))
        {
            Restart();
            currentState = GameState::GameLoop;
        }
        if(stat.clicked(state.x, state.y, state.buttons & SDL_BUTTON(1)))
        {
            currentState = GameState::Stats;
        }
        window.Clear(0xFFFFFFFF);
        start.render(window);
        stat.render(window);
        window.DrawText({150, 40, 650, 92}, "SQUARE-IO", 0xFF00FF00);
        window.Present();
    }
    inline void StatsScreen(const MouseState& state)
    {
        if(back.clicked(state.x, state.y, state.buttons & SDL_BUTTON(1)))
        {
            currentState = GameState::MainMenu;
        }
        std::string str;
        str += "Enemies Killed: " + std::to_string(stats.EnemiesKilled) + "\n";
        str += "Enemies Spawned: " + std::to_string(stats.EnemiesSpawned) + "\n";
        str += "Missiles Fired: " + std::to_string(stats.MissilesFired) + "\n";
        str += "Missiles Hit: " + std::to_string(stats.MissilesHit) + "\n";
        str += "Seeds Collected: " + std::to_string(stats.SeedsCollected) + "\n";
        str += "Player Deaths: " + std::to_string(stats.PlayerDeaths) + "\n";
        window.Clear(0xFFFFFFFF);
        back.render(window);
        window.DrawText({300, 20, 500, 80}, "STATS", 0xFF000000);
        window.DrawText({150, 100, 700, 550}, str, 0xFF000000);
        window.Present();
    }
    inline void EndSuccess(const uint8_t* keystates, const MouseState& state)
    {
        if(retry.clicked(state.x, state.y, state.buttons & SDL_BUTTON(1)))
        {
            Restart();
            currentState = GameState::GameLoop;
        }
        if(home.clicked(state.x, state.y, state.buttons & SDL_BUTTON(1))) 
            currentState = GameState::MainMenu;
        window.Clear(0xFFFFFFFF);
        window.DrawText({200, 40, 600, 92}, "Wanna Replay?", 0xFF000000);
        home.render(window);
        retry.render(window);
        window.Present();
    }
    inline void EndFail(const uint8_t* keystates, const MouseState& state)
    {
        if(retry.clicked(state.x, state.y, state.buttons & SDL_BUTTON(1)))
        {
            Restart();
            currentState = GameState::GameLoop;
        }
        if(home.clicked(state.x, state.y, state.buttons & SDL_BUTTON(1))) 
            currentState = GameState::MainMenu;
        window.Clear(0xFFFFFFFF);
        window.DrawText({250, 40, 550, 92}, "Try Again...", 0xFF000000);
        retry.render(window);
        home.render(window);
        window.Present();
    }
    inline void GameLoop(const uint8_t* keystates, const MouseState& mouseState)
    {
        while(enemies.size() < 4)
        {
            SpawnEnemy((pShape)rand(0, 3), 
            {
                rand(0.0f, (float)window.GetWidth()),
                rand(0.0f, (float)window.GetHeight())
            });
        }

        player.cooldown--;

        if(keystates[SDL_SCANCODE_W] && player.rect.position.y - player.rect.height * 0.5 - player.velocity > 0) 
            player.rect.position.y -= player.velocity;
        if(keystates[SDL_SCANCODE_S] && player.rect.position.y + player.rect.height * 0.5 + player.velocity < window.GetHeight())
            player.rect.position.y += player.velocity;
        if(keystates[SDL_SCANCODE_A] && player.rect.position.x - player.rect.width * 0.5 - player.velocity > 0)
            player.rect.position.x -= player.velocity;
        if(keystates[SDL_SCANCODE_D] && player.rect.position.x + player.rect.width * 0.5 + player.velocity < window.GetWidth()) 
            player.rect.position.x += player.velocity;

        ps.Update(8);

        if(player.cooldown <= 0 && (mouseState.buttons & SDL_BUTTON(1)))
        {
            SpawnMissile(player.rect.position, v2f(mouseState.x, mouseState.y), 0xFF0000FF, 600.0f);
            player.cooldown = 20;
        }

        for(auto& m : missiles)
        {
            stats.MissilesFired++;
            ps.position = m.triangle.position;
            ps.Generate(smoke, 6, pMode::Normal, pShape::Pixel,
            pBehaviour::Directional, 0.0f, 0.0f, 10);
            m.triangle.position.x += cos(m.angle) * m.velocity;
            m.triangle.position.y += sin(m.angle) * m.velocity;
            m.triangle.SetRotation(m.angle - pi * 0.5f);
            m.distance_current += m.velocity;
            float expRadius = 1.0f;

            if(m.distance_current > m.distance_max)
            {
                expRadius = 2.5f;
                ExplodeMissile(m);
            }

            if(m.triangle.color == 0xFFFF0000 && std::hypot(m.triangle.position.x - player.rect.position.x,
                m.triangle.position.y - player.rect.position.y) < player.rect.width * 0.6 * expRadius)
            {
                player.health -= 2;
                ExplodeMissile(m);
            }

            if(m.triangle.color == 0xFF0000FF)
                for(auto& enemy : enemies)
                    if(std::hypot(m.triangle.position.x - enemy.shape->position.x,
                        m.triangle.position.y - enemy.shape->position.y) < 30 * expRadius)
                    {
                        stats.MissilesHit++;
                        enemy.health -= 10;
                        ExplodeMissile(m);
                    }
        }

        for(auto& enemy : enemies){
            float x = player.rect.position.x - enemy.shape->position.x;
            float y = player.rect.position.y - enemy.shape->position.y;
            float dist = std::hypot(x, y);
            float angle = atan2(y, x);
            enemy.shape->SetRotation(angle - pi * 0.5);
            
            if(dist > 400.0f)
            {
                enemy.shape->position.x += cos(angle) * enemy.velocity;
                enemy.shape->position.y += sin(angle) * enemy.velocity;
            }
            
            if(dist <= 400.0f && enemy.cooldown <= 0)
            {
                SpawnMissile(enemy.shape->position, player.rect.position, 0xFFFF0000, 350.0f);
                enemy.cooldown = 150;
            }
            
            enemy.cooldown--;
            
            if(enemy.health <= 0)
            {
                stats.EnemiesKilled++;
                kill.colors[0] = enemy.shape->color;
                ps.position = enemy.shape->position;
                ps.Generate(kill, 15, pMode::Normal, enemy.data,
                pBehaviour::Directional, 0.0f, 70.0f, 65);
                enemy.remove = true;
                delete enemy.shape;
                enemy.shape = nullptr;
            }
        }

        for(auto& s : seeds)
        {
            if(s.position.x < player.rect.position.x + player.rect.width * 0.5 &&
            s.position.x > player.rect.position.x - player.rect.width * 0.5 && 
            s.position.y < player.rect.position.y + player.rect.height * 0.5 &&
            s.position.y > player.rect.position.y - player.rect.height * 0.5 && !s.remove)
            {
                player.rect.width += 2.5f;
                player.rect.height += 2.5f;
                player.velocity += 0.01f;
                stats.SeedsCollected++;
                s.remove = true;
            }
        }

        if(seeds.empty())
            currentState = GameState::EndSuccess;

        if(player.health <= 0.0f)
        {
            currentState = GameState::EndFail;
            stats.PlayerDeaths++;
        }

        enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](Enemy& e){return e.remove;}), enemies.end());
        missiles.erase(std::remove_if(missiles.begin(), missiles.end(), [](Missile& m){return m.remove;}), missiles.end());
        seeds.erase(std::remove_if(seeds.begin(), seeds.end(), [](seed& s){return s.remove;}), seeds.end());

        window.Clear(0xFFFFFF00);

        for(auto& s : seeds)
            window.DrawCircle(0xFFFF00FF, s.position.x, s.position.y, 3.0f);

        ps.Draw(window);

        for(auto& enemy : enemies)
            enemy.shape->Draw(window);

        for(auto& missile : missiles)
            missile.triangle.Draw(window);

        player.rect.Draw(window);

        window.DrawText(10, 10, "HEALTH:" + std::to_string(player.health), 2);

        window.DrawText({650, 10, 790, 36}, "SEEDS:" + std::to_string(seeds.size()));

        window.Present();
    }
    inline void Loop()
    {
        while (!window.bShouldClose) 
        {
            SDL_Event e;
            while (SDL_PollEvent(&e))
            {
                switch (e.type) 
                {
                    case SDL_QUIT:
                        window.bShouldClose = true;
                    break;
                }
            }

            MouseState state;
            const uint8_t* keystates = SDL_GetKeyboardState(NULL);
            state.buttons = SDL_GetMouseState(&state.x, &state.y);
            if(keystates[SDL_SCANCODE_P])
                TakeScreenShot(window, captures.directory + captures.prefix + std::to_string(captures.count++) + ".png");
            UpdateAndDraw(keystates, state);
        }
    }
    inline void End()
    {
        savefile["Enemies"]["Killed"].SetNumber(stats.EnemiesKilled);
        savefile["Enemies"]["Spawned"].SetNumber(stats.EnemiesSpawned);
        savefile["Missiles"]["Fired"].SetNumber(stats.MissilesFired);
        savefile["Missiles"]["Hit"].SetNumber(stats.MissilesHit);
        savefile["Player Deaths"].SetNumber(stats.PlayerDeaths);
        savefile["Seeds Collected"].SetNumber(stats.SeedsCollected);
        savefile["Captures"]["Count"].SetNumber(captures.count);
        savefile["Captures"]["Directory"].SetString(captures.directory);
        savefile["Captures"]["Prefix"].SetString(captures.prefix);
        Serialize(savefile, "datafile.txt");
        window.~Window();
    }
};

int main(int argc, char** argv) 
{
    Game instance;
    instance.Start();
    instance.Loop();
    instance.End();
    return 0;
}