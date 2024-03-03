#ifndef WINDOW_H
#define WINDOW_H

#include "includes.h"

inline float CharSize(const char c, float size)
{
    if(c == '\t')
        return TAB_SPACE * size;
    else
        return (FONT_WIDTH + 1) * size;
}

inline v2f StringSize(const std::string& text, float size)
{
    v2f stringSize = v2f(0, FONT_HEIGHT);
    float buffer = 0;
    for(auto& c : text)
    {
        if(c == '\n')
        {
            stringSize.x = std::max(stringSize.x, buffer);
            stringSize.y += FONT_HEIGHT + 1;
            buffer = 0;
        }
        else
            buffer += CharSize(c, size);
    }
    return v2f(std::max(stringSize.x, buffer), stringSize.y * size);
}

inline uint32_t LerpColor(uint32_t color1, uint32_t color2, float fraction)
{
    uint8_t red1 = color1 & 0xFF;
    uint8_t red2 = color2 & 0xFF;
    uint8_t green1 = (color1 >> 8) & 0xFF;
    uint8_t green2 = (color2 >> 8) & 0xFF;
    uint8_t blue1 = (color1 >> 16) & 0xFF;
    uint8_t blue2 = (color2 >> 16) & 0xFF;
    return 0xFF << 24 | 
    (int)((blue2 - blue1) * fraction + blue1) << 16 | 
    (int)((green2 - green1) * fraction + green1) << 8 | 
    (int)((red2 - red1) * fraction + red1);
}

enum class DrawMode
{
    Normal,
    Periodic,
    Clamp
};

enum class PixelMode
{
    Normal,
    Mask
};

enum class hDirection
{
    Norm,
    Flip
};

enum class vDirection
{
    Norm,
    Flip
};

struct Sprite
{
    DrawMode drawMode = DrawMode::Normal;
    std::vector<uint32_t> data;
    int width, height;
    Sprite() = default;
    Sprite(const std::string& path);
    void SetPixel(uint32_t color, int x, int y);
    uint32_t GetPixel(int x, int y);
};

struct rect
{
    float sx;
    float sy;
    float ex;
    float ey;
};

struct vertex
{
    v2f coord;
    v2f tex;
#if defined VERTEX_COLOR
    uint32_t color;
#endif
};

struct Camera
{
    v2f position = v2f(0);
    bool enabled = false;
};

struct Window
{
    Camera camera;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* surface;
    std::string name;
    std::vector<Sprite> drawTargets;
    int currentDrawTarget;
    bool shouldClose;
    PixelMode pixelMode;
    void Init(std::string name, int width, int height);
    void CreateWindow(std::string name, int width, int height);
    void CreateRenderer();
    void CreateSurface();
    void Present();
    int GetWidth();
    int GetHeight();
    void Clear(uint32_t color);
    void SetDrawMode(DrawMode drawMode);
    void SetPixel(uint32_t color, int x, int y);
    uint32_t GetPixel(int x, int y);
    void DrawLine(uint32_t color, int x0, int y0, int x1, int y1);
    void DrawRect(uint32_t color, int sx, int sy, int ex, int ey);
    void DrawRectOutline(uint32_t color, int sx, int sy, int ex, int ey);
    void DrawRotatedRectOutline(uint32_t color, int sx, int sy, int ex, int ey, float rotation);
    void DrawCircle(uint32_t color, int cx, int cy, int radius);
    void DrawCircleOutline(uint32_t color, int cx, int cy, int radius);
    void DrawTriangle(uint32_t color, int x1, int y1, int x2, int y2, int x3, int y3);
    void DrawTexturedTriangle(Sprite& sprite, vertex v1, vertex v2, vertex v3);
    void DrawTriangleOutline(uint32_t color, int x1, int y1, int x2, int y2, int x3, int y3);
    void DrawSprite(Sprite& sprite, Transform& transform, hDirection hor = hDirection::Norm, vDirection ver = vDirection::Norm);
    void DrawSprite(int x, int y, Sprite& sprite, float size = 1, hDirection hor = hDirection::Norm, vDirection ver = vDirection::Norm);
    void DrawSprite(int x, int y, rect dst, Sprite& sprite, float size = 1, hDirection hor = hDirection::Norm, vDirection ver = vDirection::Norm);
    void DrawSprite(rect dst, Sprite& sprite, hDirection hor = hDirection::Norm, vDirection ver = vDirection::Norm);
    void DrawSprite(rect dst, rect src, Sprite& sprite, hDirection hor = hDirection::Norm, vDirection ver = vDirection::Norm);
    void DrawCharacter(int x, int y, const char c, float size = 1, uint32_t color = 0xFF000000);
    void DrawCharacter(rect dst, const char c, uint32_t color = 0xFF000000);
    void DrawText(int x, int y, const std::string& text, float size = 1, uint32_t color = 0xFF000000);
    void DrawText(rect dst, const std::string& text, uint32_t color = 0xFF000000);
    ~Window()
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_DestroyTexture(surface);
        SDL_Quit();
    }
};

