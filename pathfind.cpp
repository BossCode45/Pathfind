#include <string.h>
#include <iostream>
#include <cmath>
#include <limits>
#include <fstream>
#include <chrono>
#include <thread>
#include <vector>
#include <stack>
#include <array>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <csignal>

#include "Board.h"

using namespace std;

Board genBoard(int cols, int rows);
Board genBoard(string boardPath);
Board genBoard(int cols, int rows, bool boardProvided, string boardPath);
bool solveBoard(Board boardInfo, bool show);

void drawBoard(Board boardInfo, bool last);
void drawBoard(Board boardInfo) {drawBoard(boardInfo, false);}

void exiting(int i);

enum boardStates {
	EMPTY,
	START,
	END,
	WALL
};
enum directions {
	NORTH,
	EAST,
	SOUTH,
	WEST
};

bool boardProvided = false;
Board board;
int delay = 25000000;
#ifndef _WIN32
string wallChar = "\033[37m\u2588\033\[0m";
string startChar = "\033[1;33mP\033[0m";
string endChar = "\033[1;34m@\033[0m";
string currentChar = "\033[1;32mC\033[0m";
#else
string wallChar = "#";
string startChar = "P";
string endChar = "@";
string currentChar = "C";
#endif


Board genBoard(int cols, int rows) { return genBoard(cols, rows, false, ""); }
Board genBoard(string boardPath) { return genBoard(0, 0, true, boardPath); }
Board genBoard(int cols, int rows, bool boardProvided, string boardPath)
{
	Board boardInfo;
	boardInfo.state = GENERATING;
	if(boardProvided)
	{
		ifstream indata;
		indata.open(boardPath);
		if(!indata)
		{
			cerr << "ERROR: board file couln't be opened!!!" << endl;
			throw std::invalid_argument("Invalid board file");
		}
		string l;
		getline(indata, l);

		cols = l.length();
		rows = 1;
		while(getline(indata, l))
			++rows;

		boardInfo.boardX = cols;
		boardInfo.boardY = rows;

		indata.clear();
		indata.seekg(0);

		int currentX=0;
		int currentY=0;
		while(getline(indata, l))
		{
			vector<int> row;
			for(char const &c: l)
			{
				int toPush = EMPTY;
				switch(c)
				{
					case '#' : toPush = WALL; break;
					case ' ' : toPush = EMPTY; break;
					case 'P' : toPush = START; boardInfo.sX = currentX; boardInfo.sY = currentY; break;
					case '@' : toPush = END; boardInfo.eX = currentX; boardInfo.eY = currentY;break;
				}
				row.push_back(toPush);
				currentX++;
			}
			currentY++;
			currentX = 0;

			boardInfo.boardValues.push_back(row);
		}
	}
	else
	{
		int sX = rand() % (cols - 1);
		int sY = rand() % (rows - 1);
		int eX = rand() % (cols - 1);
		int eY = rand() % (rows - 1);
		boardInfo.sX = sX;
		boardInfo.sY = sY;
		boardInfo.eX = eX;
		boardInfo.eY = eY;
		boardInfo.boardX = cols;
		boardInfo.boardY = rows;
		for(int y = 0; y < rows; y++)
		{
			vector<int> row;
			for(int x = 0; x < cols; x++)
			{
				row.push_back(WALL);
			}
			boardInfo.boardValues.push_back(row);
		}
		std::stack<std::array<int, 2>> stack;
		stack.push({0, 0});

		int visited = 0;
		int x, y;
		while(visited < ((rows+1)/2)*((cols+1)/2))
		{
			x = stack.top()[0];
			y = stack.top()[1];
			boardInfo.cX = x*2;
			boardInfo.cY = y*2;
			std::vector<int> dirs;

			if(y*2>0&&boardInfo.boardValues[(y-1)*2][(x)*2]!=EMPTY) { dirs.push_back(NORTH);}
			if(x*2<cols-2&&boardInfo.boardValues[(y)*2][(x+1)*2]!=EMPTY) { dirs.push_back(EAST); }
			if(y*2<rows-2&&boardInfo.boardValues[(y+1)*2][(x)*2]!=EMPTY) { dirs.push_back(SOUTH); }
			if(x*2>0&&boardInfo.boardValues[(y)*2][(x-1)*2]!=EMPTY) { dirs.push_back(WEST); }

			if(dirs.size()==0)
			{
				stack.pop();
			}
			else
			{
				int oX = 0;
				int oY = 0;
				switch(dirs[rand()%(dirs.size())])
				{
					case NORTH: oY = -1; break;
					case EAST: oX = 1; break;
					case SOUTH: oY = 1; break;
					case WEST: oX = -1; break;
				}
				boardInfo.boardValues[(y+oY)*2][(x+oX)*2] = EMPTY;
				boardInfo.boardValues[y*2+oY][x*2+oX] = EMPTY;
				stack.push({x + oX, y + oY});
				x = stack.top()[0];
				y = stack.top()[1];
				visited++;
				drawBoard(boardInfo);
			}
		}
		boardInfo.boardValues[eY][eX] = END;
		boardInfo.boardValues[sY][sX] = START;
	}
	return boardInfo;
}

