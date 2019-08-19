#include <SFML/Graphics.hpp>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include<functional>
#include "arial.h"
using namespace std;

void HideConsole()
{
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}

// I declare the window size here so i can refer to it in classes as the window is created in main, and other funcions can't assume there is a window, also cant create the window here,.. i tried that.
bool debounceEscape = false;
bool debounceClick = false;
const int x = 30;
const int y = 20;
const int TileSize = 32;
int windowWidth = x*TileSize;
int windowHeight = y*TileSize;
float gravity = 0.5f;
float friction = 1.5f;
sf::Vector2f worldPos;
bool playerExists = false;
// an enum for menu nav i just switch through this every frame, each state has a way of getting to other states.
enum ProgramState{mainWindow,testMovement};
ProgramState programState = testMovement;


class Actor : public sf::Drawable, public sf::Transformable
{
private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		target.draw(sprite, states);
	}
public:
	sf::Vector2f velocity = sf::Vector2f(0,0);
	bool grounded = false;
	float speed = 2.5f;
	float jumpSpeed = 0.5f;
	sf::Sprite sprite;
	enum Type { None, Enemy, Coin, Player, Trap, Exit };
	Type type = None;
	sf::Texture doorTexture;
	sf::Texture coinTexture;
	sf::Texture enemyAliveTexture;
	sf::Texture enemyDeadTexture;
	sf::Texture playerTexture;
	sf::Texture blankTexture;
	Actor()
	{
	}
	void loadTextures()
	{
		if (!blankTexture.loadFromFile("PlatformerSprites/None.png"))
		{
			cout << "failed to load None.png";
		}
		if (!coinTexture.loadFromFile("PlatformerSprites/Coin.png"))
		{
			cout << "failed to load Coin.png";
		}
		if (!doorTexture.loadFromFile("PlatformerSprites/Door.png"))
		{
			cout << "failed to load Door.png";
		}
		if (!enemyAliveTexture.loadFromFile("PlatformerSprites/EnemyAlive.png"))
		{
			cout << "failed to load EnemyAlive.png";
		}
		if (!enemyDeadTexture.loadFromFile("PlatformerSprites/EnemyDead.png"))
		{
			cout << "failed to load EnemyDead.png";
		}
		if (!playerTexture.loadFromFile("PlatformerSprites/Player.png"))
		{
			cout << "failed to load Player.png";
		}
	}

	void init(int x, int y)
	{
		loadTextures();
		sprite.setPosition(x, y);
		RefreshActor();
	}

	void RefreshActor()
	{
		switch (type)
		{
		case Player:
			sprite.setTexture(playerTexture);
			break;
		case Enemy:
			sprite.setTexture(enemyAliveTexture);
			break;
		case Coin:
			sprite.setTexture(coinTexture);
			break;
		case Exit:
			sprite.setTexture(doorTexture);
			break;
		case None:
			sprite.setTexture(blankTexture);
			break;
		default:
			sprite.setTexture(blankTexture);
			break;
		}
	}

};
class Tile : public sf::Drawable, public sf::Transformable
{
private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		target.draw(sprite, states);

	}
public:
	enum Type { Sky, Platform, Lava };
	Type type = Tile::Sky;
	//sf::RectangleShape shape;
	sf::Sprite sprite;
	sf::FloatRect rBounds;

	sf::Texture blockLavaTexture;
	sf::Texture blockPlatformTexture;
	sf::Texture blockSkyTexture;

	void init(int x, int y)
	{
		loadTextures();
		sprite.setPosition(x, y);

		refreshTile();
	}

	void loadTextures()
	{
		if (!blockLavaTexture.loadFromFile("PlatformerSprites/BlockLava.png"))
		{
			cout << "failed to load BlockLava.png";
		}
		if (!blockPlatformTexture.loadFromFile("PlatformerSprites/BlockPlatform.png"))
		{
			cout << "failed to load BlockPlatform.png";
		}
		if (!blockSkyTexture.loadFromFile("PlatformerSprites/BlockSky.png"))
		{
			cout << "failed to load BlockSky.png";
		}
	}

	bool mouseOver(sf::Vector2f windPos)
	{
		rBounds = sprite.getGlobalBounds();
		if (rBounds.contains(windPos.x, windPos.y))
		{
			return true;
		}
		else
		{
			return false;
		}

	}
	void refreshTile()
	{
		switch (type)
		{
		case Type::Sky:
			sprite.setTexture(blockSkyTexture);
			break;
		case Type::Platform:
			sprite.setTexture(blockPlatformTexture);
			break;
		case Type::Lava:
			sprite.setTexture(blockLavaTexture);
			break;
		}
	}
	void ChangeType(Type t)
	{
		type = t;
	}

};

int sign(int x) {
	return (x > 0) - (x < 0);
}
int sign(float x) {
	return (x > 0.0f) - (x < 0.0f);
}

