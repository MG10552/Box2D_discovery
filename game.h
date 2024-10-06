#pragma once

class  b2World;
class  b2Body;
struct b2Vec2;



class Game
{
  int       timer;
  float     win_dx;
  float     win_dy;
  float     scroll_x, scroll_y;

  //swiat
  b2World*     b2world;
  
  struct Object
  {
    b2Body*   body;
    Mesh*     mesh;
    int       lifetime;
    Object() : body(NULL), mesh(NULL), lifetime(0)
    {

    }
  };
  typedef std::vector<Object*>  stdv_objs;

  //nasz statek
  Object      starship;
  //pole gry
  Object      boundary;
  
  enum AsteroidType
  {
    AsteroidType_Basic,
    AsteroidType_Complex,
    AsteroidTypes_Cnt,
  };
  
  //lista asteroidów
  stdv_objs   vasteroids;
  //list pocisków
  stdv_objs   vbullets;
  //lista partikli
  stdv_objs   vparts;
  //textura
  Texture     tex_0;


  //dx,dy - szerokoœæ i wysokoœæ ograniczenia
  void      createGameBoundary(float dx, float dy);
  //
  void      createStarship();
  //pozycja pocz¹tkowa pocisku, kierunek strza³u
  void      createBullet(const b2Vec2& bullet_pos, const b2Vec2& bullet_dir);
  void      createAsteroid(AsteroidType type);
  void      createParticles(const b2Vec2& vpos, int parts_cnt);
  void      userInput(const Input& in);
  void      render();

public:
  Game() : win_dx(0), win_dy(0){}
  void      create(int win_size_x, int win_size_y);
  void      destroy();
  void      process(const Input& in);
};