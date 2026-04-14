#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <string>
#include <optional>
using namespace std;

const int W = 800, H = 600;

struct Bomber { float x, y, dx; bool alive; int bombTimer; };
struct Bullet  { float x, y, dx, dy; bool alive; };
struct Bomb    { float x, y; bool alive; };
struct Explosion { float x, y, r, alpha; bool alive; };

int score=0, lives=3, ammo=15, level=1;
float bomberSpeed=1.5f;
int spawnInterval=120, spawnTimer=0, frameCount=0;
bool gameOver=false, gameStarted=false;

vector<Bomber> bombers;
vector<Bullet> bullets;
vector<Bomb> bombs;
vector<Explosion> explosions;

float cannonX=W/2, cannonY=H-60;

void spawnBomber(){
    Bomber b;
    bool left=rand()%2;
    b.x=left?-40.f:W+40.f;
    b.y=40.f+rand()%(H/3);
    b.dx=left?bomberSpeed:-bomberSpeed;
    b.alive=true;
    b.bombTimer=60+rand()%80;
    bombers.push_back(b);
}

void fireBullet(float mx, float my){
    if(ammo<=0) return;
    ammo--;
    float dx=mx-cannonX, dy=my-cannonY;
    float len=sqrt(dx*dx+dy*dy);
    if(len==0) return;
    bullets.push_back({cannonX,cannonY-20,(dx/len)*9,(dy/len)*9,true});
}

void addExplosion(float x, float y){
    explosions.push_back({x,y,5.f,1.f,true});
}

void update(){
    if(!gameStarted||gameOver) return;
    frameCount++; spawnTimer++;
    if(spawnTimer>=spawnInterval){ spawnTimer=0; spawnBomber(); }
    if(frameCount%400==0){
        level++; bomberSpeed+=0.3f;
        spawnInterval=max(40,spawnInterval-10);
        ammo=min(ammo+5,25);
    }
    for(auto& b:bombers){
        if(!b.alive) continue;
        b.x+=b.dx; b.bombTimer--;
        if(b.bombTimer<=0){
            b.bombTimer=50+rand()%70;
            bombs.push_back({b.x,b.y+20,true});
        }
        if(b.x<-50||b.x>W+50){
            b.alive=false; lives--;
            if(lives<=0) gameOver=true;
        }
    }
    for(auto& bl:bullets){
        if(!bl.alive) continue;
        bl.x+=bl.dx; bl.y+=bl.dy;
        if(bl.x<0||bl.x>W||bl.y<0||bl.y>H){ bl.alive=false; continue; }
        for(auto& bm:bombers){
            if(!bm.alive) continue;
            if(fabs(bl.x-bm.x)<30&&fabs(bl.y-bm.y)<18){
                bl.alive=false; bm.alive=false;
                score+=10*level;
                addExplosion(bm.x,bm.y);
            }
        }
    }
    for(auto& bm:bombs){
        if(!bm.alive) continue;
        bm.y+=3.f;
        if(bm.y>H){ bm.alive=false; continue; }
        if(fabs(bm.x-cannonX)<22&&fabs(bm.y-cannonY)<22){
            bm.alive=false; addExplosion(cannonX,cannonY);
            lives--; if(lives<=0) gameOver=true;
        }
    }
    for(auto& e:explosions){ e.r+=2.5f; e.alpha-=0.04f; if(e.alpha<=0) e.alive=false; }
    bombers.erase(std::remove_if(bombers.begin(),bombers.end(),[](const Bomber& b){return !b.alive;}),bombers.end());
    bullets.erase(std::remove_if(bullets.begin(),bullets.end(),[](const Bullet& b){return !b.alive;}),bullets.end());
    bombs.erase(std::remove_if(bombs.begin(),bombs.end(),[](const Bomb& b){return !b.alive;}),bombs.end());
    explosions.erase(std::remove_if(explosions.begin(),explosions.end(),[](const Explosion& e){return !e.alive;}),explosions.end());
}

void drawBomber(sf::RenderWindow& win, Bomber& b){
    sf::RectangleShape body(sf::Vector2f(60,18));
    body.setOrigin(sf::Vector2f(30,9));
    body.setPosition(sf::Vector2f(b.x,b.y));
    body.setFillColor(sf::Color(100,160,220));
    win.draw(body);

    sf::CircleShape cockpit(10);
    cockpit.setOrigin(sf::Vector2f(10,10));
    cockpit.setPosition(sf::Vector2f(b.x+(b.dx>0?10:-10),b.y-10));
    cockpit.setFillColor(sf::Color(180,220,255));
    win.draw(cockpit);

    sf::RectangleShape wing(sf::Vector2f(30,8));
    wing.setOrigin(sf::Vector2f(15,4));
    wing.setPosition(sf::Vector2f(b.x,b.y+4));
    wing.setFillColor(sf::Color(70,120,180));
    win.draw(wing);
}