class Button : public sf::Drawable, public sf::Transformable
{
private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{

		target.draw(rect, states);
		target.draw(text, states);
	}

public:

	int textSize = 16;
	float rectOutline = 1;
	sf::RectangleShape rect;
	sf::Font font;
	sf::Text text;
	sf::FloatRect rBounds;
	//Title init
	Button()
	{
		font.loadFromMemory(&arial_ttf, arial_ttf_size);

		text.setFont(font);
		text.setCharacterSize(textSize);
		text.setFillColor(sf::Color::Black);
		text.setStyle(sf::Text::Regular);
		rect.setFillColor(sf::Color::White);
		rect.setOutlineColor(sf::Color::Black);
		rect.setOutlineThickness(rectOutline);

	}
	bool mouseOver(sf::Vector2f windPos)
	{
		rBounds = rect.getGlobalBounds();

		if (rBounds.contains(windPos.x, windPos.y))
		{
			rect.setOutlineThickness(2);
			return true;
		}
		else
		{
			rect.setOutlineThickness(rectOutline);
			return false;
		}

	}
	// this checks to see if the button has been clicked and if so, runs a function passed asn anrgument with its own arguments already bound to it.
	void checkClick(std::function<void()> f, sf::Vector2f windPos)
	{
		if (mouseOver(windPos) && sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			f();
		}
	}
};

class UI
{
public:
	//Text Variable delcarations
	float titleSize = 60;
	float titleOutline = 6;
	float uiTextSize = 30;
	float uiTextOutline = 3;
	
