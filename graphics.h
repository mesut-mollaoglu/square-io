#ifndef WINDOW_H
#define WINDOW_H

#include "includes.h"

inline void char_size(const char c, float& sx, float& sy, float size_x, float size_y)
{
    switch(c)
    {
        case ' ': 
        {
            sx += 10 * size_x;
            return;
        }
        case '\n':
        {
            sy += 10 * size_y;
            sx = 0;
            return;
        }
        case '\t':
        {
            sx += 18 * size_x;
            return;
        }
        default: 
        {
            sx += 9 * size_x;
            return;
        }
    }
}

inline void string_size(const std::string& text, float size_x, float size_y, float& sx, float& sy)
{
    for(auto& c : text)
    {
        char_size(c, sx, sy, size_x, size_y);
    }
}

enum class DrawMode
{
    Normal,
    Periodic
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
    void SetPixel(uint32_t p, int x, int y);
    uint32_t GetPixel(int x, int y);
};

struct DstRect
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
    int width, height;
    std::string name;
    uint32_t* pixels;
    bool bShouldClose;
    PixelMode pixelMode;
    void Init(std::string name, int width, int height);
    void CreateWindow(std::string name, int width, int height);
    void CreateRenderer();
    void CreateSurface();
    void Clear(uint32_t color);
    void Present();
    void SetPixel(uint32_t p, int x, int y, DrawMode drawMode = DrawMode::Normal);
    uint32_t GetPixel(int x, int y, DrawMode drawMode = DrawMode::Normal);
    void DrawLine(uint32_t p, int x0, int y0, int x1, int y1, DrawMode drawMode = DrawMode::Normal);
    void DrawRect(uint32_t p, int sx, int sy, int ex, int ey, DrawMode drawMode = DrawMode::Normal);
    void DrawCircle(uint32_t p, int cx, int cy, int radius, DrawMode drawMode = DrawMode::Normal);
    void DrawTriangle(uint32_t p, int x1, int y1, int x2, int y2, int x3, int y3, DrawMode drawMode = DrawMode::Normal);
    void DrawSprite(Sprite& sprite, Transform& transform, DrawMode drawMode = DrawMode::Normal);
    void DrawSprite(int x, int y, Sprite& sprite, int size = 1, DrawMode drawMode = DrawMode::Normal);
    void DrawSprite(int x, int y, DstRect dst, Sprite& sprite, int size = 1, DrawMode drawMode = DrawMode::Normal);
    void DrawSprite(DstRect dst, Sprite& sprite, DrawMode drawMode = DrawMode::Normal);
    void DrawSprite(DstRect dst, DstRect src, Sprite& sprite, DrawMode drawMode = DrawMode::Normal);
    void DrawCharacter(int x, int y, const char c, int size = 1, uint32_t color = 0xFF000000, DrawMode drawMode = DrawMode::Normal);
    void DrawCharacter(DstRect dst, const char c, uint32_t color = 0xFF000000, DrawMode drawMode = DrawMode::Normal);
    void DrawText(int x, int y, const std::string& text, int size = 1, uint32_t color = 0xFF000000, DrawMode drawMode = DrawMode::Normal);
    void DrawText(DstRect dst, const std::string& text, uint32_t color = 0xFF000000, DrawMode drawMode = DrawMode::Normal);
    ~Window()
    {
        delete pixels;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_DestroyTexture(surface);
        SDL_Quit();
    }
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

void Sprite::SetPixel(uint32_t p, int x, int y)
{
    if(x >= 0 && x < width && y >= 0 && y < height)
        data[y * width + x] = p;
    else
        return;
}

uint32_t Sprite::GetPixel(int x, int y)
{
    if(x >= 0 && x < width && y >= 0 && y < height)
        return data[y * width + x];
    else
        return 0x00000000;
}

void Window::Init(std::string name, int width, int height)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    CreateWindow(name, width, height);
    CreateRenderer();
    CreateSurface();
}

void Window::CreateWindow(std::string name, int width, int height)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    IMG_Init(IMG_INIT_JPG);
    this->name = name;
    this->width = width;
    this->height = height;
    window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    bShouldClose = false;
    pixelMode = PixelMode::Normal;
}

void Window::CreateRenderer()
{
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    SDL_RenderSetLogicalSize(renderer, width, height);
}

