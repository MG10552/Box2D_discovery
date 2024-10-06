#include <Windows.h>
#include <gl/GL.h>
#include "box2d/Box2D.h"

#include "misc.h"
#include "game.h"


// Box to screen ratio
const float b2s = 10.0f;
// Screen to box ratio
const float s2b = 1.0f / b2s;


struct Filters {
    enum Bits {
        Bit_Boundary = 1,
        Bit_Starship = 2,
        Bit_Shield = 4,
        Bit_Bullet = 8,
        Bit_Asteroid_Core = 16,
        Bit_Asteroid_Shield = 32,
        Bit_Asteroid_Parts = 64,
    };
};


void  Game::createGameBoundary(float dx, float dy) {
    // Tworzymy statyczny obiekt, ograniczaj¹cy pole gry
    //// Struktura b2BodyDef domyœlnie typ ma ustawiony na b2_staticBody, dla jasnoœci przyk³adu ustawiamy j¹ jawnie
    b2BodyDef bd;
    bd.type = b2_staticBody;
    boundary.body = b2world -> CreateBody(&bd);
    b2FixtureDef fd;
    //// Gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
    fd.density = 0.0f; 
    //// Wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
    fd.friction = 5;   
    //// Wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
    fd.restitution = 0; 
    
    // Filtr kolizji
    //// Boundary kolidowaæ bêdzie z ...
    fd.filter.categoryBits = Filters::Bit_Boundary;
    
    //// ... naszym statkiem
    fd.filter.maskBits = Filters::Bit_Starship; 
    b2ChainShape cs;
    b2Vec2 boundary_verts[4] = { s2b * b2Vec2(-dx / 2, dy / 2), s2b * b2Vec2(dx / 2, dy / 2), s2b * b2Vec2(dx / 2, -dy / 2), s2b * b2Vec2(-dx / 2, -dy / 2) };
    cs.CreateLoop(boundary_verts, 4);
    fd.shape = &cs;
    boundary.body -> CreateFixture(&fd);
    boundary.body -> SetUserData(&boundary);
}

void  Game::createStarship() {
    // Dynamiczny obiekt
    b2BodyDef bd;
    bd.type = b2_dynamicBody;
    
    // userData - miejsce na nasze dane w obiekcie b2Body - bêdzie to pointer -> Object starship
    bd.userData = &starship;  
    starship.body = b2world -> CreateBody(&bd);
    b2FixtureDef fd;
    fd.density = 1.0f;
    fd.friction = 1.0f;
    fd.restitution = 0.0f;
    
    // Filtry kolizji
    fd.filter.categoryBits = Filters::Bit_Starship;
    
    // z czym kolidujemy ? 
    fd.filter.maskBits = Filters::Bit_Boundary | Filters::Bit_Asteroid_Core | Filters::Bit_Asteroid_Shield | Filters::Bit_Asteroid_Parts;
    b2PolygonShape ps;
    
    // PolygonShape -> pe³ny wielok¹t, max 8 wierzcho³ków, definiuj¹cych obiekt wypuk³y
    // wierzcho³ki musza byæ podane w kolejnosci CCW (przeciwnej do ruchu wskazówek zegara)
    b2Vec2 ship_verts[] = { s2b * b2Vec2(20.0f, 0), s2b * b2Vec2(-16.0f, 10.0f), s2b * b2Vec2(-16.0f, -10.0f) };
    ps.Set(ship_verts, sizeof(ship_verts) / sizeof(ship_verts[0]));
    fd.shape = &ps;
    starship.body -> CreateFixture(&fd);
    starship.body -> SetUserData(&starship);

    // Ustawimy wygaszanie ruchu liniowego 
    starship.body -> SetLinearDamping(0.25f);
    
    // I obrotowego dla statku
    starship.body -> SetAngularDamping(3.0f);
}