struct Button
{
    Sprite image;
    v2f position;
    float size = 1;
    Button() = default;
    Button(const std::string& path);
    bool clicked(int x, int y, bool clicked);
    bool hover(int x, int y);
    void render(Window& window);
    ~Button() {}
};

struct SpriteSheet
{
    Sprite sprite;
    int cellWidth, cellHeight;
    SpriteSheet() = default;
    SpriteSheet(const std::string& path, int cw, int ch);
    rect GetSubImage(int cx, int cy);
    void Draw(Window& window, int x, int y, float size, int cx, int cy, hDirection hor = hDirection::Norm, vDirection ver = vDirection::Norm);
    ~SpriteSheet() {}
};

#endif

#ifdef WINDOW_H
#undef WINDOW_H

Sprite::Sprite(const std::string& path)
{
    SDL_Surface* image = IMG_Load(path.c_str());
    SDL_Surface* converted = SDL_ConvertSurfaceFormat(image, SDL_PIXELFORMAT_RGBA32, 0);
    this->width = converted->w;
    this->height = converted->h;
    const int size = width * height;
    data.resize(size);
    memcpy(data.data(), converted->pixels, size*4);
    SDL_FreeSurface(image);
    SDL_FreeSurface(converted);
    image = nullptr;
    converted = nullptr;
}

void Sprite::SetPixel(uint32_t color, int x, int y)
{
    switch(drawMode)
    {
        case DrawMode::Normal:
        {
            if(x < 0 || x >= width || y < 0 || y >= height) 
                return;
        }
        break;
        case DrawMode::Periodic:
        {
            x = x % width;
            y = y % height;
            x += (x < 0) ? width : 0;
            y += (y < 0) ? height : 0;
        }
        break;
    }
    data[width * y + x] = color;
}

uint32_t Sprite::GetPixel(int x, int y)
{
    switch(drawMode)
    {
        case DrawMode::Normal:
        {
            if(x < 0 || x >= width || y < 0 || y >= height) 
                return 0x00000000;
        }
        break;
        case DrawMode::Periodic:
        {
            x = x % width;
            y = y % height;
            x += (x < 0) ? width : 0;
            y += (y < 0) ? height : 0;
        }
        break;
        case DrawMode::Clamp:
        {
            x = std::clamp(x, 0, width);
            y = std::clamp(y, 0, height);
        }
        break;
    }
    return data[width * y + x];
}

void Window::Init(std::string name, int width, int height)
{
    CreateWindow(name, width, height);
    CreateRenderer();
    CreateSurface();
}

void Window::CreateWindow(std::string name, int width, int height)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    Sprite drawTarget;
    drawTarget.width = width;
    drawTarget.height = height;
    drawTargets.push_back(std::move(drawTarget));
    currentDrawTarget = 0;
    window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    shouldClose = false;
    pixelMode = PixelMode::Normal;
}