	//Font init
	sf::Font font;
	//Text declarations
	sf::Text title;
	sf::Text gameOver;
	//Button declarations
	Button mainMenuButton;
	Button exitButton;
	Button resumeButton;
	//panel
	sf::RectangleShape panel;
	sf::Vector2f panelSize = sf::Vector2f(350, 550);
	UI()
	{
		//Loading the font, I check to see if there is a font in the run dir, if not load the one i've embedded using a BIN2C converter. 
		if (!font.loadFromFile("arial.ttf"))
		{
			font.loadFromMemory(&arial_ttf, arial_ttf_size);
		}

		//Text initialisations -----------------------------------

		//Title init
		title.setString("LevelEditor");
		title.setFont(font);
		title.setPosition((windowWidth / 2) - ((title.getGlobalBounds().width / 2) + 75), 200);
		title.setCharacterSize(titleSize);
		title.setFillColor(sf::Color::White);
		title.setOutlineColor(sf::Color::Black);
		title.setOutlineThickness(titleOutline);
		title.setStyle(sf::Text::Regular);

		//panel Init
		panel.setSize(panelSize);
		panel.setFillColor(sf::Color(255,255,255,150));
		panel.setOutlineColor(sf::Color::Black);
		panel.setOutlineThickness(1.0f);
		panel.setPosition((windowWidth / 2) - (panelSize.x / 2), 180);

		//Buttons Initialisations -------------------------------

		//Main Menu Button Init
		mainMenuButton.text.setString("Main Menu");
		mainMenuButton.rect.setSize(sf::Vector2f(150, 35));
		mainMenuButton.rect.setPosition(((windowWidth / 2) - mainMenuButton.rect.getGlobalBounds().width / 2) , 600);
		mainMenuButton.text.setPosition(((windowWidth / 2) - mainMenuButton.text.getGlobalBounds().width / 2) , 600);
		//Exit Button Init
		exitButton.text.setString("Exit");
		exitButton.rect.setSize(sf::Vector2f(60, 35));
		exitButton.rect.setPosition(((windowWidth / 2) - exitButton.rect.getGlobalBounds().width / 2), 550);
		exitButton.text.setPosition(((windowWidth / 2) - exitButton.text.getGlobalBounds().width / 2), 550);
		//Resume Button Init
		resumeButton.text.setString("Resume");
		resumeButton.rect.setSize(sf::Vector2f(100, 35));
		resumeButton.rect.setPosition(((windowWidth / 2) - resumeButton.rect.getGlobalBounds().width / 2), 400);
		resumeButton.text.setPosition(((windowWidth / 2) - resumeButton.text.getGlobalBounds().width / 2), 400);
}
};
void load(Tile tile[x][y])
{
	// reading a text file
	string line;
	ifstream myfile("save.sav");
	if (myfile.is_open())
	{
		int a = 0;
		int b = 0;
		string saveHold;
		while (getline(myfile, line))
		{
			if (b == 0)
			{
				cout << "Loading Tiles \n";
			}
			if (b < x) {
				for (int i = 0; i < line.size(); i++) {
					switch (line[i])
					{

					case ',':
						a += 1;
						break;
					case '0':
						tile[b][a].ChangeType(tile[a][b].Type::Sky);
						break;
					case '1':
						tile[b][a].ChangeType(tile[a][b].Type::Platform);
						break;
					case '2':
						tile[b][a].ChangeType(tile[a][b].Type::Lava);
						break;

					}
					tile[b][a].refreshTile();
					//tile[b][a].ChangeActor(Actor::Type::None);
				}
			}
			else if (false)//(b >= x)
			{
				// just checking the first two letters on lines after the tiles are loaded.
				//this is where i will load all the 'actors'

				string lineHold = line;
				int curStart;
				int curEnd = 0;
				string posString;
				if (line[0] == 'C' && line[1] == 'o')
				{
					cout << "Loading Coins \n";


					while (curEnd < lineHold.length() && lineHold.find('(') != string::npos)
					{
						curStart = lineHold.find('(');
						curEnd = lineHold.find(')');

						posString = lineHold.substr(curStart + 1, curEnd - (curStart + 1));
						cout << " Loaded Coin at :" << posString << "\n";
						string xStr = posString.substr(0, posString.find(','));
						string yStr = posString.substr(posString.find(',') + 1, posString.length() - 1);
						cout << "x = " << xStr << ", y = " << yStr << "\n";
						Actor coin;
						coin.type = Actor::Type::Coin;
						coin.init(stoi(xStr), stoi(yStr));
						lineHold[curStart] = '<';
						lineHold[curEnd] = '>';
					}
				}
				else if (line[0] == 'E' && line[1] == 'n')
				{
					cout << "Loading Enemies \n";
					while (curEnd < lineHold.length() && lineHold.find('(') != string::npos)
					{
						curStart = lineHold.find('(');
						curEnd = lineHold.find(')');

						posString = lineHold.substr(curStart + 1, curEnd - (curStart + 1));
						cout << " Loaded Enemy at :" << posString << "\n";
						string xStr = posString.substr(0, posString.find(','));
						string yStr = posString.substr(posString.find(',') + 1, posString.length() - 1);
						cout << "x = " << xStr << ", y = " << yStr << "\n";
						//tile[stoi(xStr)][stoi(yStr)].actor.ChangeActor(Actor::Type::Enemy);

						lineHold[curStart] = '<';
						lineHold[curEnd] = '>';
					}
				}
				else if (line[0] == 'T' && line[1] == 'r')
				{
					cout << "Loading traps \n";
					while (curEnd < lineHold.length() && lineHold.find('(') != string::npos)
					{
						curStart = lineHold.find('(');
						curEnd = lineHold.find(')');

						posString = lineHold.substr(curStart + 1, curEnd - (curStart + 1));
						cout << " Loaded Traps at :" << posString << "\n";
						string xStr = posString.substr(0, posString.find(','));
						string yStr = posString.substr(posString.find(',') + 1, posString.length() - 1);
						cout << "x = " << xStr << ", y = " << yStr << "\n";
						//tile[stoi(xStr)][stoi(yStr)].actor.ChangeActor(Actor::Type::Trap);

						lineHold[curStart] = '<';
						lineHold[curEnd] = '>';
					}
				}
				else if (line[0] == 'E' && line[1] == 'x')
				{
					cout << "Loading Exit \n";
					curStart = lineHold.find('(');
					curEnd = lineHold.find(')');
					posString = lineHold.substr(curStart + 1, curEnd - (curStart + 1));
					string xStr = posString.substr(0, posString.find(','));
					string yStr = posString.substr(posString.find(',') + 1, posString.length() - 1);
					posString = lineHold.substr(curStart + 1, curEnd - (curStart + 1));
					//tile[stoi(xStr)][stoi(yStr)].actor.ChangeActor(Actor::Type::Exit);
				}
				else if (line[0] == 'P' && line[1] == 'l')
				{
					cout << "Loading player \n";
					curStart = lineHold.find('(');
					curEnd = lineHold.find(')');
					posString = lineHold.substr(curStart + 1, curEnd - (curStart + 1));
					string xStr = posString.substr(0, posString.find(','));
					string yStr = posString.substr(posString.find(',') + 1, posString.length() - 1);
					posString = lineHold.substr(curStart + 1, curEnd - (curStart + 1));
					//tile[stoi(xStr)][stoi(yStr)].actor.ChangeActor(Actor::Type::Player);
				}

			}

			b += 1;
			a = 0;
		}
		myfile.close();
		cout << "file Loaded \n";
	}
	else cout << "Unable to open file \n";

}
/*void load(Tile tile[x][y])
{
	// reading a text file
	string line;
	ifstream myfile("save.Txt");
	if (myfile.is_open())
	{
		int a = 0;
		int b = 0;
		string saveHold;
		while (getline(myfile, line))
		{
			for (int i = 0; i < line.size(); i++) {
				switch (line[i]) 
				{

				case ',':
					 b += 1;
					break;
				case '0':
					tile[b][a].ChangeType(tile[b][a].Type::Sky);
					break;
				case '1':
					tile[b][a].ChangeType(tile[b][a].Type::Platform);
					break;
				case '2':
					tile[b][a].ChangeType(tile[b][a].Type::Lava);
					break;

				}
			}
			a += 1;
			b = 0;
		}
		myfile.close();
		cout << "file Loaded";
	}
	else cout << "Unable to open file";
}*/