void Game::createBullet(const b2Vec2& vpos, const b2Vec2& vdir) {
    Object *obj = new Object();
    b2BodyDef bd;
    bd.type = b2_dynamicBody;
    bd.position = vpos;
    bd.bullet = true;
    
    // Prêdkoœæ pocisku 300 px/s
    bd.linearVelocity = s2b * 300.0f * vdir;
    obj -> body = b2world -> CreateBody(&bd);
    b2FixtureDef fd;
    fd.density = 1.0f;
    fd.friction = 0.0f;
    fd.restitution = 0.75f;
    
    // groupIndex - elementy bêd¹ce w tej samej grupie mog¹ ze sob¹ 
    // Kolidowaæ -> groupIndex > 0 
    // Nie kolidowaæ -> groupIndex < 0
    fd.filter.groupIndex = -Filters::Bit_Bullet;
    //fd.filter.groupIndex = Filters::Bit_Bullet;  // W tym wypadku bêd¹ kolidowaæ ze sob¹

    // Maski kolizji opisuj¹ kolizje z pozosta³ymi elementami 
    fd.filter.categoryBits = Filters::Bit_Bullet;
    fd.filter.maskBits = Filters::Bit_Asteroid_Core | Filters::Bit_Asteroid_Shield;
    b2CircleShape cs;
    cs.m_radius = s2b * 3.0f;
    fd.shape = &cs;
    obj -> body -> CreateFixture(&fd);
    obj -> body -> SetUserData(obj);

    vbullets.push_back(obj);
}

void  Game::createAsteroid(AsteroidType type) {
    Object *obj = new Object();
    b2BodyDef bd;
    bd.type = b2_dynamicBody;
    bd.position = b2Mul(b2Rot(deg2rad * (rand() % 360)), b2Vec2(s2b * win_dx * 1.25f, 0));
    bd.linearVelocity = starship.body -> GetPosition() - bd.position;
    bd.linearVelocity.Normalize();
    bd.linearVelocity *= s2b * 80;  // 80 m/s 
    obj -> body = b2world -> CreateBody(&bd);
    
    // Tworzenie ko³a asteroidy - oznaczonej jako Bit_Asteroid_Core
    b2FixtureDef fd_core;
    fd_core.density = 1.0f;
    fd_core.friction = 5;
    fd_core.restitution = 0.5f;
    fd_core.filter.groupIndex = Filters::Bit_Asteroid_Core;
    fd_core.filter.categoryBits = Filters::Bit_Asteroid_Core;
    fd_core.filter.maskBits = Filters::Bit_Bullet | Filters::Bit_Starship;
    b2CircleShape cs_core;
    cs_core.m_radius = s2b * 30.0f;
    fd_core.shape = &cs_core;
    obj -> body -> CreateFixture(&fd_core);
    obj -> body -> SetUserData(obj);

    // Dla drugiego typu dodatkowo dochodzi 8 kul na okolo (Bit_Asteroid_Shield)
    if (type == AsteroidType_Complex) {
        for (int q = 0; q < 8; ++q) {
            b2CircleShape cs_shield;
            // Rozstawiamy shieldy co 45 stopni w odleg³oœci 50pix od œrodka
            cs_shield.m_p = b2Mul(b2Rot(deg2rad * q * 360 / 8), b2Vec2(0, s2b * 50.0f));
            cs_shield.m_radius = s2b * 5.0f;
            b2FixtureDef fd_shield;
            fd_shield.density = 1.0f;
            fd_shield.friction = 5;
            fd_shield.restitution = 0.5f;
            fd_shield.filter.groupIndex = -Filters::Bit_Asteroid_Shield;
            fd_shield.filter.categoryBits = Filters::Bit_Asteroid_Shield;
            fd_shield.filter.maskBits = Filters::Bit_Bullet | Filters::Bit_Starship;
            fd_shield.shape = &cs_shield;
            
            // Do³¹czamy wszystkie do cia³a asteroidy
            obj -> body -> CreateFixture(&fd_shield);
        }
    }

    vasteroids.push_back(obj);
};

void  Game::createParticles(const b2Vec2& vpos, int parts_cnt) {
    b2BodyDef bd;
    bd.type = b2_dynamicBody;
    bd.position = vpos;
    b2FixtureDef fd;
    fd.density = 1.0f;
    
    // Nie koliduj¹ ze sob¹
    fd.filter.categoryBits = -Filters::Bit_Asteroid_Parts;
    
    // Nie koliiduj¹ z bulletami
    fd.filter.maskBits &= ~Filters::Bit_Bullet;
    b2CircleShape cs;
    cs.m_radius = s2b * 2.0f;
    fd.shape = &cs;
    
    for (int q = 0; q < parts_cnt; ++q) {
        bd.linearVelocity = b2Mul(b2Rot(deg2rad * q * 360.0f / parts_cnt), b2Vec2(0, s2b * (100.0f + rand() % 40)));
        Object *obj = new Object();
        obj -> body = b2world -> CreateBody(&bd);
        obj -> body -> CreateFixture(&fd);
        vparts.push_back(obj);
    }
}