void Window::CreateRenderer()
{
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    SDL_RenderSetLogicalSize(renderer, GetWidth(), GetHeight());
}

void Window::CreateSurface()
{
    const int w = GetWidth();
    const int h = GetHeight();
    surface = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, w, h);
    drawTargets[currentDrawTarget].data.resize(w * h);
    memset(drawTargets[currentDrawTarget].data.data(), 0, 4 * w * h);
}

void Window::Clear(uint32_t color)
{
    for(int i = 0; i < GetWidth() * GetHeight(); i++)
        drawTargets[currentDrawTarget].data[i] = color;
}

void Window::Present()
{
    int pitch;
    void* buffer;
    SDL_LockTexture(surface, NULL, &buffer, &pitch);
    memcpy(buffer, drawTargets[currentDrawTarget].data.data(), 4 * GetWidth() * GetHeight());
    SDL_UnlockTexture(surface);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, surface, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void Window::SetPixel(uint32_t color, int x, int y)
{
    if(pixelMode == PixelMode::Mask && (color >> 24 & 0xFF) == 0) return;
    drawTargets[currentDrawTarget].SetPixel(color, 
        x - (camera.enabled ? camera.position.x : 0),
        y - (camera.enabled ? camera.position.y : 0)
    );
}

uint32_t Window::GetPixel(int x, int y)
{
    return drawTargets[currentDrawTarget].GetPixel(x, y);
}

void Window::SetDrawMode(DrawMode drawMode)
{
    drawTargets[currentDrawTarget].drawMode = drawMode;
}

int Window::GetWidth()
{
    return drawTargets[currentDrawTarget].width;
}

int Window::GetHeight()
{
    return drawTargets[currentDrawTarget].height;
}

void Window::DrawLine(uint32_t color, int x0, int y0, int x1, int y1)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int absdx = abs(dx);
    int absdy = abs(dy);
    int x = x0;
    int y = y0;
    if(absdx > absdy) 
    {
        int d = absdy * 2 - absdx;
        for(int i = 0; i < absdx; i++) 
        {
            x = dx < 0 ? x - 1: x + 1;
            if(d < 0)
                d = d + 2*absdy;
            else 
            {
                y = dy < 0 ? y - 1 : y + 1;
                d = d + 2 * (absdy - absdx); 
            }
            SetPixel(color, x, y);
        }
    } 
    else 
    {
        int d = 2 * absdx - absdy;
        for(int i = 0; i < absdy ; i++)
        {
            y = dy < 0 ? y - 1 : y + 1;
            if(d < 0)
                d = d + 2 * absdx;
            else
            {
                x = dx < 0 ? x - 1 : x + 1;
                d = d + 2 * (absdx - absdy);
            }
            SetPixel(color, x, y);
        }
    }
}

void Window::DrawRect(uint32_t color, int sx, int sy, int ex, int ey)
{
    if(sx > ex) std::swap(ex, sx);
    if(sy > ey) std::swap(ey, sy);
    for(int x = sx; x < ex; x++)
        for(int y = sy; y < ey; y++)
            SetPixel(color, x, y);
}

void Window::DrawRectOutline(uint32_t color, int sx, int sy, int ex, int ey)
{
    DrawLine(color, sx, sy, sx, ey);
    DrawLine(color, sx, sy, ex, sy);
    DrawLine(color, ex, ey, sx, ey);
    DrawLine(color, ex, ey, ex, sy);
}

void Window::DrawRotatedRectOutline(uint32_t color, int sx, int sy, int ex, int ey, float rotation)
{
    if(rotation == 0.0f)
    {
        DrawRectOutline(color, sx, sy, ex, ey);
        return;
    }
    v2f p1 = rotate(rotation, v2f(sx, sy));
    v2f p2 = rotate(rotation, v2f(sx, ey));
    v2f p3 = rotate(rotation, v2f(ex, ey));
    v2f p4 = rotate(rotation, v2f(ex, sy));
    DrawLine(color, p1.x, p1.y, p2.x, p2.y);
    DrawLine(color, p1.x, p1.y, p4.x, p4.y);
    DrawLine(color, p3.x, p3.y, p2.x, p2.y);
    DrawLine(color, p3.x, p3.y, p4.x, p4.y);
}