bool solveBoard(Board boardInfo, bool show)
{
	vector<vector<int>> board = boardInfo.boardValues;
	int boardX = boardInfo.boardX;
	int boardY = boardInfo.boardY;
	int sX = boardInfo.sX;
	int sY = boardInfo.sY;
	int eX = boardInfo.eX;
	int eY = boardInfo.eY;
	double dists[boardY][boardX];
	double moves[boardY][boardX];
	double current[boardY][boardX];

	boardInfo.state = SOLVING;

	double r2 = 1.41404;

	for(int y = 0; y < boardY; y++)
	{
		vector<bool> movedCol;
		vector<bool> discoveredCol;
		vector<bool> pathCol;
		vector<vector<int>> fromCol;
		for(int x = 0; x < boardX; x++)
		{
			dists[y][x] = min(abs(eY-y), abs(eX-x))*r2 + max(abs(eY-y), abs(eX-x))-min(abs(eY-y), abs(eX-x));
			moves[y][x] = numeric_limits<double>::max();
			current[y][x] = false;

			movedCol.push_back(false);
			discoveredCol.push_back(false);
			pathCol.push_back(false);
			fromCol.push_back({-1, -1});
		}
		boardInfo.moved.push_back(movedCol);
		boardInfo.discovered.push_back(discoveredCol);
		boardInfo.path.push_back(pathCol);
		boardInfo.from.push_back(fromCol);
	}

	boardInfo.discovered[sY][sX] = true;
	moves[sY][sX] = 0;

	bool finished = false;
	bool found = false;

	while(!finished)
	{
		double minScore = numeric_limits<double>::max();
		double cY, cX;
		found = false;

		for(int y = 0; y < boardY; y++)
		{
			for(int x = 0; x < boardX; x++)
			{
				bool currMoved = (!current[y][x]|(current[y][x]&&!boardInfo.moved[y][x]));
				if(boardInfo.discovered[y][x]&&moves[y][x] + dists[y][x]<minScore&&currMoved)
				{
					minScore = moves[y][x] + dists[y][x];
					cY = y;
					cX = x;
					found = true;
				}
			}
		}

		if(!found) finished = true;

		boardInfo.cX = cX;
		boardInfo.cY = cY;
		
		current[(int)cY][(int)cX] = true;
		boardInfo.moved[(int)cY][(int)cX] = true;
		for(int dY = -1; dY < 2; dY++)
		{
			for(int dX = -1; dX < 2; dX++)
			{
				if(cY+dY<0|cX+dX<0|cY+dY>boardY-1|cX+dX>boardX-1)
				{
					continue;
				}
				if(dY==0&&dX==0)
				{
					continue;
				}
				else if(cY+dY==eY&&cX+dX==eX)
				{
					finished = true;

					boardInfo.path[cY+dY][cX+dX] = true;

					int pX = boardInfo.from[(int)cY][(int)cX][0];
					int pY = boardInfo.from[(int)cY][(int)cX][1];
					boardInfo.path[pY][pX] = true;
					boardInfo.path[(int)cY][(int)cX] = true;
					int finalDist = 2;
					while(!(pX==sX&&pY==sY))
					{
						int ppX = pX;
						int ppY = pY;
						pX = boardInfo.from[ppY][ppX][0];
						pY = boardInfo.from[ppY][ppX][1];
						boardInfo.path[pY][pX] = true;
						finalDist++;
					}
					//if(show) {cout << finalDist << endl;};
				}

				if(board[(int)cY+dY][(int)cX+dX] == WALL)
				{
					continue;
				}
				
				if(dY!=0&&dX!=0)
				{
					double pMove = moves[(int)cY][(int)cX] + r2;
					if(pMove < moves[(int)cY+dY][(int)cX+dX])
					{
						moves[(int)cY+dY][(int)cX+dX] = pMove;
						boardInfo.from[(int)cY+dY][(int)cX+dX][0] = cX;
						boardInfo.from[(int)cY+dY][(int)cX+dX][1] = cY;

						current[(int)cY+dY][(int)cX+dX] = false;
					}
					else
					{
						current[(int)cY+dY][(int)cX+dX] = true;
					}
				}
				else
				{
					double pMove = moves[(int)cY][(int)cX] + 1;
					if(pMove < moves[(int)cY+dY][(int)cX+dX])
					{
						moves[(int)cY+dY][(int)cX+dX] = pMove;
						boardInfo.from[(int)cY+dY][(int)cX+dX][0] = cX;
						boardInfo.from[(int)cY+dY][(int)cX+dX][1] = cY;

						current[(int)cY+dY][(int)cX+dX] = false;
					}
					else
					{
						current[(int)cY+dY][(int)cX+dX] = true;
					}
				}
				
				boardInfo.discovered[(int)cY+dY][(int)cX+dX] = true;
			}
		}
		if(show) { drawBoard(boardInfo); }
	}

	if(!found) { return false; }

	boardInfo.state = FINISHED;

	if(show) { drawBoard(boardInfo, true); }
	return true;
}

