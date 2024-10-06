#pragma once
#include <vector>

typedef unsigned int u32;
typedef unsigned char u8;


inline u32 rgba2i(u8 r, u8 g, u8 b, u8 a) {
    return u32(r | (g << 8) | (b << 16) | (a << 24));
}

inline u8 getB(u32 rgba) {
    return u8((rgba >> 16) & 0x000000ff);
}

inline u8 getA(u32 rgba) {
    return u8((rgba >> 24) & 0x000000ff);
}

inline u8 getR(u32 rgba) {
    return u8(rgba & 0x000000ff);
}

inline u8 getG(u32 rgba) {
    return u8((rgba >> 8) & 0x000000ff);
}

inline float getBf(u32 rgba) {
    return getB(rgba) / 255.0f;
}

inline float getGf(u32 rgba) {
    return getG(rgba) / 255.0f;
}

inline float getRf(u32 rgba) {
    return getR(rgba) / 255.0f;
}

inline float getAf(u32 rgba) {
    return getA(rgba) / 255.0f;
}

const float rad2deg = 180.0f / 3.141592f;
const float deg2rad = 3.141592f / 180.0f;

struct  Input {
    enum {
        Idle,
        Down,
        Hold,
        Up,
    };

    int ms[2];
    int mx;
    int my;
    int kb[255];

    Input() {
        ms[0] = ms[1] = 0;
        mx = my = 0;
        memset(kb, 0, sizeof(kb));
    };
};

class ContextGL {
    HWND hwin;
    HDC hdc;
    HGLRC hgl;
    
    public:
        ContextGL(): hwin(NULL), hdc(NULL), hgl(NULL) { }
        void create(HWND hwin);
        void destroy();
        void swap();
};

struct Texture {
    u32 tex_id;
    int size_x;
    int size_y;

    Texture() {
        tex_id = 0;
        size_x = 0;
        size_y = 0;
    }
};

Texture loadTexture(const char *filename);

struct  VtxPT {
    float x, y;
    float u, v;

    VtxPT(float px, float py, float tu = 0.0f, float tv = 0.0f) {
        x = px;
        y = py;
        u = tu;
        v = tv;
    }
};

class Mesh {
    std::vector<VtxPT> vvtxs;
    int  tex_id;
    
    public:
        float x, y;
        float roto;
        float sx, sy;
        u32 color;
        float size_dx;
        float size_dy;

        Mesh();
        void createRect(int size_x, int size_y, u32 rgba = 0xffffffff, const Texture* tex = NULL, int tex_x0 = 0, int tex_y0 = 0);
        void createWheel(int rad, u32 rgba = 0xffffffff, int tris_cnt = 10, const Texture* tex = NULL, int tex_x0 = 0, int tex_y0 = 0);
        void render();
};

class b2World;

void debugDraw(const b2World* world, float scale);