void Window::DrawCircle(uint32_t color, int cx, int cy, int radius)
{
    auto drawLine = [&](int sx, int ex, int y)
    {
        for(int x = sx; x < ex; x++) 
            SetPixel(color, x, y);
    };
    const int r2 = radius * radius;
    for(int py = -radius; py < radius; py++)
    {
        int px = (int)sqrt((r2 - py * py) + 0.5);
        int y = cy + py;
        drawLine(cx - px, cx + px, y);
    }
}

void Window::DrawCircleOutline(uint32_t color, int cx, int cy, int radius)
{
    auto drawPixels = [&](int x, int y)
    {
        SetPixel(color, cx+x, cy+y); 
        SetPixel(color, cx-x, cy+y); 
        SetPixel(color, cx+x, cy-y); 
        SetPixel(color, cx-x, cy-y); 
        SetPixel(color, cx+y, cy+x); 
        SetPixel(color, cx-y, cy+x); 
        SetPixel(color, cx+y, cy-x); 
        SetPixel(color, cx-y, cy-x); 
    };
    float t1 = radius / 16;
    int x = radius, y = 0;
    while(y < x)
    {
        drawPixels(x, y);
        t1 += ++y;
        if(t1 >= x) t1 -= x--;
    }
}

void Window::DrawTriangle(uint32_t color, int x1, int y1, int x2, int y2, int x3, int y3)
{
    auto drawLine = [&](int sx, int ex, int y)
    {
        if(sx > ex) std::swap(sx, ex); 
        for(int x = sx; x < ex; x++)
            SetPixel(color, x, y);
    };
    if(y2 < y1) 
    {
        std::swap(y1, y2); 
        std::swap(x1, x2);
    }
    if(y3 < y1) 
    {   
        std::swap(y1, y3); 
        std::swap(x1, x3);
    }
    if(y3 < y2) 
    {
        std::swap(y3, y2); 
        std::swap(x3, x2);
    }
    float far = (x3 - x1) / (y3 - y1 + 1.0);
    float upper = (x2 - x1) / (y2 - y1 + 1.0);
    float low = (x3 - x2) / (y3 - y2 + 1.0);
    float start = x1;
    float end = x1 + upper;
    for(int y = y1; y <= y3; y++)
    {
        drawLine((int)start, (int)end, y);
        start += far;
        end += (y < y2) ? upper : low;
    }
}