void drawBoard(Board boardInfo, bool last)
{
	int eY = boardInfo.eY;
	int eX = boardInfo.eX;
	int sY = boardInfo.sY;
	int sX = boardInfo.sX;
	int boardY = boardInfo.boardY;
	int boardX = boardInfo.boardX;

	int cX = boardInfo.cX;
	int cY = boardInfo.cY;


	string lines = "";

	for(int y = -1; y < boardY + 1; y++)
	{
		string line = wallChar;
		for(int x = 0; x < boardX; x++)
		{
			if(y==-1 || y==boardY)
			{
				line+=wallChar;
			}
			else if(y==eY&&x==eX)
			{
				line+=endChar;
			}
			else if(y==sY&&x==sX)
			{
				line+=startChar;
			}
			else if(boardInfo.boardValues[y][x]==WALL)
			{
				line+=wallChar;
			}
			else if(y==cY && x==cX && boardInfo.state!=FINISHED)
			{
				line+=currentChar;
			}
			else if(boardInfo.state>=FINISHED && (boardInfo.path[y][x]||(y==cY&&x==cX)))
			{
				if(boardInfo.from[y][x][1]==y)
				{
					line+="-";
					continue;
				}
				else if(boardInfo.from[y][x][0]==x)
				{
					line+="|";
				}
				else if((boardInfo.from[y][x][1]>y&&boardInfo.from[y][x][0]>x)||(boardInfo.from[y][x][1]<y&&boardInfo.from[y][x][0]<x))
				{
					line+="\\";
				}
				else
				{
					line+="/";
				}
			}
			else if(boardInfo.state==SOLVING && boardInfo.moved[y][x])
			{
				line+=" ";
			}
			else if(boardInfo.state>=SOLVING && boardInfo.discovered[y][x])
			{
				line+="+";
			}
			else if(boardInfo.state==SOLVING)
			{
				line+="'";
			}
			else
			{
				line+=" ";
			}
		}
		if(y!=boardY)
		{
			lines += line + wallChar + "\n";
		}
		else
		{
			lines += line + wallChar;
		}
	}
	cout << lines;
	this_thread::sleep_for(std::chrono::nanoseconds(delay));
	if(!last) { printf("\033[%d;%dH", (0), (0)); }
}

