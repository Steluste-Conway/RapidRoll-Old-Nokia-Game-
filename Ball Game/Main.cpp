#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <random>
using namespace olc;
using namespace std;

//OBJECTS//
class Object
{
protected:
	PixelGameEngine* engine = nullptr;

public:
	Object() {}
	Object(PixelGameEngine& engine)
	{
		this->engine = &engine;
	}
	
	vf2d pos = vf2d(0, 0);
	vf2d size = vf2d(10, 10);
	Pixel color = WHITE;

	void draw()
	{
		if (engine == nullptr) return;
		engine->FillRect(pos, size, color);
	}
};

//PLAYER//
class Player : public Object
{
public:
	Player(PixelGameEngine& engine) : Object(engine) {}

	vf2d speed = vf2d(0, 0);
	float radius = 0;
	unique_ptr<Sprite> spr = make_unique<Sprite>("Ball.png");
	vf2d sprPos = { 0, 0 };
	

	void drawSprite() {
		engine->DrawSprite(sprPos, spr.get());
	}

	void drawCircle()
	{
		engine->FillCircle(pos, radius, color);
	}
};

//MAIN CLASS//
class Game : public PixelGameEngine
{
private:
	size_t size = 6;
	Player ball = Player(*this);
	Object* block = new Object[size];
	float gravity = 9;
	float gameSpeed = 50;
	int Score = 0;
	int HighScore = 0;
	
	void ResetGame() {
		ball.pos = { 240, 30 };
		ball.radius = 15;

		for (int i = 0; i < size; i++) {
			random_device dev;
			mt19937 rng(dev());
			uniform_int_distribution<mt19937::result_type> dist(0, 420);

			block[i] = Object(*this);
			block[i].pos = { (float)dist(rng), (300 + (float)i * 100) };
			block[i].size = { 60, 10 };
			block[i].color = DARK_BLUE;
		}
	}
	
public:
	Game()
	{
		sAppName = "Ball Game";
	}

//START//
public:
	bool OnUserCreate() override
	{
		string mystring;
		ifstream MyReadFile("highscore.hscr");
		while (getline(MyReadFile, mystring)) {
			cout << mystring;
		}
		MyReadFile.close();

		if (mystring == "") {
			ofstream MyWriteFile("highscore.hscr");
			MyWriteFile << HighScore;
			MyWriteFile.close();
		}
		ResetGame();
		return true;
	}

	vf2d clampVec(vf2d value, vf2d min, vf2d max) {
		return vf2d(std::max(min.x, std::min(max.x, value.x)), std::max(min.y, std::min(max.y, value.y)));
	}

//UPDATE//
	bool death = false;

	bool OnUserUpdate(float deltaTime) override
	{
		Clear(DARK_GREY);
		if (GetKey(ESCAPE).bPressed) return false;

		if (death) {
			if (GetKey(R).bPressed) {
				Score = 0;
				gameSpeed = 50;
				gravity = 9;
				size = 6;
				ResetGame();
				death = false;
			}

			string mystring;
			ifstream MyReadFile("highscore.hscr");
			while (getline(MyReadFile, mystring)) {
				int hs = stoi(mystring);
				HighScore = hs;
			}
			MyReadFile.close();

			if (Score > HighScore) {
				HighScore = Score;
				ofstream MyWriteFile("highscore.hscr");
				MyWriteFile << HighScore;
				MyWriteFile.close();
			}

			DrawString({ ScreenWidth()/2-160, 150}, "Game Over!", RED, 4);
			string scoremsg = "Your Score: " + to_string(Score);
			string hscoremsg = "High Score: " + to_string(HighScore);
			DrawString({ (ScreenWidth() - (int)scoremsg.length() * 16)/2, ScreenHeight() / 2 - 16 }, scoremsg, WHITE, 2);
			DrawString({ (ScreenWidth() - (int)hscoremsg.length() * 16) / 2, ScreenHeight() / 2 - 16 + 50 }, hscoremsg, WHITE, 2);
			DrawString({ ScreenWidth()/2-144, ScreenHeight()-100}, "Press R to restart", WHITE, 2);
		}

		if (death) return true;
		if (ball.pos.y < ball.radius || ball.pos.y > 540 + ball.radius) {
			death = true;
		}

		gravity += .7f;
		ball.speed = { 0, 0 };
		Move();
		ball.pos += vf2d(1 * ball.speed.x * deltaTime, (1 * ball.speed.y + gravity) * deltaTime);

		for (int i = 0; i < size; i++) {
			random_device dev;
			mt19937 rng(dev());
			uniform_int_distribution<mt19937::result_type> dist(0, 420);

			if (block[i].pos.y < 0) {
				block[i].pos = { (float)dist(rng), (570 + (float)i * 100) };
				Score++;
			}

			if (checkCollision(ball.pos, ball.radius, block[i].pos, block[i].size)) {
				gravity = 0;
				ball.pos.y = block[i].pos.y - ball.radius;
			}
			block[i].pos -= { 0, gameSpeed * deltaTime };
			block[i].draw();
		}
		gameSpeed += .002f;
		DrawString({ 10, 10 }, "Score: " + to_string(Score), WHITE, 2);

		return true;
	}
	
	bool OnUserDestroy() override 
	{
		delete[] block;
		return true;
	}

	boolean checkCollision(vf2d circlePos, float radius, vf2d rectPos, vf2d rectSize)
	{
		vf2d testPos = { circlePos.x, circlePos.y };

		if (circlePos.x < rectPos.x) testPos.x = rectPos.x;
		else if (circlePos.x > rectPos.x + rectSize.x) testPos.x = rectPos.x + rectSize.x;
		if (circlePos.y < rectPos.y) testPos.y = rectPos.y;
		else if (circlePos.y > rectPos.y + rectSize.y) testPos.y = rectPos.y + rectSize.y;

		vf2d distPos = { circlePos.x - testPos.x, circlePos.y - testPos.y };
		float distance = sqrt((distPos.x * distPos.x) + (distPos.y * distPos.y));
		
		if (distance <= radius) {
			return true;
		}
		return false;
	}

	void Move() {
		if (GetKey(D).bHeld || GetKey(RIGHT).bHeld) ball.speed.x = 250;
		if (GetKey(A).bHeld || GetKey(LEFT).bHeld) ball.speed.x = -250;
		
		SetPixelMode(Pixel::MASK);
		ball.drawSprite();
		ball.sprPos = ball.pos - vf2d(ball.radius, ball.radius);
		SetPixelMode(Pixel::NORMAL);
	}
};

int main()
{
	Game application;
	if (application.Construct(480, 540, 1, 1))
		application.Start();
	return 0;
}