void Window::DrawTexturedTriangle(Sprite& sprite, vertex v1, vertex v2, vertex v3)
{
    const int w = sprite.width;
    const int h = sprite.height;
#if defined VERTEX_COLOR
    auto drawLine = [&](int sx, int ex, int y, float tsx, float tex, float tsy, float tey, uint32_t sc, uint32_t ec)
#else
    auto drawLine = [&](int sx, int ex, int y, float tsx, float tex, float tsy, float tey)
#endif
    {
        if(ex < sx)
        {
            std::swap(sx, ex);
            std::swap(tsx, tex);
            std::swap(tsy, tey);
        }
        float dx = 1.0f / (ex - sx), curr = 0.0f;
        for(int x = sx; x < ex; x++)
        {
            float u = (tsx + curr * (tex - tsx)) * w;
            float v = (tsy + curr * (tey - tsy)) * h;
#if defined VERTEX_COLOR
            uint32_t color = LerpColor(sc, ec, curr);
            SetPixel(LerpColor(color, sprite.GetPixel((int)u, (int)v), 0.5f), x, y);
#else
            SetPixel(sprite.GetPixel((int)u, (int)v), x, y);
#endif
            curr += dx;
        }
    };
    if(v2.coord.y < v1.coord.y) std::swap(v1, v2);
    if(v3.coord.y < v1.coord.y) std::swap(v1, v3);
    if(v3.coord.y < v2.coord.y) std::swap(v2, v3);
    float dx1 = (v3.coord.x - v1.coord.x) / (v3.coord.y - v1.coord.y + 1.0);
    float dx2 = (v3.coord.x - v2.coord.x) / (v3.coord.y - v2.coord.y + 1.0);
    float dx3 = (v2.coord.x - v1.coord.x) / (v2.coord.y - v1.coord.y + 1.0);
    float du1 = (v3.tex.x - v1.tex.x) / (v3.coord.y - v1.coord.y + 1.0);
    float du2 = (v3.tex.x - v2.tex.x) / (v3.coord.y - v2.coord.y + 1.0);
    float du3 = (v2.tex.x - v1.tex.x) / (v2.coord.y - v1.coord.y + 1.0);
    float dv1 = (v3.tex.y - v1.tex.y) / (v3.coord.y - v1.coord.y + 1.0);
    float dv2 = (v3.tex.y - v2.tex.y) / (v3.coord.y - v2.coord.y + 1.0);
    float dv3 = (v2.tex.y - v1.tex.y) / (v2.coord.y - v1.coord.y + 1.0);
    float sx = v1.coord.x, ex = v1.coord.x + dx3;
    float tsx = v1.tex.x, tex = v1.tex.x + du3;
    float tsy = v1.tex.y, tey = v1.tex.y + dv3;
#if defined VERTEX_COLOR
    float dy = 1.0f / (v3.coord.y - v1.coord.y);
    float curr = 0.0f;
#endif
    for(int y = v1.coord.y; y <= v3.coord.y; y++)
    {
#if defined VERTEX_COLOR
        curr += dy;
        uint32_t sc = LerpColor(v1.color, v3.color, curr);
        uint32_t ec = LerpColor(v2.color, (y < v2.coord.y) ? v3.color : v1.color, curr);
        drawLine(sx, ex, y, tsx, tex, tsy, tey, sc, ec);
#else
        drawLine(sx, ex, y, tsx, tex, tsy, tey);
#endif
        sx += dx1;
        tsx += du1;
        tsy += dv1;
        ex += y < v2.coord.y ? dx3 : dx2;
        tex += y < v2.coord.y ? du3 : du2;
        tey += y < v2.coord.y ? dv3 : dv2;
    }
}

void Window::DrawTriangleOutline(uint32_t color, int x1, int y1, int x2, int y2, int x3, int y3)
{
    DrawLine(color, x1, y1, x2, y2);
    DrawLine(color, x1, y1, x3, y3);
    DrawLine(color, x2, y2, x3, y3);
}

void Window::DrawSprite(Sprite& sprite, Transform& transform, hDirection hor, vDirection ver)
{
    float ex, ey;
    float sx, sy;
    float px, py;
    transform.Forward(0.0f, 0.0f, sx, sy);
    px = sx; py = sy;
    sx = std::min(sx, px); sy = std::min(sy, py);
    ex = std::max(ex, px); ey = std::max(ey, py);
    transform.Forward((float)sprite.width, (float)sprite.height, px, py);
    sx = std::min(sx, px); sy = std::min(sy, py);
    ex = std::max(ex, px); ey = std::max(ey, py);
    transform.Forward(0.0f, (float)sprite.height, px, py);
    sx = std::min(sx, px); sy = std::min(sy, py);
    ex = std::max(ex, px); ey = std::max(ey, py);
    transform.Forward((float)sprite.width, 0.0f, px, py);
    sx = std::min(sx, px); sy = std::min(sy, py);
    ex = std::max(ex, px); ey = std::max(ey, py);
    transform.Invert();
    if (ex < sx) std::swap(ex, sx);
    if (ey < sy) std::swap(ey, sy);
    for (float i = sx; i < ex; ++i)
        for (float j = sy; j < ey; ++j)
        {
            float ox, oy;
            transform.Backward(i, j, ox, oy);
            int u = hor == hDirection::Flip ? sprite.width - ceil(ox) : floor(ox);
            int v = ver == vDirection::Flip ? sprite.height - ceil(oy) : floor(oy);
            this->SetPixel(sprite.GetPixel(u, v), (int)i, (int)j);
        }
}