int main(int argc, char *argv[])
{
	srand(time(NULL));

	printf("\e[?25l");

    signal(SIGINT, exiting);
    signal(SIGABRT, exiting);
    signal(SIGTERM, exiting);
    signal(SIGTSTP, exiting);

	int cols = 39;
	int rows = 39;
	bool boardProvided = false;
	string boardPath = "";

	string error = "The provided arguements are wrong!!!\n\nSee 'pathfind -h' for help!\n";
	for(int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-h") == 0)
		{
			cout << "Usage: pathfind <flags>\n";
			cout << "Flags:\n";
			cout << " -b : The path to a board.txt file\n";
			cout << " -c : The amount of columns (width) - Default 20 (Note: this doesn't actually correspond to the amount of rows in your terminal it will take up, just the amount of empty cells possible)\n";
			cout << " -r : The amount of rows (height) - Default 20 (Note: this doesn't actually correspond to the amount of rows in your terminal it will take up, just the amount of empty cells possible)\n";
			//40*40 by default
			cout << " -h : Shows help and exits\n";
			cout << " -t : Time delay between draws of the pathfind visualisation (ms) - Default 25\n";
			cout << " -f : Fills the terminal screen\n";
			cout << "\nThe characters in the output:\n";
			cout << " " + endChar + " : Used to mark the end (Goes in board.txt)\n";
			cout << " " + startChar + " : Used to mark the start (Goes in board.txt)\n";
			cout << " " + wallChar + " : Used to mark a wall ( # goes in board.txt)\n";
			cout << " (empty) : Used to mark an empty (Goes in board.txt)\n";
			cout << " + : Used to mark cells that have been looked at but not entered yet\n";
			cout << " -|/\\ : Used to mark the final path (Only in final draw)\n";
			cout << " " + currentChar + " : Used to mark the current cell (Only in pathfind visualisation)\n";
			cout << " ' : Used to mark cells that haven't yet been looked at (Only in pathfind visualisation)\n";
			return 0;
		}
		else if(strcmp(argv[i], "-c") == 0)
		{
			if(argc < i+1) { cerr << error; return 1; }
			cols = stoi(argv[i+1]) * 2 - 1;
			i++;
		}
		else if(strcmp(argv[i], "-r") == 0)
		{
			if(argc < i+1) { cerr << error; return 1; }
			rows = stoi(argv[i+1]) * 2 - 1;
			i++;
		}
		else if(strcmp(argv[i], "-b") == 0)
		{
			if(argc < i+1) { cerr << error; return 1; }
			boardProvided = true;
			boardPath = argv[i+1];
			i++;
		}
		else if(strcmp(argv[i], "-t") == 0)
		{
			if(argc < i+1) { cerr << error; return 1; }
			delay = stoi(argv[i+1])*1000000;
			i++;
		}
		else if(strcmp(argv[i], "-f") == 0)
		{
			struct winsize w;
			ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
			rows = w.ws_row - 2;
			cols = w.ws_col - 2;
		}

	}

	system("clear");

	board = genBoard(cols, rows, boardProvided, boardPath);

	system("clear");
	printf("\033[%d;%dH", (0), (0));
	solveBoard(board, true);

	exiting(0);

	return 0;
}

void exiting(int i)
{
	printf("\e[?25h\n");
	exit(0);
}