void  Game::create(int win_size_x, int win_size_y) {
    timer = 0;
    win_dx = (float)win_size_x;
    win_dy = (float)win_size_y;
    scroll_x = 0;
    scroll_y = 0;
    tex_0 = loadTexture("assets/sample_tex.png");

    // Tworzenie œwiata - bez grawitacji
    b2world = new b2World(b2Vec2(0, 0));

    // Tworzenie ograniczenia pola gry
    createGameBoundary(win_dx / 2, win_dy / 2);

    // Tworzymy statek
    createStarship();

    // Przygotowaæ std::vectory
    vbullets.reserve(128);
    vasteroids.reserve(50);
}

void  Game::userInput(const Input& in) {
    // Sterowanie statkiem
    if (in.kb[VK_LEFT] == Input::Hold) {
        // Nadajemy statkowi moment obrotowy w kierunku CCW (counter clockwise)
        starship.body -> ApplyTorque(100, true);
    }

    if (in.kb[VK_RIGHT] == Input::Hold) {
        // Nadajemy statkowi moment obrotowy w kierunku CW (clockwise)
        starship.body -> ApplyTorque(-100, true);
    }

    // Przyœpieszanie w przód
    if (in.kb[VK_UP] == Input::Hold) {
        // GetTransform().q -> obiekt b2Rot, jego pola c i s s¹ odpowiednio cosinusem i sinusem k¹ta obrotu obiektu
        // cosinus i sinus odpowiednio tworz¹ dla nas wersor, który bêdzie dla statku kierunku ruchu dla danego k¹ta -> b2Vec2(cosine, sine)
        b2Vec2 vdir(starship.body -> GetTransform().q.c, starship.body -> GetTransform().q.s);
        
        // Dzia³my si³a w wyznaczonym kierunku
        float forward_force = 40.0f;
        starship.body -> ApplyForceToCenter(forward_force * vdir, true);
    }

    // Przyœpieszanie w ty³
    if (in.kb[VK_DOWN] == Input::Hold) {
        b2Vec2 vdir(starship.body -> GetTransform().q.c, starship.body -> GetTransform().q.s);
        
        // Odwracamy ci¹g, si³a odwrotna ni¿ przy przyœpieszeniu(i mniejsza)
        float backward_force = -20.0f;
        starship.body -> ApplyForceToCenter(backward_force * vdir, true);
    }

    //Strzelanie
    if (in.kb[VK_SPACE] == Input::Down) {
        b2Vec2 ship_pos = starship.body -> GetPosition();
        b2Vec2 vdir(starship.body -> GetTransform().q.c, starship.body -> GetTransform().q.s);
        
        // Ustawienie pocisku 20pixeli od œrodka statku - przed dziobem
        b2Vec2 bullet_pos = ship_pos + s2b * 20.0f * vdir;
        createBullet(bullet_pos, vdir);
    }
}