void Window::CreateSurface()
{
    surface = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    pixels = new uint32_t[width * height];
    memset(pixels, 0, 4 * width * height);
}

void Window::Clear(uint32_t color)
{
    for(int i = 0; i < width * height; i++)
        pixels[i] = color;
}

void Window::Present()
{
    int pitch;
    void* buffer;
    SDL_LockTexture(surface, NULL, &buffer, &pitch);
    memcpy(buffer, pixels, sizeof(uint32_t) * width * height);
    SDL_UnlockTexture(surface);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, surface, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void Window::SetPixel(uint32_t p, int x, int y, DrawMode drawMode)
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
    if(pixelMode == PixelMode::Mask)
        if(((p >> 24) & 0xFF) == 0x00)
            return;
    pixels[width * y + x] = p;
}

uint32_t Window::GetPixel(int x, int y, DrawMode drawMode)
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
    }
    return pixels[width * y + x];
}

void Window::DrawLine(uint32_t p, int x0, int y0, int x1, int y1, DrawMode drawMode)
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
        for(int i = 0; i < absdx; i++) {
            x = dx < 0 ? x - 1: x + 1;
            if(d < 0) {
                d = d + 2*absdy;
            } else {
                y = dy < 0 ? y - 1 : y + 1;
                d = d + 2 * (absdy - absdx); 
            }
        SetPixel(p, x, y, drawMode);
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
        SetPixel(p, x, y, drawMode);
        }
    }
}

void Window::DrawRect(uint32_t p, int sx, int sy, int ex, int ey, DrawMode drawMode)
{
    if(sx > ex) std::swap(ex, sx);
    if(sy > ey) std::swap(ey, sy);
    for(int x = sx; x < ex; x++)
        for(int y = sy; y < ey; y++)
            SetPixel(p, x, y, drawMode);
}

void Window::DrawCircle(uint32_t p, int cx, int cy, int radius, DrawMode drawMode)
{
    auto drawLine = [&](int sx, int ex, int y)
    {
        for(int x = sx; x < ex; x++) 
            SetPixel(p, x, y, drawMode);
    };
    const int r2 = radius * radius;
    for(int py = -radius; py < radius; py++)
    {
        int px = (int)std::sqrt((r2 - py * py) + 0.5);
        int y = cy + py;
        drawLine(cx - px, cx + px, y);
    }
}

