#ifndef WINDOW_H
#define WINDOW_H

#include "includes.h"

inline float char_size(const char c, float size)
{
    if(c == '\t')
        return TAB_SPACE * size;
    else
        return (FONT_WIDTH + 1) * size;
}

inline float string_size_x(const std::string& text, float size)
{
    float max_size = 0, buff = 0;
    for (auto &c : text) 
    {
	    if (c == '\n')
        { 
            max_size = std::max(max_size, buff); 
            buff = 0;
        }
	    else 
        	buff += char_size(c, size);
    }
    max_size = std::max(max_size, buff);
    return max_size;
}

inline float string_size_y(const std::string& text, float size)
{
    int count = std::count(text.begin(), text.end(), '\n');
    return (count * (FONT_HEIGHT + 1) + FONT_HEIGHT) * size;
}

enum class DrawMode
{
    Normal,
    Periodic,
    Clamp
};

enum PixelMode
{
    Normal,
    Mask
};

struct Sprite
{
    std::vector<uint32_t> data;
    int width, height;
    Sprite() = default;
    Sprite(const std::string& path);
    void SetPixel(uint32_t color, int x, int y, DrawMode drawMode = DrawMode::Normal);
    uint32_t GetPixel(int x, int y, DrawMode drawMode = DrawMode::Normal);
};

struct rect
{
    float sx;
    float sy;
    float ex;
    float ey;
};

struct Window
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* surface;
    std::string name;
    std::vector<Sprite> drawTargets;
    int currentDrawTarget;
    bool bShouldClose;
    PixelMode pixelMode;
    void Init(std::string name, int width, int height);
    void CreateWindow(std::string name, int width, int height);
    void CreateRenderer();
    void CreateSurface();
    void Clear(uint32_t color);
    void Present();
    int GetWidth();
    int GetHeight();
    void SetPixel(uint32_t color, int x, int y, DrawMode drawMode = DrawMode::Normal);
    uint32_t GetPixel(int x, int y, DrawMode drawMode = DrawMode::Normal);
    void DrawLine(uint32_t color, int x0, int y0, int x1, int y1, DrawMode drawMode = DrawMode::Normal);
    void DrawRect(uint32_t color, int sx, int sy, int ex, int ey, DrawMode drawMode = DrawMode::Normal);
    void DrawRectOutline(uint32_t color, int sx, int sy, int ex, int ey, DrawMode drawMode = DrawMode::Normal);
    void DrawRotatedRectOutline(uint32_t color, int sx, int sy, int ex, int ey, float rotation, DrawMode drawMode = DrawMode::Normal);
    void DrawCircle(uint32_t color, int cx, int cy, int radius, DrawMode drawMode = DrawMode::Normal);
    void DrawCircleOutline(uint32_t color, int cx, int cy, int radius, DrawMode drawMode = DrawMode::Normal);
    void DrawTriangle(uint32_t color, int x1, int y1, int x2, int y2, int x3, int y3, DrawMode drawMode = DrawMode::Normal);
    void DrawTriangleOutline(uint32_t color, int x1, int y1, int x2, int y2, int x3, int y3, DrawMode drawMode = DrawMode::Normal);
    void DrawSprite(Sprite& sprite, Transform& transform, DrawMode drawMode = DrawMode::Normal);
    void DrawSprite(int x, int y, Sprite& sprite, float size = 1, DrawMode drawMode = DrawMode::Normal);
    void DrawSprite(int x, int y, rect dst, Sprite& sprite, float size = 1, DrawMode drawMode = DrawMode::Normal);
    void DrawSprite(rect dst, Sprite& sprite, DrawMode drawMode = DrawMode::Normal);
    void DrawSprite(rect dst, rect src, Sprite& sprite, DrawMode drawMode = DrawMode::Normal);
    void DrawCharacter(int x, int y, const char c, float size = 1, uint32_t color = 0xFF000000, DrawMode drawMode = DrawMode::Normal);
    void DrawCharacter(rect dst, const char c, uint32_t color = 0xFF000000, DrawMode drawMode = DrawMode::Normal);
    void DrawText(int x, int y, const std::string& text, float size = 1, uint32_t color = 0xFF000000, DrawMode drawMode = DrawMode::Normal);
    void DrawText(rect dst, const std::string& text, uint32_t color = 0xFF000000, DrawMode drawMode = DrawMode::Normal);
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
    void render(Window& window, DrawMode drawMode = DrawMode::Normal);
    ~Button() {}
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

void Sprite::SetPixel(uint32_t color, int x, int y, DrawMode drawMode)
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

