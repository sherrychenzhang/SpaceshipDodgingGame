#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <ncurses.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <cstring>

using namespace std;

WINDOW* window;
int height;
int width;

class Spaceship 
{
private:
	int pos;
	char symbol;

public:
	Spaceship(char symbol)
	{
		this->symbol = symbol;

		this->pos = width / 2;

		display();
	};

	void move(int delta)
	{
		// do not move the ship if current pos is most left or most right
		if (this->pos + delta < 0 || this->pos +delta >= width)
			return;

		// clean current pos of spaceship
		mvwaddch(window, height - 1, this->pos, ' ');

		this->pos += delta;

		display();
	};

	int getPos()
	{
		return this->pos;
	};
	
	char getSymbol()
	{
		return this->symbol;
	};	

	void display()
	{
		mvwaddch(window, height - 1, this->pos, symbol);

		refresh();
	};
};

class Obstacles
{
private:
	char symbol;
	vector<vector<char> > lines;
	int * flag;

	vector<char> createNewLine()
	{
		vector<char> line;
		for(int i = 0; i < width; i ++)
			line.push_back(rand() % 100 > 87 ? this->symbol : ' '); 
		return line;
	};

	void display()
	{
		int lineNumber = 0;
		for(int i = this->lines.size() - 1; i >= 0 ; i --, lineNumber ++)
		{
			for(int j = 0; j < width; j ++)
				mvwaddch(window, lineNumber, j, this->lines[i][j]);
		}
		refresh();
	};	

	void drop()
	{
		if (this->lines.size() >= (size_t)height)
		{
			this->lines[0].clear();
			this->lines.erase(this->lines.begin());	
			

			memset(this->flag, 0, width);
			for(size_t i = 0; i < this->lines.front().size(); i ++)
				this->flag[i] = this->lines.front()[i] == this->symbol;
		}

		this->lines.push_back(createNewLine());

		display();
	};

	bool detectCollision(Spaceship spaceship)
	{
		if (this->flag[spaceship.getPos()])
		{
			for (int i = 0; i < 5; i ++)
			{
				mvwaddch(window, height - 1, spaceship.getPos(), spaceship.getSymbol() | A_BLINK);
				refresh();
				usleep(800000 * 0.8);
			}
			return true;
		}

		return false;
	};

public:
	Obstacles(char symbol)
	{
		this->symbol = symbol;

		this->flag = new int[width];
		memset(this->flag, 0, width * sizeof(int));
	};

	bool detectCollisionAndDrop(Spaceship spaceship)
	{
		if (detectCollision(spaceship))
			return true;
		else
		{
			drop();
			return detectCollision(spaceship);
		}
	}
};

void initialize(int &height, int &width)
{
	cbreak();
    noecho();
    clear();
    refresh();
    keypad(window, true);
    nodelay(window, true);
    curs_set(0);

	getmaxyx(window, height, width);
}

void cleanUp()
{
	endwin();
}

void printLine(string str, int offsetX, int offsetY)
{
	for (size_t i = 0; i < str.size(); i ++)
		mvwaddch(window, offsetX, offsetY + i, str[i]);
}

void displayScore(int score)
{
	string scoreStr = "Score: " + to_string(score);
	for (size_t i = 0; i < scoreStr.size(); i ++)
		mvwaddch(window, 0, i, scoreStr[i]);
}

void displayFinalScore(int score)
{
	clear();
	
	int countDown = 5;

	string scoreStr = "Your final score: " + to_string(score);
	string message1 = "Good luck next time!";
	string message2 = "Press \'q\' to exit or this window will automatically close in " + to_string(countDown) + " seconds.";

	printLine(scoreStr, 0, 0);
	printLine(message1, 1, 0);
	printLine(message2, 2, 0);

	bool quit = false;

	while(true) {
       	switch(wgetch(window)) 
		{
       		case 'q':
			   quit = true;
			   break;   
            default:
            	break;         
   		} 
		
		if (quit || countDown == 0)
			return;

		usleep(1000000);	

		countDown --;

		string msg = "Press \'q\' to exit or this window will automatically close in " + to_string(countDown) + (countDown == 1 ? " second." : " seconds.");
		
		clear();
		printLine(scoreStr, 0, 0);
		printLine(message1, 1, 0);
		printLine(msg, 2, 0);

		refresh();
	}
}

void startGame(char symbol_obstacle, char symbol_spaceship)
{
	Obstacles obstacles(symbol_obstacle);
	Spaceship spaceship(symbol_spaceship);

	int score = 0;

	bool quit = false;
	
    while(true) {
       	switch(wgetch(window)) 
		{
       		case 'q':
			   quit = true;
			   break;   
			case KEY_LEFT:
            	spaceship.move(-1);
            	break;
            case KEY_RIGHT:
            	spaceship.move(1);
            	break;
            default:
            	break;         
   		}

		if (quit || obstacles.detectCollisionAndDrop(spaceship))
			break;

		spaceship.display();

		score ++;

		displayScore(score);

		refresh();
		
		usleep(100000);	
   	}

	displayFinalScore(score);   
}

bool parseParameters(int argc, char *argv[], char &symbol_obstacle, char &symbol_spaceship)
{
	if (argc % 2 == 0)
		return false;

	for (int i = 1; i < argc; i = i + 2)
	{
		string arg = string(argv[i]);
		if (arg == "-a")
			symbol_obstacle = argv[i + 1][0];
		else if (arg == "-b")
			symbol_spaceship = argv[i + 1][0];
		else
			return false;
	}		

	return true;
}

int main(int argc, char *argv[]) 
{
	char symbol_obstacle = '-';
	char symbol_spaceship = '*';
	
	if (!parseParameters(argc, argv, symbol_obstacle, symbol_spaceship))
	{
		cout << "Parameters are incorrect! Please check again" << endl;
		cout << "Optional parameters:" << endl;
		cout << "\t-a char: obstacle symbol. Only the first character will be assigned as symbol, if more than one character is provided." << endl;
		cout << "\t-b char: spaceship symbol. Only the first character will be assigned as symbol, if more than one character is provided." << endl;
		return 0;
	}
	
	window = initscr();

	initialize(height, width);

	startGame(symbol_obstacle, symbol_spaceship);

	cleanUp();
}