void drawCannon(sf::RenderWindow& win, float mx, float my){
    sf::CircleShape base(22);
    base.setOrigin(sf::Vector2f(22,22));
    base.setPosition(sf::Vector2f(cannonX,cannonY));
    base.setFillColor(sf::Color(80,80,90));
    win.draw(base);

    float angle=atan2(my-cannonY,mx-cannonX);
    sf::RectangleShape barrel(sf::Vector2f(40,8));
    barrel.setOrigin(sf::Vector2f(0,4));
    barrel.setPosition(sf::Vector2f(cannonX,cannonY));
    barrel.setRotation(sf::degrees(angle*180.f/3.14159f));
    barrel.setFillColor(sf::Color(180,180,190));
    win.draw(barrel);
}

int main(){
    srand((unsigned)time(0));
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(W,H)), "Buzz Bomber");
    window.setFramerateLimit(60);

    sf::Font font;
    bool fontLoaded = font.openFromFile("C:/Windows/Fonts/arial.ttf");

    float mouseX=W/2, mouseY=0;

    while(window.isOpen()){
        while(auto event=window.pollEvent()){
            if(event->is<sf::Event::Closed>()) window.close();
            if(auto* mm=event->getIf<sf::Event::MouseMoved>()){
                mouseX=(float)mm->position.x;
                mouseY=(float)mm->position.y;
            }
            if(event->is<sf::Event::MouseButtonPressed>()){
                if(!gameStarted) gameStarted=true;
                else if(!gameOver) fireBullet(mouseX,mouseY);
                else{
                    score=0;lives=3;ammo=15;level=1;
                    bomberSpeed=1.5f;spawnInterval=120;
                    spawnTimer=0;frameCount=0;gameOver=false;
                    bombers.clear();bullets.clear();
                    bombs.clear();explosions.clear();
                }
            }
        }

        update();
        window.clear(sf::Color(10,10,30));

        // Stars
        for(int i=0;i<60;i++){
            sf::CircleShape star(1);
            star.setPosition(sf::Vector2f((float)((i*137)%W),(float)((i*97)%(H*2/3))));
            star.setFillColor(sf::Color(255,255,255,150));
            window.draw(star);
        }

        // Ground
        sf::RectangleShape ground(sf::Vector2f(W,30));
        ground.setPosition(sf::Vector2f(0,H-30));
        ground.setFillColor(sf::Color(30,80,30));
        window.draw(ground);

        for(auto& b:bombers) drawBomber(window,b);

        // Bullets
        for(auto& bl:bullets){
            sf::CircleShape dot(4);
            dot.setOrigin(sf::Vector2f(4,4));
            dot.setPosition(sf::Vector2f(bl.x,bl.y));
            dot.setFillColor(sf::Color(100,220,160));
            window.draw(dot);
        }

        // Bombs
        for(auto& bm:bombs){
            sf::CircleShape dot(6);
            dot.setOrigin(sf::Vector2f(6,6));
            dot.setPosition(sf::Vector2f(bm.x,bm.y));
            dot.setFillColor(sf::Color(239,159,39));
            window.draw(dot);
        }

        // Explosions
        for(auto& e:explosions){
            sf::CircleShape ring(e.r);
            ring.setOrigin(sf::Vector2f(e.r,e.r));
            ring.setPosition(sf::Vector2f(e.x,e.y));
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineThickness(2);
            ring.setOutlineColor(sf::Color(239,159,39,(uint8_t)(e.alpha*255)));
            window.draw(ring);
        }

        drawCannon(window,mouseX,mouseY);

        // HUD Text
        if(fontLoaded){
            sf::Text scoreT(font,"Score: "+to_string(score),20);
            scoreT.setPosition(sf::Vector2f(10,10));
            scoreT.setFillColor(sf::Color::White);
            window.draw(scoreT);

            sf::Text livesT(font,"Lives: "+to_string(lives),20);
            livesT.setPosition(sf::Vector2f(200,10));
            livesT.setFillColor(sf::Color(255,100,100));
            window.draw(livesT);

            sf::Text ammoT(font,"Ammo: "+to_string(ammo),20);
            ammoT.setPosition(sf::Vector2f(380,10));
            ammoT.setFillColor(sf::Color(100,220,160));
            window.draw(ammoT);

            sf::Text levelT(font,"Level: "+to_string(level),20);
            levelT.setPosition(sf::Vector2f(560,10));
            levelT.setFillColor(sf::Color(255,200,80));
            window.draw(levelT);

            if(!gameStarted){
                sf::Text startT(font,"Click to Start!",30);
                startT.setPosition(sf::Vector2f(W/2-100,H/2));
                startT.setFillColor(sf::Color::Yellow);
                window.draw(startT);
            }
            if(gameOver){
                sf::Text goT(font,"GAME OVER!",40);
                goT.setPosition(sf::Vector2f(W/2-100,H/2-30));
                goT.setFillColor(sf::Color(255,80,80));
                window.draw(goT);

                sf::Text scT(font,"Score: "+to_string(score),25);
                scT.setPosition(sf::Vector2f(W/2-60,H/2+20));
                scT.setFillColor(sf::Color::White);
                window.draw(scT);

                sf::Text restT(font,"Click to Restart",20);
                restT.setPosition(sf::Vector2f(W/2-80,H/2+60));
                restT.setFillColor(sf::Color::Yellow);
                window.draw(restT);
            }
        }

        window.display();
    }
    return 0;
}