uint32_t Sprite::GetPixel(int x, int y, DrawMode drawMode)
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
        case DrawMode::Clamp:
        {
            x = std::clamp(x, 0, width - 1);
            y = std::clamp(y, 0, height - 1);
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
    IMG_Init(IMG_INIT_PNG);
    IMG_Init(IMG_INIT_JPG);
    Sprite drawTarget;
    drawTarget.width = width;
    drawTarget.height = height;
    drawTargets.push_back(std::move(drawTarget));
    currentDrawTarget = 0;
    window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    bShouldClose = false;
    pixelMode = PixelMode::Normal;
}

void Window::CreateRenderer()
{
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    SDL_RenderSetLogicalSize(renderer, GetWidth(), GetHeight());
}

void Window::CreateSurface()
{
    surface = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, GetWidth(), GetHeight());
    drawTargets[currentDrawTarget].data.resize(GetWidth() * GetHeight());
    memset(drawTargets[currentDrawTarget].data.data(), 0, 4 * GetWidth() * GetHeight());
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

void Window::SetPixel(uint32_t color, int x, int y, DrawMode drawMode)
{
    if(pixelMode == PixelMode::Mask)
        if((color >> 24 & 0xFF) == 00)
            return;
    drawTargets[currentDrawTarget].SetPixel(color, x, y, drawMode);
}

uint32_t Window::GetPixel(int x, int y, DrawMode drawMode)
{
    return drawTargets[currentDrawTarget].GetPixel(x, y, drawMode);
}

int Window::GetWidth()
{
    return drawTargets[currentDrawTarget].width;
}

int Window::GetHeight()
{
    return drawTargets[currentDrawTarget].height;
}

void Window::DrawLine(uint32_t color, int x0, int y0, int x1, int y1, DrawMode drawMode)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int absdx = std::abs(dx);
    int absdy = std::abs(dy);
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
        SetPixel(color, x, y, drawMode);
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
        SetPixel(color, x, y, drawMode);
        }
    }
}

void Window::DrawRect(uint32_t color, int sx, int sy, int ex, int ey, DrawMode drawMode)
{
    if(sx > ex) std::swap(ex, sx);
    if(sy > ey) std::swap(ey, sy);
    for(int x = sx; x < ex; x++)
        for(int y = sy; y < ey; y++)
            SetPixel(color, x, y, drawMode);
}

void Window::DrawRectOutline(uint32_t color, int sx, int sy, int ex, int ey, DrawMode drawMode)
{
    DrawLine(color, sx, sy, sx, ey, drawMode);
	DrawLine(color, sx, sy, ex, sy, drawMode);
	DrawLine(color, ex, ey, sx, ey, drawMode);
	DrawLine(color, ex, ey, ex, sy, drawMode);
}

void Window::DrawRotatedRectOutline(uint32_t color, int sx, int sy, int ex, int ey, float rotation, DrawMode drawMode)
{
    if(rotation == 0.0f)
    {
        DrawRectOutline(color, sx, sy, ex, ey, drawMode);
        return;
    }
    v2f p1 = rotate(rotation, v2f(sx, sy));
    v2f p2 = rotate(rotation, v2f(sx, ey));
    v2f p3 = rotate(rotation, v2f(ex, ey));
    v2f p4 = rotate(rotation, v2f(ex, sy));
    DrawLine(color, p1.x, p1.y, p2.x, p2.y, drawMode);
    DrawLine(color, p1.x, p1.y, p4.x, p4.y, drawMode);
    DrawLine(color, p3.x, p3.y, p2.x, p2.y, drawMode);
    DrawLine(color, p3.x, p3.y, p4.x, p4.y, drawMode);
}

void Window::DrawCircle(uint32_t color, int cx, int cy, int radius, DrawMode drawMode)
{
    auto drawLine = [&](int sx, int ex, int y)
    {
        for(int x = sx; x < ex; x++) 
            SetPixel(color, x, y, drawMode);
    };
    const int r2 = radius * radius;
    for(int py = -radius; py < radius; py++)
    {
        int px = (int)std::sqrt((r2 - py * py) + 0.5);
        int y = cy + py;
        drawLine(cx - px, cx + px, y);
    }
}

void Window::DrawCircleOutline(uint32_t color, int cx, int cy, int radius, DrawMode drawMode)
{
    auto drawPixels = [&](int x, int y)
    {
        SetPixel(color, cx+x, cy+y, drawMode); 
        SetPixel(color, cx-x, cy+y, drawMode); 
        SetPixel(color, cx+x, cy-y, drawMode); 
        SetPixel(color, cx-x, cy-y, drawMode); 
        SetPixel(color, cx+y, cy+x, drawMode); 
        SetPixel(color, cx-y, cy+x, drawMode); 
        SetPixel(color, cx+y, cy-x, drawMode); 
        SetPixel(color, cx-y, cy-x, drawMode); 
    };
    float t1 = radius / 16;
    int x = radius, y = 0;
    while(y < x)
    {
        drawPixels(x, y);
        y++;
        t1 += y;
        if(t1 >= x)
        {
            t1 -= x;
            x--;
        }
    }
}