void  Game::process(const Input& in) {
    stdv_objs vobj2del;
    vobj2del.reserve(32);
    b2world -> Step(1.0f / 60, 5, 5);
    userInput(in);

    // Ekran bêdzie porusza³ siê wraz z ruchem statku
    //scroll_x = b2s * starship.body -> GetPosition().x;
    //scroll_y = b2s * starship.body -> GetPosition().y;

    // Niszczymy pociski jeœli s¹ w odegloœci wiêkszej ni¿ dist_max od statku
    const float dist_max = s2b * 300.0f;
    
    for (size_t q = 0; q < vbullets.size();) {
        b2Vec2 vbull_pos = vbullets[q] -> body -> GetPosition();
        
        if ((vbullets[q] -> body -> GetPosition() - starship.body -> GetPosition()).LengthSquared() < dist_max * dist_max)
            ++q;
        
        else {
            vobj2del.push_back(vbullets[q]);
            std::swap(vbullets[q], vbullets.back());
            vbullets.resize(vbullets.size() - 1);
        }
    }

    // Asteroidy
    //// co 5s tworzymy nowy asteroid, max 5 asteroidow
    if ((++timer % 60 * 5) == 0 && vasteroids.size() < 5) {
        AsteroidType type = (AsteroidType)(rand() % AsteroidTypes_Cnt);
        createAsteroid(type);
    }

    // Jeœli asteroidy s¹ poza polem statku, kierujemy je na statek
    for (size_t a = 0; a < vasteroids.size(); a++) {
        vasteroids[a] -> body -> SetAngularVelocity(1.0f);
        b2Vec2 vpos = vasteroids[a] -> body -> GetPosition();
        
        if (fabs(vpos.x) > s2b * win_dx / 2 || fabs(vpos.y) > s2b * win_dy / 2) {
            b2Vec2 vdir = starship.body->GetPosition() - vpos;
            vdir.Normalize();
            vdir *= 200.0f;
            vasteroids[a] -> body -> SetLinearVelocity(s2b * vdir);
            // ApplyForceToCenter(vdir, true);
        }
    }

    // Sprawdzamy listê kolizji pocisków
    for (size_t q = 0; q < vbullets.size();) {
        bool collided = false;
        
        // Iteracja po wszystkich kontaktach danego pocisku
        for (b2ContactEdge* ce = vbullets[q] -> body -> GetContactList(); ce != NULL; ce = ce -> next) {
            // true -> kontakt nast¹pi³, 
            // false -> jedynie pr¹stok¹ty opisuj¹ce sie pokrywaj¹
            if (ce -> contact -> IsTouching()) {
                b2Filter filterA = ce -> contact -> GetFixtureA() -> GetFilterData();
                b2Filter filterB = ce -> contact -> GetFixtureB() -> GetFilterData();
                
                // Kolizja z asteroid core
                if ((filterA.categoryBits | filterB.categoryBits) & Filters::Bit_Asteroid_Core) {
                    // Asteroida idzie do zniszczenia
                    Object* obj = (Object*)ce -> other -> GetUserData();
                    vobj2del.push_back(obj);
                    collided = true;

                    // W manifoldzie w polu points jest pozycja kontaktu, bêdzie to pozycja emitera cz¹stek
                    b2WorldManifold wm;
                    ce -> contact -> GetWorldManifold(&wm);
                    createParticles(wm.points[0], 32);
                }

                // Kolizja z asteroid shield
                if ((filterA.categoryBits | filterB.categoryBits) & Filters::Bit_Asteroid_Shield) {
                    // Particles
                    // W manifoldzie w polu points jest pozycja kontaktu, bêdzie to pozycja emitera cz¹stek
                    b2WorldManifold wm;
                    ce -> contact -> GetWorldManifold(&wm);
                    createParticles(wm.points[0], 5);
                }
            }
        }

        if (collided) {
            // Bullet idzie do zniszczenia
            vobj2del.push_back(vbullets[q]);
            std::swap(vbullets[q], vbullets.back());
            vbullets.resize(vbullets.size() - 1);
        }
        else
            ++q;
    }

    // Particle - usuwamy jeœli lifietime > 5s
    for (size_t p = 0; p < vparts.size(); ++p) {
        if (++vparts[p] -> lifetime > 5 * 60) {
            // Dodaj do listy do usuniecia
            vobj2del.push_back(vparts[p]);
            vparts[p] = NULL;
        }
    }
    vparts.erase(std::remove(vparts.begin(), vparts.end(), (Object*)NULL), vparts.end());

    // Niszczy obiekty i usuwamy zniszczone z list
    // Do wyjaœnienia na zajêciach
    std::sort(vobj2del.begin(), vobj2del.end());
    Object* last_destroyed = NULL;
    
    for (size_t q = 0; q < vobj2del.size(); ++q) {
        if (last_destroyed != vobj2del[q]) {
            last_destroyed = vobj2del[q];
            auto iter = std::find(vasteroids.begin(), vasteroids.end(), vobj2del[q]);
            
            if (iter != vasteroids.end()) {
                std::swap(*iter, vasteroids.back());
                vasteroids.resize(vasteroids.size() - 1);
            }

            b2world -> DestroyBody(vobj2del[q] -> body);
            delete vobj2del[q] -> mesh;
            delete vobj2del[q];
        }
    }

    render();
}

void  Game::destroy() {
    glDeleteTextures(1, &tex_0.tex_id);
    delete b2world;
    b2world = NULL;
}

void  Game::render() {
    // GL global settings
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.0f);
    // Camera
    glViewport(0, 0, (int)win_dx, (int)win_dy);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0f, 1.0f, -win_dy / win_dx, win_dy / win_dx, 1.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(2.0f / win_dx, 2.0f / win_dx, 1.0f);
    glTranslatef(-scroll_x, -scroll_y, -1.0f);

    // Clear screen
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render
    glPushMatrix();

    debugDraw(b2world, b2s);

    glPopMatrix();
}