void Window::DrawSprite(int x, int y, Sprite& sprite, float size, hDirection hor, vDirection ver)
{
    rect dst;
    dst.sx = x - sprite.width * size * 0.5f;
    dst.sy = y - sprite.height * size * 0.5f;
    dst.ex = x + sprite.width * size * 0.5f;
    dst.ey = y + sprite.height * size * 0.5f;
    DrawSprite(dst, sprite, hor, ver);
}

void Window::DrawSprite(int x, int y, rect src, Sprite& sprite, float size, hDirection hor, vDirection ver)
{
    if(src.ex == src.sx || src.ey == src.sy) return;
    if(src.ex < src.sx) std::swap(src.ex, src.sx);
    if(src.ey < src.sy) std::swap(src.ey, src.sy);
    rect dst;
    dst.sx = x - (src.ex - src.sx) * 0.5f * size;
    dst.sy = y - (src.ey - src.sy) * 0.5f * size;
    dst.ex = x + (src.ex - src.sx) * 0.5f * size;
    dst.ey = y + (src.ey - src.sy) * 0.5f * size;
    DrawSprite(dst, src, sprite, hor, ver);
}

void Window::DrawSprite(rect dst, Sprite& sprite, hDirection hor, vDirection ver)
{
    if(dst.ex == dst.sx || dst.ey == dst.sy) return;
    if(dst.ex < dst.sx) std::swap(dst.sx, dst.ex);
    if(dst.ey < dst.sy) std::swap(dst.sy, dst.ey);
    float xScale = (dst.ex - dst.sx) / sprite.width;
    float yScale = (dst.ey - dst.sy) / sprite.height;
    float px = hor == hDirection::Flip ? -1 : 1;
    float dx = hor == hDirection::Flip ? dst.ex : dst.sx;
    float py = ver == vDirection::Flip ? -1 : 1;
    float dy = ver == vDirection::Flip ? dst.ey : dst.sy;
    for(float x = 0; x < dst.ex - dst.sx; x++)
        for(float y = 0; y < dst.ey - dst.sy; y++)
        {
            int ox = floor(x / xScale);
            int oy = floor(y / yScale);
            this->SetPixel(sprite.GetPixel(ox, oy), (int)(dx + x * px), (int)(dy + y * py));
        }
}

void Window::DrawSprite(rect dst, rect src, Sprite& sprite, hDirection hor, vDirection ver)
{
    if(dst.ex == dst.sx || dst.ey == dst.sy || src.ex == src.sx || src.ey == src.sy) return;
    if(dst.ex < dst.sx) std::swap(dst.sx, dst.ex);
    if(dst.ey < dst.sy) std::swap(dst.sy, dst.ey);
    if(src.ex < src.sx) std::swap(src.sx, src.ex);
    if(src.ey < src.sy) std::swap(src.sy, src.ey);
    float xScale = (dst.ex - dst.sx) / (src.ex - src.sx);
    float yScale = (dst.ey - dst.sy) / (src.ey - src.sy);
    float px = hor == hDirection::Flip ? -1 : 1;
    float dx = hor == hDirection::Flip ? dst.ex : dst.sx;
    float py = ver == vDirection::Flip ? -1 : 1;
    float dy = ver == vDirection::Flip ? dst.ey : dst.sy;
    for(float x = 0; x < dst.ex - dst.sx; x++)
        for(float y = 0; y < dst.ey - dst.sy; y++)
        {
            int ox = floor(x / xScale);
            int oy = floor(y / yScale);
            this->SetPixel(sprite.GetPixel(src.sx + ox, src.sy + oy), (int)(dx + x * px), (int)(dy + y * py));
        }
}