void Window::DrawTriangle(uint32_t color, int x1, int y1, int x2, int y2, int x3, int y3, DrawMode drawMode)
{
    auto drawLine = [&](int sx, int ex, int y)
    {
        if(sx > ex) std::swap(sx, ex); 
        for(int x = sx; x < ex; x++)
            SetPixel(color, x, y, drawMode);
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
    float far = (float)(x3 - x1) / (y3 - y1 + 1.0);
    float upper = (float)(x2 - x1) / (y2 - y1 + 1.0);
    float low = (float)(x3 - x2) / (y3 - y2 + 1.0);
    float start = x1;
    float end = x1 + upper;
    for(int y = y1; y <= y3; y++)
    {
        drawLine((int)start, (int)end, y);
        start += far;
        end += (y < y2) ? upper : low;
    }
}

void Window::DrawTriangleOutline(uint32_t color, int x1, int y1, int x2, int y2, int x3, int y3, DrawMode drawMode)
{
    DrawLine(color, x1, y1, x2, y2, drawMode);
    DrawLine(color, x1, y1, x3, y3, drawMode);
    DrawLine(color, x2, y2, x3, y3, drawMode);
}

void Window::DrawSprite(Sprite& sprite, Transform& transform, DrawMode drawMode)
{
    float ex, ey;
    float sx, sy;
    float px, py;
    const int tw = sprite.width;
    const int th = sprite.height;
    transform.Forward(0.0f, 0.0f, sx, sy);
    px = sx; py = sy;
    sx = std::min<float>(sx, px); sy = std::min<float>(sy, py);
    ex = std::max<float>(ex, px); ey = std::max<float>(ey, py);
    transform.Forward((float)tw, (float)th, px, py);
    sx = std::min<float>(sx, px); sy = std::min<float>(sy, py);
    ex = std::max<float>(ex, px); ey = std::max<float>(ey, py);
    transform.Forward(0.0f, (float)th, px, py);
    sx = std::min<float>(sx, px); sy = std::min<float>(sy, py);
    ex = std::max<float>(ex, px); ey = std::max<float>(ey, py);
    transform.Forward((float)tw, 0.0f, px, py);
    sx = std::min<float>(sx, px); sy = std::min<float>(sy, py);
    ex = std::max<float>(ex, px); ey = std::max<float>(ey, py);
    transform.Invert();
    if (ex < sx) std::swap(ex, sx);
    if (ey < sy) std::swap(ey, sy);
    for (float i = sx; i < ex; i++)
    	for (float j = sy; j < ey; j++)
        {
    		float ox, oy;
    		transform.Backward(i, j, ox, oy);
    		this->SetPixel(sprite.GetPixel((int)(ox+0.5f), (int)(oy+0.5f)), (int)i, (int)j, drawMode);
    	}
}

void Window::DrawSprite(int x, int y, Sprite& sprite, float size, DrawMode drawMode)
{
    rect dst;
    dst.sx = (float)x;
    dst.sy = (float)y;
    dst.ex = dst.sx + sprite.width * size;
    dst.ey = dst.sy + sprite.height * size;
    DrawSprite(dst, sprite, drawMode);
}

void Window::DrawSprite(int x, int y, rect src, Sprite& sprite, float size, DrawMode drawMode)
{
    if(src.ex == src.sx || src.ey == src.sy) return;
    if(src.ex < src.sx) std::swap(src.ex, src.sx);
    if(src.ey < src.sy) std::swap(src.ey, src.sy);
    rect dst;
    dst.sx = (float)x;
    dst.sy = (float)y;
    dst.ex = dst.sx + src.ex - src.sx;
    dst.ey = dst.sy + src.ey - src.sy;
    DrawSprite(dst, src, sprite, drawMode);
}

void Window::DrawSprite(rect dst, Sprite& sprite, DrawMode drawMode)
{
    if(dst.ex == dst.sx || dst.ey == dst.sy) return;
    if(dst.ex < dst.sx) std::swap(dst.sx, dst.ex);
    if(dst.ey < dst.sy) std::swap(dst.sy, dst.ey);
    float scale_x = (dst.ex - dst.sx) / sprite.width;
    float scale_y = (dst.ey - dst.sy) / sprite.height;
    for(float x = 0; x < dst.ex - dst.sx; x++)
        for(float y = 0; y < dst.ey - dst.sy; y++)
        {
            int ox = int(x / scale_x + 0.5f);
            int oy = int(y / scale_y + 0.5f);
            this->SetPixel(sprite.GetPixel(ox, oy), (int)(dst.sx + x), (int)(dst.sy + y), drawMode);
        }
}

void Window::DrawSprite(rect dst, rect src, Sprite& sprite, DrawMode drawMode)
{
    if(dst.ex == dst.sx || dst.ey == dst.sy || src.ex == src.sx || src.ey == src.sy) return;
    if(dst.ex < dst.sx) std::swap(dst.sx, dst.ex);
    if(dst.ey < dst.sy) std::swap(dst.sy, dst.ey);
    if(src.ex < src.sx) std::swap(src.sx, src.ex);
    if(src.ey < src.sy) std::swap(src.sy, src.ey);
    float scale_x = (dst.ex - dst.sx) / (src.ex - src.sx);
    float scale_y = (dst.ey - dst.sy) / (src.ey - src.sy);
    for(float x = 0; x < dst.ex - dst.sx; x++)
        for(float y = 0; y < dst.ey - dst.sy; y++)
        {
            int ox = int(x / scale_x + 0.5f);
            int oy = int(y / scale_y + 0.5f);
            this->SetPixel(sprite.GetPixel(ox, oy), (int)(dst.sx + x), (int)(dst.sy + y), drawMode);
        }
}

void Window::DrawCharacter(int x, int y, const char c, float size, uint32_t color, DrawMode drawMode)
{
    rect dst;
    dst.sx = (float)x;
    dst.sy = (float)y;
    dst.ex = dst.sx + char_size(c, size);
    dst.ey = dst.sy + FONT_HEIGHT * size;
    DrawCharacter(dst, c, color, drawMode);
}

void Window::DrawText(int x, int y, const std::string& text, float size, uint32_t color, DrawMode drawMode)
{
    rect dst;
    dst.sx = (float)x;
    dst.sy = (float)y;
    dst.ex = dst.sx + string_size_x(text, size);
    dst.ey = dst.sy + string_size_y(text, size);
    DrawText(dst, text, color, drawMode);
}

void Window::DrawCharacter(rect dst, const char c, uint32_t color, DrawMode drawMode)
{
    if(dst.ex == dst.sx || dst.sy == dst.ey) return;
    if(dst.ex < dst.sx) std::swap(dst.ex, dst.sx);
    if(dst.ey < dst.sy) std::swap(dst.ey, dst.sy);
    float scale_x = (dst.ex - dst.sx) / FONT_WIDTH;
    float scale_y = (dst.ey - dst.sy) / FONT_HEIGHT;
    for(float x = 0; x < dst.ex - dst.sx; x++)
        for(float y = 0; y < dst.ey - dst.sy; y++)
        {
            int ox = int(x / scale_x);
            int oy = int(y / scale_y);
            if(font_data[(int)c - 32][oy] & (1 << ox))
            {
                SetPixel(color, (int)(dst.sx + (FONT_WIDTH * scale_x - x)), (int)(dst.sy + (FONT_HEIGHT * scale_y - y)), drawMode);        
            }
        }
}

void Window::DrawText(rect dst, const std::string& text, uint32_t color, DrawMode drawMode)
{
    if(dst.ex == dst.sx || dst.sy == dst.ey || text.empty()) return;
    if(dst.ex < dst.sx) std::swap(dst.ex, dst.sx);
    if(dst.ey < dst.sy) std::swap(dst.ey, dst.sy);
    float scale_x = (dst.ex - dst.sx) / string_size_x(text, 1);
    float scale_y = (dst.ey - dst.sy) / string_size_y(text, 1);
    float sx = dst.sx, sy = dst.sy;
    for(auto c : text)
    {
        DrawCharacter({sx, sy, sx + scale_x * FONT_WIDTH, sy + scale_y * FONT_HEIGHT}, c, color, drawMode);
        if(c == '\n')
        {
            sy += (FONT_HEIGHT + 1) * scale_y;
            sx = dst.sx;
        }
        else
            sx += char_size(c, scale_x);
    }
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

void Button::render(Window& window, DrawMode drawMode)
{
    int x = (int)(position.x - image.width * 0.5f * size);
    int y = (int)(position.y - image.height * 0.5f * size);
    window.pixelMode = PixelMode::Mask;
    window.DrawSprite(x, y, image, size, drawMode);
    window.pixelMode = PixelMode::Normal;
}

#endif