void Window::DrawTriangle(uint32_t p, int x1, int y1, int x2, int y2, int x3, int y3, DrawMode drawMode)
{
    auto drawLine = [&](int sx, int ex, int y)
    {
        if(sx > ex) std::swap(sx, ex); 
        for(int x = sx; x < ex; x++)
            SetPixel(p, x, y, drawMode);
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

void Window::DrawSprite(int x, int y, Sprite& sprite, int size, DrawMode drawMode)
{
    if(size > 1)
    {
        for(int i = 0; i < sprite.width; i++)
            for(int j = 0; j < sprite.height; j++)
                for(int is = 0; is < size; is++)
                    for(int js = 0; js < size; js++)
                        SetPixel(sprite.GetPixel(i, j), x + i * size + is, y + j * size + js, drawMode);
        return;
    }
    for(int i = 0; i < sprite.width; i++)
        for(int j = 0; j < sprite.height; j++)
            SetPixel(sprite.GetPixel(i, j), x + i, y + j, drawMode);
}

void Window::DrawSprite(int x, int y, DstRect dst, Sprite& sprite, int size, DrawMode drawMode)
{
    if(dst.ex < dst.sx) std::swap(dst.ex, dst.sx);
    if(dst.ey < dst.sy) std::swap(dst.ey, dst.sy);
    if(size > 1)
    {
        for(int i = 0; i < dst.ex - dst.sx; i++)
            for(int j = 0; j < dst.ey - dst.sy; j++)
                for(int is = 0; is < size; is++)
                    for(int js = 0; js < size; js++)
                        SetPixel(sprite.GetPixel(i + dst.sx, j + dst.sy), x + i * size + is, y + j * size + js, drawMode);
        return;
    }
    for(int i = 0; i < dst.ex - dst.sx; i++)
        for(int j = 0; j < dst.ey - dst.sy; j++)
            SetPixel(sprite.GetPixel(i + dst.sx, j + dst.sy), x + i, y + j, drawMode);
}

void Window::DrawSprite(DstRect dst, Sprite& sprite, DrawMode drawMode)
{
    if(dst.ex == dst.sx || dst.ey == dst.sy) return;
    if(dst.ex < dst.sx) std::swap(dst.sx, dst.ex);
    if(dst.ey < dst.sy) std::swap(dst.sy, dst.ey);
    float scale_x = (dst.ex - dst.sx) / sprite.width;
    float scale_y = (dst.ey - dst.sy) / sprite.height;
    for(float x = dst.sx; x < dst.ex; x++)
        for(float y = dst.sy; y < dst.ey; y++)
        {
            int ox = int(x / scale_x + 0.5f);
            int oy = int(y / scale_y + 0.5f);
            this->SetPixel(sprite.GetPixel(ox, oy), (int)x, (int)y, drawMode);
        }
}

void Window::DrawSprite(DstRect dst, DstRect src, Sprite& sprite, DrawMode drawMode)
{
    if(dst.ex == dst.sx || dst.ey == dst.sy || src.ex == src.sx || src.ey == src.sy) return;
    if(dst.ex < dst.sx) std::swap(dst.sx, dst.ex);
    if(dst.ey < dst.sy) std::swap(dst.sy, dst.ey);
    if(src.ex < src.sx) std::swap(src.sx, src.ex);
    if(src.ey < src.sy) std::swap(src.sy, src.ey);
    float scale_x = (dst.ex - dst.sx) / (src.ex - src.sx);
    float scale_y = (dst.ey - dst.sy) / (src.ey - src.sy);
    for(float x = dst.sx; x < dst.ex; x++)
        for(float y = dst.sy; y < dst.ey; y++)
        {
            int ox = int(x / scale_x + 0.5f);
            int oy = int(y / scale_y + 0.5f);
            this->SetPixel(sprite.GetPixel(ox, oy), (int)x, (int)y, drawMode);
        }
}

void Window::DrawCharacter(int x, int y, const char c, int size, uint32_t color, DrawMode drawMode)
{
    if(size > 1)
    {
        for(int i = 0; i < FONT_HEIGHT; i++)
            for(int j = 0; j < FONT_WIDTH; j++)
                if(font_data[(int)c - 32][i] & (1 << j))
                    for(int is = 0; is < size; is++)
                        for(int js = 0; js < size; js++)
                            SetPixel(color, x + (FONT_WIDTH - j) * size + js, y + (FONT_HEIGHT - i) * size + is, drawMode);
        return;
    }
    for(int i = 0; i < FONT_HEIGHT; i++)
        for(int j = 0; j < FONT_WIDTH; j++)
            if(font_data[(int)c - 32][i] & (1 << j))
                SetPixel(color, x + FONT_WIDTH - j, y + FONT_HEIGHT - i, drawMode);
}

void Window::DrawText(int x, int y, const std::string& text, int size, uint32_t color, DrawMode drawMode)
{
    float sx = 0, sy = 0;
    for(auto& c : text)
    {
        DrawCharacter(x + (int)sx, y + (int)sy, c, size, color, drawMode);
        char_size(c, sx, sy, size, size);
    }
}

void Window::DrawCharacter(DstRect dst, const char c, uint32_t color, DrawMode drawMode)
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
            if(font_data[(int)c - 32][oy] & (1 << (ox)))
            {
                SetPixel(color, (int)(dst.sx + (FONT_WIDTH * scale_x - x)), (int)(dst.sy + (FONT_HEIGHT * scale_y - y)), drawMode);        
            }
        }
}

void Window::DrawText(DstRect dst, const std::string& text, uint32_t color, DrawMode drawMode)
{
    if(dst.ex == dst.sx || dst.sy == dst.ey || text.empty()) return;
    if(dst.ex < dst.sx) std::swap(dst.ex, dst.sx);
    if(dst.ey < dst.sy) std::swap(dst.ey, dst.sy);
    float sx = 0, sy = FONT_HEIGHT;
    string_size(text, 1, 1, sx, sy);
    float scale_x = (dst.ex - dst.sx) / sx;
    float scale_y = (dst.ey - dst.sy) / sy;
    sx = dst.sx; sy = dst.sy;
    for(auto c : text)
    {
        DrawCharacter({sx, sy, sx + scale_x * FONT_WIDTH, sy + scale_y * FONT_HEIGHT}, c, color, drawMode);
        char_size(c, sx, sy, scale_x, scale_y);
    }
}

#endif