void Window::DrawCharacter(int x, int y, const char c, float size, uint32_t color)
{
    rect dst;
    dst.sx = (float)x;
    dst.sy = (float)y;
    dst.ex = dst.sx + CharSize(c, size);
    dst.ey = dst.sy + FONT_HEIGHT * size;
    DrawCharacter(dst, c, color);
}

void Window::DrawText(int x, int y, const std::string& text, float size, uint32_t color)
{
    v2f stringSize = StringSize(text, size);
    rect dst;
    dst.sx = (float)x;
    dst.sy = (float)y;
    dst.ex = dst.sx + stringSize.x;
    dst.ey = dst.sy + stringSize.y;
    DrawText(dst, text, color);
}

void Window::DrawCharacter(rect dst, const char c, uint32_t color)
{
    if(dst.ex == dst.sx || dst.sy == dst.ey) return;
    if(dst.ex < dst.sx) std::swap(dst.ex, dst.sx);
    if(dst.ey < dst.sy) std::swap(dst.ey, dst.sy);
    float xScale = (dst.ex - dst.sx) / FONT_WIDTH;
    float yScale = (dst.ey - dst.sy) / FONT_HEIGHT;
    for(float x = 0; x < dst.ex - dst.sx; x++)
        for(float y = 0; y < dst.ey - dst.sy; y++)
        {
            int ox = floor(x / xScale);
            int oy = floor(y / yScale);
            if(fontData[(int)c - 32][oy] & (1 << ox))
            {
                SetPixel(color, (int)(dst.sx + (FONT_WIDTH * xScale - x)), (int)(dst.sy + (FONT_HEIGHT * yScale - y)));        
            }
        }
}

void Window::DrawText(rect dst, const std::string& text, uint32_t color)
{
    if(dst.ex == dst.sx || dst.sy == dst.ey || text.empty()) return;
    if(dst.ex < dst.sx) std::swap(dst.ex, dst.sx);
    if(dst.ey < dst.sy) std::swap(dst.ey, dst.sy);
    v2f stringSize = StringSize(text, 1.0f);
    float xScale = (dst.ex - dst.sx) / stringSize.x;
    float yScale = (dst.ey - dst.sy) / stringSize.y;
    float sx = dst.sx, sy = dst.sy;
    for(auto c : text)
    {
        DrawCharacter({sx, sy, sx + xScale * FONT_WIDTH, sy + yScale * FONT_HEIGHT}, c, color);
        if(c == '\n')
        {
            sy += (FONT_HEIGHT + 1) * yScale;
            sx = dst.sx;
        }
        else
            sx += CharSize(c, xScale);
    }
}

SpriteSheet::SpriteSheet(const std::string& path, int cw, int ch)
{
    sprite = Sprite(path);
    cellWidth = cw;
    cellHeight = ch;
}

rect SpriteSheet::GetSubImage(int cx, int cy)
{
    rect rc;
    rc.sx = cx * cellWidth;
    rc.ex = rc.sx + cellWidth;
    rc.sy = cy * cellHeight;
    rc.ey = rc.sy + cellHeight;
    return rc;
}

void SpriteSheet::Draw(Window& window, int x, int y, float size, int cx, int cy, hDirection hor, vDirection ver)
{
    window.DrawSprite(x, y, GetSubImage(cx, cy), this->sprite, size, hor, ver);
}

Button::Button(const std::string& path)
{
    image = Sprite(path);
}

bool Button::clicked(int x, int y, bool clicked)
{
    return clicked && hover(x, y);
}

bool Button::hover(int x, int y)
{
    const int w = this->image.width * size;
    const int h = this->image.height * size;
    return (x < position.x + w * 0.5f && x > position.x - w * 0.5f && y < position.y + h * 0.5f && y > position.y - h * 0.5f);
}

void Button::render(Window& window)
{
    window.pixelMode = PixelMode::Mask;
    window.DrawSprite(position.x, position.y, image, size);
    window.pixelMode = PixelMode::Normal;
}

#endif