int main()
{
	//window init -- This stuff just sets up the rendering window, and hides the console.
	//HideConsole();
	sf::RenderWindow window(sf::VideoMode(windowWidth,windowHeight), "Why no work?.", sf::Style::Titlebar | sf::Style::Close);
	sf::Clock clock;
	float deltaTime;
	sf::Time tDiff;
	Tile tile[x][y];
	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < y; j++)
		{

			tile[i][j].init(i* TileSize + ((windowWidth / 2) - ((TileSize*x) / 2)), j*TileSize);
		}
	}
	Actor Enemies;
	Actor Dude;
	Dude.init(TileSize,windowHeight - TileSize*3);
	Dude.sprite.setTexture(Dude.playerTexture);
	load(tile);

	// This is the main loop,.. Or Game loop.. 
	while (window.isOpen())
	{ 
		deltaTime = clock.restart().asSeconds();
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type) 
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyReleased:
				switch (event.key.code)
				{
				case sf::Keyboard::V:
					cout << Dude.velocity.x << ',' << Dude.velocity.y << ".";
					break;
				}
				break;
			}
		}

		window.clear(sf::Color::White);
		switch (programState) 
		{
			case mainWindow:
				for (int i = 0; i < x; i++)
				{
					for (int j = 0; j < y; j++)
					{

						//if (tile[i][j]){
						//}
						window.draw(tile[i][j]);
					}
				}
				break;

			case testMovement:
				//tDiff = clock.getElapsedTime() - elapsed1;
				//Dude.sprite.setPosition(Dude.sprite.getPosition().x + Dude.Velocity.x, Dude.sprite.getPosition().y + Dude.Velocity.y);

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
				{

					if (Dude.grounded) 
					{
						Dude.grounded = false;
						Dude.velocity.y += -Dude.jumpSpeed;
					}
				}


				if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
				{
					if (Dude.grounded)
					{
						Dude.velocity.x += Dude.speed * deltaTime;
					}
					else 
					{
						Dude.velocity.x += Dude.speed/2 * deltaTime;
					}
				}

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
				{
					if (Dude.grounded)
					{
						Dude.velocity.x += -Dude.speed * deltaTime;
					}
					else 
					{
						Dude.velocity.x += -Dude.speed/2 * deltaTime;
					}
				}

				for (int i = 0; i < x; i++)
				{
					for (int j = 0; j < y; j++)
					{
						//if (tile[i][j]){
						//}
						window.draw(tile[i][j]);
					}
				}
				if (Dude.grounded) {

					if (abs(Dude.velocity.x) > 0)
					{
						Dude.velocity.x -= friction * deltaTime * (sign(Dude.velocity.x));
					}
					else
					{
						Dude.velocity.x = 0.0f;
					}
				}
				if (abs(Dude.velocity.x) > 0.6f)
				{
					Dude.velocity.x = 0.6f * sign(Dude.velocity.x);
				}

				//adding gravity
				if (Dude.velocity.y < 1.0f)
				{
					Dude.velocity.y += gravity * deltaTime; //(sign(Dude.velocity.x));
				}


				//adding a maximum jump velocity
				else if (Dude.velocity.y < -1.0f) 
				{
					Dude.velocity.y == -1.0f;
				}


				Dude.sprite.setPosition(Dude.sprite.getPosition() + Dude.velocity);



				// boundaries 
				//y
				if(Dude.sprite.getPosition().y  > windowHeight - TileSize)
				{
					Dude.sprite.setPosition(Dude.sprite.getPosition().x, windowHeight - TileSize);
					Dude.grounded = true;
				}
				else if (Dude.sprite.getPosition().y < 0.1f)
				{
					Dude.sprite.setPosition(Dude.sprite.getPosition().x, 0.1f);
					Dude.velocity.y = 0.0f;
				}
				//x
				if(Dude.sprite.getPosition().x <0.1f)
				{
					Dude.sprite.setPosition(0.1f,Dude.sprite.getPosition().y);
					Dude.velocity.x = 0.0f;
				}
				else if (Dude.sprite.getPosition().x > windowWidth - TileSize) 
				{
					Dude.sprite.setPosition(windowWidth - TileSize, Dude.sprite.getPosition().y);
					Dude.velocity.x = 0.0f;
				}
				
				window.draw(Dude.sprite);
				break;

			default:
				cout << "No Mode Selected";
				break;
		}	
		window.display();
		
	}
}
