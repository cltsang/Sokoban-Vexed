#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct Object{
	char symbol;
	int y;
	int x;
	bool ignoreGravity;
};

const int verticalStep = 2;
const int horizontalStep = 4;
int numLevels;
int input;
char *board;
int boardWidth, boardHeight;
struct Object *movableObjects;
int numMovableObjects;
char *controlSequence;
int controlSequenceSize;

bool blueMagnetActivated;
bool redMagnetActivated;
bool greenMagnetActivated;
bool gameOver;
struct Object *aboveHarry;

void displayMessage(char *msg){
	int x;
	attron(COLOR_PAIR(1));
	mvprintw(boardHeight * verticalStep, 0, "%s", msg);
	// fill the remaining with spaces
	for(x=strlen(msg); x < boardWidth * horizontalStep; x++)
		mvaddch(boardHeight * verticalStep, x, ' ');
	refresh();
}

int cursorXToConsoleX(int x){
	return horizontalStep * x;
}
int cursorYToConsoleY(int y){
	return verticalStep * y;
}

struct Object* getObject(int y, int x){
	struct Object *target = NULL;
	int i;
	
	for(i = 0; i < numMovableObjects; i++){
		if(y == movableObjects[i].y && x == movableObjects[i].x)
			target = &(movableObjects[i]);
	}
	return target;
}

struct Object *getPlayer(){
	int i;
	for(i = 0; i < numMovableObjects; i++)
		if(movableObjects[i].symbol == 'P')
			return &(movableObjects[i]);
	return NULL;
}

char getElement(int y, int x){
	if(y >= boardHeight || x >= boardWidth)
		return '\0';
	return board[y * boardWidth + x];
}

void printGrid(int y, int x, char symbol){
	int cursorX = cursorXToConsoleX(x);
	int cursorY = cursorYToConsoleY(y);
	char element;
	switch(symbol){
	case 'P': // Human
		if(getPlayer()->ignoreGravity){
			attron(COLOR_PAIR(1));
			mvaddch(cursorY+1, cursorX, ACS_DIAMOND);
			mvaddch(cursorY, cursorX+3, '/');
			attroff(COLOR_PAIR(1));
		}
		
		element = getElement(y, x);
		if(element == 'v'){
			attron(COLOR_PAIR(2));
			mvaddch(cursorY+1, cursorX+1, '/');
			mvaddch(cursorY+1, cursorX+2, '\\');
			attroff(COLOR_PAIR(2));
			
			if(blueMagnetActivated)
				attron(COLOR_PAIR(2));
			else
				attron(COLOR_PAIR(1));				
			mvaddch(cursorY, cursorX+1, ACS_LEQUAL);
			mvaddch(cursorY, cursorX+2, ACS_GEQUAL);
			if(blueMagnetActivated)
				attroff(COLOR_PAIR(2));
			else
				attroff(COLOR_PAIR(1));
		}
		else if(element == 'a'){
			attron(COLOR_PAIR(3));
			mvaddch(cursorY+1, cursorX+1, '/');
			mvaddch(cursorY+1, cursorX+2, '\\');
			attroff(COLOR_PAIR(3));
			
			if(redMagnetActivated)
				attron(COLOR_PAIR(3));
			else
				attron(COLOR_PAIR(1));				
			mvaddch(cursorY, cursorX+1, ACS_LEQUAL);
			mvaddch(cursorY, cursorX+2, ACS_GEQUAL);
			if(redMagnetActivated)
				attroff(COLOR_PAIR(3));
			else
				attroff(COLOR_PAIR(1));
		}
		else if(element == 'o'){
			attron(COLOR_PAIR(6));
			mvaddch(cursorY+1, cursorX+1, '/');
			mvaddch(cursorY+1, cursorX+2, '\\');
			attroff(COLOR_PAIR(6));
			
			if(greenMagnetActivated)
				attron(COLOR_PAIR(6));
			else
				attron(COLOR_PAIR(1));				
			mvaddch(cursorY, cursorX+1, ACS_LEQUAL);
			mvaddch(cursorY, cursorX+2, ACS_GEQUAL);
			if(greenMagnetActivated)
				attroff(COLOR_PAIR(6));
			else
				attroff(COLOR_PAIR(1));
		}
		else{
			attron(COLOR_PAIR(1));
			mvaddch(cursorY, cursorX+1, ACS_LEQUAL);
			mvaddch(cursorY, cursorX+2, ACS_GEQUAL);
			mvaddch(cursorY+1, cursorX+1, '/');
			mvaddch(cursorY+1, cursorX+2, '\\');
			attroff(COLOR_PAIR(1));
		}
		break;
	case '#': // Rock
		attron(COLOR_PAIR(1));
		mvaddch(cursorY, cursorX, ACS_CKBOARD);
		mvaddch(cursorY, cursorX+1, ACS_CKBOARD);
		mvaddch(cursorY, cursorX+2, ACS_CKBOARD);
		mvaddch(cursorY, cursorX+3, ACS_CKBOARD);
		mvaddch(cursorY+1, cursorX, ACS_CKBOARD);
		mvaddch(cursorY+1, cursorX+1, ACS_CKBOARD);
		mvaddch(cursorY+1, cursorX+2, ACS_CKBOARD);
		mvaddch(cursorY+1, cursorX+3, ACS_CKBOARD);
		attroff(COLOR_PAIR(1));
		break;
	case 'H': // Ladder
		attron(COLOR_PAIR(1));
		mvaddch(cursorY, cursorX, ACS_RTEE);
		mvaddch(cursorY, cursorX+1, ACS_HLINE);
		mvaddch(cursorY, cursorX+2, ACS_HLINE);
		mvaddch(cursorY, cursorX+3, ACS_LTEE);
		mvaddch(cursorY+1, cursorX, ACS_RTEE);
		mvaddch(cursorY+1, cursorX+1, ACS_HLINE);
		mvaddch(cursorY+1, cursorX+2, ACS_HLINE);
		mvaddch(cursorY+1, cursorX+3, ACS_LTEE);
		attroff(COLOR_PAIR(1));
		break;
	case ' ': // empty space
		attron(COLOR_PAIR(1));
		mvaddch(cursorY, cursorX, ' ');
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY, cursorX+3, ' ');
		mvaddch(cursorY+1, cursorX, ' ');
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		mvaddch(cursorY+1, cursorX+3, ' ');
		attroff(COLOR_PAIR(1));
		break;
	case 'b': // blue box
		attron(COLOR_PAIR(2));
		mvaddch(cursorY, cursorX, ' ');
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY, cursorX+3, ' ');
		mvaddch(cursorY+1, cursorX, ' ');
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		mvaddch(cursorY+1, cursorX+3, ' ');
		attroff(COLOR_PAIR(2));
		break;
	case 'r': // red box
		attron(COLOR_PAIR(3));
		mvaddch(cursorY, cursorX, ' ');
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY, cursorX+3, ' ');
		mvaddch(cursorY+1, cursorX, ' ');
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		mvaddch(cursorY+1, cursorX+3, ' ');
		attroff(COLOR_PAIR(3));
		break;
	case 'c': // cyan box
		attron(COLOR_PAIR(4));
		mvaddch(cursorY, cursorX, ' ');
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY, cursorX+3, ' ');
		mvaddch(cursorY+1, cursorX, ' ');
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		mvaddch(cursorY+1, cursorX+3, ' ');
		attroff(COLOR_PAIR(4));
		break;
	case 'm': // magenta box
		attron(COLOR_PAIR(5));
		mvaddch(cursorY, cursorX, ' ');
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY, cursorX+3, ' ');
		mvaddch(cursorY+1, cursorX, ' ');
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		mvaddch(cursorY+1, cursorX+3, ' ');
		attroff(COLOR_PAIR(5));
		break;
	case 'g': // green box
		attron(COLOR_PAIR(6));
		mvaddch(cursorY, cursorX, ' ');
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY, cursorX+3, ' ');
		mvaddch(cursorY+1, cursorX, ' ');
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		mvaddch(cursorY+1, cursorX+3, ' ');
		attroff(COLOR_PAIR(6));
		break;
	case 'y': // yellow box
		attron(COLOR_PAIR(7));
		mvaddch(cursorY, cursorX, ' ');
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY, cursorX+3, ' ');
		mvaddch(cursorY+1, cursorX, ' ');
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		mvaddch(cursorY+1, cursorX+3, ' ');
		attroff(COLOR_PAIR(7));
		break;
	case 'l': // black box
		attron(COLOR_PAIR(8));
		mvaddch(cursorY, cursorX, ' ');
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY, cursorX+3, ' ');
		mvaddch(cursorY+1, cursorX, ' ');
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		mvaddch(cursorY+1, cursorX+3, ' ');
		attroff(COLOR_PAIR(8));
		break;
	
	case '-': // broom
		attron(COLOR_PAIR(1));
		mvaddch(cursorY, cursorX, ' ');
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, '_');
		mvaddch(cursorY, cursorX+3, '/');
		mvaddch(cursorY+1, cursorX, ACS_DIAMOND);
		mvaddch(cursorY+1, cursorX+1, ACS_LRCORNER);
		mvaddch(cursorY+1, cursorX+2, ' ');
		mvaddch(cursorY+1, cursorX+3, ' ');
		attroff(COLOR_PAIR(1));
		break;
	case 'D': // brick
		attron(COLOR_PAIR(7));
		mvaddch(cursorY, cursorX, '#');
		mvaddch(cursorY, cursorX+1, '#');
		mvaddch(cursorY, cursorX+2, '#');
		mvaddch(cursorY, cursorX+3, '#');
		mvaddch(cursorY+1, cursorX, '#');
		mvaddch(cursorY+1, cursorX+1, '#');
		mvaddch(cursorY+1, cursorX+2, '#');
		mvaddch(cursorY+1, cursorX+3, '#');
		attroff(COLOR_PAIR(7));
		break;
	case 'v': // blue scroll
		if(!blueMagnetActivated){
			attron(COLOR_PAIR(1));
			mvaddch(cursorY, cursorX, ' ');
			mvaddch(cursorY, cursorX+1, ' ');
			mvaddch(cursorY, cursorX+2, ' ');
			mvaddch(cursorY, cursorX+3, ' ');
			mvaddch(cursorY+1, cursorX, ' ');
			mvaddch(cursorY+1, cursorX+3, ' ');
			attroff(COLOR_PAIR(1));
			attron(COLOR_PAIR(2));
			mvaddch(cursorY+1, cursorX+1, ' ');
			mvaddch(cursorY+1, cursorX+2, ' ');
			attroff(COLOR_PAIR(2));
		}
		else{
			attron(COLOR_PAIR(1));
			mvaddch(cursorY, cursorX, ACS_ULCORNER);
			mvaddch(cursorY, cursorX+3, ACS_URCORNER);
			mvaddch(cursorY+1, cursorX, ACS_LLCORNER);
			mvaddch(cursorY+1, cursorX+3, ACS_LRCORNER);
			attroff(COLOR_PAIR(1));
			attron(COLOR_PAIR(2));
			mvaddch(cursorY, cursorX+1, ACS_TTEE);
			mvaddch(cursorY, cursorX+2, ACS_TTEE);
			mvaddch(cursorY+1, cursorX+1, ACS_BTEE);
			mvaddch(cursorY+1, cursorX+2, ACS_BTEE);
			attroff(COLOR_PAIR(2));
		}
		break;
	case 'a': // red scroll
		if(!redMagnetActivated){
			attron(COLOR_PAIR(1));
			mvaddch(cursorY, cursorX, ' ');
			mvaddch(cursorY, cursorX+1, ' ');
			mvaddch(cursorY, cursorX+2, ' ');
			mvaddch(cursorY, cursorX+3, ' ');
			mvaddch(cursorY+1, cursorX, ' ');
			mvaddch(cursorY+1, cursorX+3, ' ');
			attroff(COLOR_PAIR(1));
			attron(COLOR_PAIR(3));
			mvaddch(cursorY+1, cursorX+1, ' ');
			mvaddch(cursorY+1, cursorX+2, ' ');
			attroff(COLOR_PAIR(3));
		}
		else{
			attron(COLOR_PAIR(1));
			mvaddch(cursorY, cursorX, ACS_ULCORNER);
			mvaddch(cursorY, cursorX+3, ACS_URCORNER);
			mvaddch(cursorY+1, cursorX, ACS_LLCORNER);
			mvaddch(cursorY+1, cursorX+3, ACS_LRCORNER);
			attroff(COLOR_PAIR(1));
			attron(COLOR_PAIR(3));
			mvaddch(cursorY, cursorX+1, ACS_TTEE);
			mvaddch(cursorY, cursorX+2, ACS_TTEE);
			mvaddch(cursorY+1, cursorX+1, ACS_BTEE);
			mvaddch(cursorY+1, cursorX+2, ACS_BTEE);
			attroff(COLOR_PAIR(3));
		}
		break;
	case 'o': // green scroll
		if(!greenMagnetActivated){
			attron(COLOR_PAIR(1));
			mvaddch(cursorY, cursorX, ' ');
			mvaddch(cursorY, cursorX+1, ' ');
			mvaddch(cursorY, cursorX+2, ' ');
			mvaddch(cursorY, cursorX+3, ' ');
			mvaddch(cursorY+1, cursorX, ' ');
			mvaddch(cursorY+1, cursorX+3, ' ');
			attroff(COLOR_PAIR(1));
			attron(COLOR_PAIR(6));
			mvaddch(cursorY+1, cursorX+1, ' ');
			mvaddch(cursorY+1, cursorX+2, ' ');
			attroff(COLOR_PAIR(6));
		}
		else{
			attron(COLOR_PAIR(1));
			mvaddch(cursorY, cursorX, ACS_ULCORNER);
			mvaddch(cursorY, cursorX+3, ACS_URCORNER);
			mvaddch(cursorY+1, cursorX, ACS_LLCORNER);
			mvaddch(cursorY+1, cursorX+3, ACS_LRCORNER);
			attroff(COLOR_PAIR(1));
			attron(COLOR_PAIR(6));
			mvaddch(cursorY, cursorX+1, ACS_TTEE);
			mvaddch(cursorY, cursorX+2, ACS_TTEE);
			mvaddch(cursorY+1, cursorX+1, ACS_BTEE);
			mvaddch(cursorY+1, cursorX+2, ACS_BTEE);
			attroff(COLOR_PAIR(6));
		}
		break;
	case 'V': // blue magnet
		attron(COLOR_PAIR(2));
		mvaddch(cursorY, cursorX, ' ');
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY, cursorX+3, ' ');
		mvaddch(cursorY+1, cursorX, ' ');
		mvaddch(cursorY+1, cursorX+3, ' ');
		attroff(COLOR_PAIR(2));
		attron(COLOR_PAIR(1));
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		attroff(COLOR_PAIR(1));
		break;
	case 'A': // red magnet
		attron(COLOR_PAIR(3));
		mvaddch(cursorY, cursorX, ' ');
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY, cursorX+3, ' ');
		mvaddch(cursorY+1, cursorX, ' ');
		mvaddch(cursorY+1, cursorX+3, ' ');
		attroff(COLOR_PAIR(3));
		attron(COLOR_PAIR(1));
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		attroff(COLOR_PAIR(1));
		break;
	case 'O': // green magnet
		attron(COLOR_PAIR(6));
		mvaddch(cursorY, cursorX, ' ');
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY, cursorX+3, ' ');
		mvaddch(cursorY+1, cursorX, ' ');
		mvaddch(cursorY+1, cursorX+3, ' ');
		attroff(COLOR_PAIR(6));
		attron(COLOR_PAIR(1));
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		attroff(COLOR_PAIR(1));
		break;
	case 'B': // blue bludgerr
		attron(COLOR_PAIR(1));
		mvaddch(cursorY, cursorX, '/');
		mvaddch(cursorY, cursorX+3, '\\');
		mvaddch(cursorY+1, cursorX, '\\');
		mvaddch(cursorY+1, cursorX+3, '/');
		attroff(COLOR_PAIR(1));
		attron(COLOR_PAIR(2));
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		attroff(COLOR_PAIR(2));
		break;
	case 'R': // red bludgerr
		attron(COLOR_PAIR(1));
		mvaddch(cursorY, cursorX, '/');
		mvaddch(cursorY, cursorX+3, '\\');
		mvaddch(cursorY+1, cursorX, '\\');
		mvaddch(cursorY+1, cursorX+3, '/');
		attroff(COLOR_PAIR(1));
		attron(COLOR_PAIR(3));
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		attroff(COLOR_PAIR(3));
		break;
	case 'C': // cyan bludgerr
		attron(COLOR_PAIR(1));
		mvaddch(cursorY, cursorX, '/');
		mvaddch(cursorY, cursorX+3, '\\');
		mvaddch(cursorY+1, cursorX, '\\');
		mvaddch(cursorY+1, cursorX+3, '/');
		attroff(COLOR_PAIR(1));
		attron(COLOR_PAIR(4));
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		attroff(COLOR_PAIR(4));
		break;
	case 'M': // magenta bludgerr
		attron(COLOR_PAIR(1));
		mvaddch(cursorY, cursorX, '/');
		mvaddch(cursorY, cursorX+3, '\\');
		mvaddch(cursorY+1, cursorX, '\\');
		mvaddch(cursorY+1, cursorX+3, '/');
		attroff(COLOR_PAIR(1));
		attron(COLOR_PAIR(5));
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		attroff(COLOR_PAIR(5));
		break;
	case 'G': // green bludgerr
		attron(COLOR_PAIR(1));
		mvaddch(cursorY, cursorX, '/');
		mvaddch(cursorY, cursorX+3, '\\');
		mvaddch(cursorY+1, cursorX, '\\');
		mvaddch(cursorY+1, cursorX+3, '/');
		attroff(COLOR_PAIR(1));
		attron(COLOR_PAIR(6));
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		attroff(COLOR_PAIR(6));
		break;
	case 'Y': // yellow bludgerr
		attron(COLOR_PAIR(1));
		mvaddch(cursorY, cursorX, '/');
		mvaddch(cursorY, cursorX+3, '\\');
		mvaddch(cursorY+1, cursorX, '\\');
		mvaddch(cursorY+1, cursorX+3, '/');
		attroff(COLOR_PAIR(1));
		attron(COLOR_PAIR(7));
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		attroff(COLOR_PAIR(7));
		break;
	case 'L': // black bludgerr
		attron(COLOR_PAIR(1));
		mvaddch(cursorY, cursorX, '/');
		mvaddch(cursorY, cursorX+3, '\\');
		mvaddch(cursorY+1, cursorX, '\\');
		mvaddch(cursorY+1, cursorX+3, '/');
		attroff(COLOR_PAIR(1));
		attron(COLOR_PAIR(8));
		mvaddch(cursorY, cursorX+1, ' ');
		mvaddch(cursorY, cursorX+2, ' ');
		mvaddch(cursorY+1, cursorX+1, ' ');
		mvaddch(cursorY+1, cursorX+2, ' ');
		attroff(COLOR_PAIR(8));
		break;
	}
	//mvaddch(cursorY, cursorX, symbol);
}

void printBoard(){
	int row, column;
	for(row = 0; row < boardHeight; row++)
		for(column = 0; column < boardWidth; column++)
			printGrid(row, column, board[row*boardWidth + column]);
}

void printObjects(){
	int i;
	for(i = 0; i < numMovableObjects; i++)
		printGrid(movableObjects[i].y, movableObjects[i].x, movableObjects[i].symbol);
}

void clearBackGround(){
	int row, column;
	for(row = 0; row < boardHeight + 2; row++)
		for(column = 0; column < boardWidth + 2; column++)
			printGrid(row, column, 'l');
	refresh();
}

void initializeNcurses(){
	curs_set(0);
	cbreak();
	noecho();
	keypad(stdscr, true);
	start_color();
	init_pair(1, COLOR_BLACK, COLOR_WHITE); // empty space, human, ladder, rock; broom, 
	init_pair(2, COLOR_BLACK, COLOR_BLUE); // blue box; blue bludgerr, scroll magnet
	init_pair(3, COLOR_BLACK, COLOR_RED); // red box; red bludgerr, scroll magnet
	init_pair(4, COLOR_BLACK, COLOR_CYAN); // cyan box; cyan bludgerr
	init_pair(5, COLOR_BLACK, COLOR_MAGENTA); // magenta box; magenta bludgerr
	init_pair(6, COLOR_BLACK, COLOR_GREEN); // green box; green bludgerr, green bludgerr scroll magnet
	init_pair(7, COLOR_BLACK, COLOR_YELLOW); // yellow box; yellow bludgerr, brick
	init_pair(8, COLOR_BLACK, COLOR_BLACK); // black box, background; black bludgerr
}

void readFileName(char **fileName){
	int fileNameLength = 0;
	int c;
	attron(COLOR_PAIR(1));
	mvprintw(boardHeight * verticalStep, 0, "file name:");
	echo();
	
	while(true){
		c = getch();
		*fileName = realloc(*fileName, fileNameLength + 1);
		if(c == '\r' || c == '\n'){
			*((*fileName)+fileNameLength+1) = '\0';
			break;
		}
		*((*fileName)+fileNameLength++) = c;
	}
	
	displayMessage("");
	sprintf(*fileName, "%.*s", fileNameLength, *fileName);
	attroff(COLOR_PAIR(1));
	noecho();
}

bool fileIsValid(char *fileName){
	if(access(fileName, F_OK | R_OK) != -1)
		return true;
	else
		return false;
}

bool readControlSequenceFile(char *fileName){
	char control, space;
	
	if(!fileIsValid(fileName))
		return false;
	controlSequenceSize = 0;
	FILE *controlSequenceFile = fopen(fileName, "r");
	while(true){
		control = fgetc(controlSequenceFile);
		space = fgetc(controlSequenceFile);
		if(control == 'u' || control == 'd' || control == 'l' || control == 'r'){
			// replace 'r' to avoid collision with reset button
			if(control == 'r')
				control = '>';
			controlSequence = realloc(controlSequence, controlSequenceSize + 1);
			controlSequence[controlSequenceSize++] = control;
		}
		else
			break;
		if(space != ' ')
			break;
	}
	
	fclose(controlSequenceFile);
	return true;
}

bool readMapFile(char *fileName){
	char element;
	int row, column;
//displayMessage(fileName);
	
	if(!fileIsValid(fileName))
		return false;
	FILE *mapFile = fopen(fileName, "r");
	fscanf(mapFile, "%d %d", &boardWidth, &boardHeight);
	board = malloc(boardWidth * boardHeight);
	numMovableObjects = 0;
	for(row = 0; row < boardHeight; row++){
		for(column = 0; column < boardWidth; column++){
			do{
				element = fgetc(mapFile);
			}while(element == '\r' || element == '\n');
			if(element=='#' || element=='H' || element==' ' || element=='-' || element=='D'
			|| element=='v' || element=='a' || element=='o' || element=='V' || element=='A' || element=='O'){
				board[row*boardWidth + column] = element;
			}
			// P, brcmgyl, BRCMGYL
			else{
				board[row*boardWidth + column] = ' ';
				numMovableObjects++;
				movableObjects = realloc(movableObjects, numMovableObjects * sizeof(struct Object));
				movableObjects[numMovableObjects-1].symbol = element;
				movableObjects[numMovableObjects-1].y = row;
				movableObjects[numMovableObjects-1].x = column;
				movableObjects[numMovableObjects-1].ignoreGravity = false;
			}
		}
	}

	fclose(mapFile);
	return true;
}

void removeElement(int y, int x){
	board[y * boardWidth + x] = ' ';
}

bool hasSameTypeColorNeighbour(int y, int x, char symbol){
	// ul, uc, ur; cl, cr; ll, lc, lr
	struct Object *u = getObject(y-1, x);
	struct Object *l = getObject(y, x-1);
	struct Object *r = getObject(y, x+1);
	struct Object *b = getObject(y+1, x);
	
	if(u != NULL)
		if(symbol == u->symbol)
			return true;
	if(l != NULL)
		if(symbol == l->symbol)
			return true;
	if(r != NULL)
		if(symbol == r->symbol)
			return true;
	if(b != NULL)
		if(symbol == b->symbol)
			return true;
	
	return false;
}

bool objectIsABox(struct Object *object){
	if(object->symbol == 'b' || object->symbol == 'r' || object->symbol == 'c' || object->symbol == 'm'
	|| object->symbol == 'g' || object->symbol == 'y' || object->symbol == 'l')
		return true;
	return false;
}

bool objectIsAbludger(struct Object *object){
	if(object->symbol == 'B' || object->symbol == 'R' || object->symbol == 'C' || object->symbol == 'M'
	|| object->symbol == 'G' || object->symbol == 'Y' || object->symbol == 'L')
		return true;
	return false;
}

bool moveObject(struct Object *source, int velocityY, int velocityX){
	bool blocked;
	int targetY = source->y + velocityY;
	int targetX = source->x + velocityX;
	char element = getElement(targetY, targetX);
	char elementAbove;
	struct Object *sink = getObject(targetY, targetX);
	struct Object *objectBelowMagnet = NULL;
	int y, x, magnetY, magnetX;

	if(sink != NULL){
		if(source->symbol == 'P'){
			if(velocityY == 0)
				blocked = !moveObject(sink, velocityY, velocityX);
			else // cannot push box or bludger in vertical direction
				blocked = true;
		}
		// a bludger falling to Harry
		else if(objectIsAbludger(source) && sink->symbol == 'P' && aboveHarry != source && velocityY > 0){
			blocked = true;
			gameOver = true;
		}
		else // boxex or bludger with different colors
			blocked = true;
	}
	else{
		// space, ladder, spell scroll
		if(element == ' ' || element == 'H' || element == 'v' || element == 'a' || element == 'o')
			blocked = false;
		// rock, magnet, brick, broom
		else
			blocked = true;
		
		if(source->symbol =='P'){
			/*
			if(element == 'D'){
				// remove the brick
				removeElement(targetY, targetX);
				blocked = false;
			}
			*/
			if(element == '-'){
				removeElement(targetY, targetX);
				getPlayer()->ignoreGravity = true;
				blocked = false;
			}
			
			if(element == 'v' || element == 'a' || element == 'o'){
				for(y = 0; y < boardHeight; y++)
					for(x = 0; x < boardWidth; x++)
						if(getElement(y, x) == element - 32){
							magnetY = y;
							magnetX = x;
							break;
						}
				
				objectBelowMagnet = getObject(magnetY+1, magnetX);
				if(element == 'v'){
					if(blueMagnetActivated){
						blueMagnetActivated = false;
						if(objectBelowMagnet != NULL)
							objectBelowMagnet->ignoreGravity = false;
					}
					else
						blueMagnetActivated = true;
				}
				if(element == 'a'){
					if(redMagnetActivated){
						redMagnetActivated = false;
						if(objectBelowMagnet != NULL)
							objectBelowMagnet->ignoreGravity = false;
					}
					else
						redMagnetActivated = true;
				}
				if(element == 'o'){
					if(greenMagnetActivated){
						greenMagnetActivated = false;
						if(objectBelowMagnet != NULL)
							objectBelowMagnet->ignoreGravity = false;
					}
					else
						greenMagnetActivated = true;
				}
			}
		}
		
		// a magnet above the object, and the object is a box or a bludger
		else if(source->ignoreGravity && velocityX != 0){
			elementAbove = getElement(source->y-1, source->x);
			source->ignoreGravity = false;
		}
	}
	
	if(!blocked){
		source->y = targetY;
		source->x = targetX;
		return true;
	}
	return false;
}

void handleInput(){
	struct Object *player = getPlayer();
	char *controlSequenceFileName;
	
	switch(input){
		case 'r':
			break;
		case 's':
			controlSequenceFileName = NULL;
			readFileName(&controlSequenceFileName);
			readControlSequenceFile(controlSequenceFileName); // "maps/sol1.txt"
			free(controlSequenceFileName);
			break;
		case KEY_UP: case 'u':
			if(getPlayer()->ignoreGravity || (getElement(player->y, player->x) == 'H' || getElement(player->y-1, player->x) == 'H')){
				if(getElement(player->y - 1, player->x) == 'D')
					removeElement(player->y - 1, player->x);
				moveObject(player, -1, 0);
			}
			break;
		case KEY_DOWN: case 'd':
			if(getPlayer()->ignoreGravity || (getElement(player->y, player->x) == 'H' || getElement(player->y+1, player->x) == 'H')){
				if(getElement(player->y + 1, player->x) == 'D')
					removeElement(player->y + 1, player->x);
				moveObject(player, 1, 0);
			}
			break;
		case KEY_LEFT: case 'l':
			if(getElement(player->y, player->x - 1) == 'D')
				removeElement(player->y, player->x - 1);
			moveObject(player, 0, -1);
			break;
		case KEY_RIGHT: case '>':
			if(getElement(player->y, player->x + 1) == 'D')
				removeElement(player->y, player->x + 1);
			moveObject(player, 0, 1);
			break;
		default:
			break;
	}
}

bool removeBoxesAndbludgers(){
	int i, originalSize = numMovableObjects;
	bool toBeRemoved[numMovableObjects];
	for(i = 0; i < numMovableObjects; i++){
		if(movableObjects[i].symbol == 'P') // only one Harry
			toBeRemoved[i] = false;
		toBeRemoved[i] = hasSameTypeColorNeighbour(movableObjects[i].y, movableObjects[i].x, movableObjects[i].symbol);
	}
	
	for(i = 0; i < numMovableObjects; ){
		if(toBeRemoved[i]){
			movableObjects[i].y = movableObjects[numMovableObjects-1].y;
			movableObjects[i].x = movableObjects[numMovableObjects-1].x;
			movableObjects[i].symbol = movableObjects[numMovableObjects-1].symbol;
			movableObjects[i].ignoreGravity = movableObjects[numMovableObjects-1].ignoreGravity;
			
			toBeRemoved[i] = toBeRemoved[numMovableObjects-1];
			numMovableObjects--;
			//toBeRemoved = realloc(toBeRemoved, numMovableObjects * sizeof(bool));
			movableObjects = realloc(movableObjects, numMovableObjects * sizeof(struct Object));
		}
		else
			i++;
	}
	return (originalSize != numMovableObjects);
}

bool doneSpell(){
	int y, x, i;
	bool somethingMoving = false;
	bool moved = false;
	bool blueDone=false, redDone=false, greenDone=false;
	struct Object *object = NULL;	
	
	for(y = boardHeight - 1; y >= 0; y--){
		for(x = 0; x < boardWidth; x++){
			moved = false;
			if((getElement(y, x) == 'V' && blueMagnetActivated && !blueDone)
			|| (getElement(y, x) == 'A' && redMagnetActivated && !redDone)
			|| (getElement(y, x) == 'O' && greenMagnetActivated && !greenDone))
				for(i = y + 1; i < boardHeight; i++){
					// Box and Bludeger cannot pass through rock, magnet, brick and broom
					if(getElement(i, x) == '#' || getElement(i, x) == 'D' || getElement(i, x) == '-'
					|| getElement(i, x) == 'V' || getElement(i, x) == 'A' || getElement(i, x) == 'O')
						break;
						
					object = getObject(i, x);
					if(object != NULL){
						if(object->symbol == 'P')
							break;
						
						moved = moveObject(object, -1, 0);
						if(moved || i == y+1)
							object->ignoreGravity = true;
						if(getElement(y, x) == 'V') blueDone = true;
						if(getElement(y, x) == 'A') redDone = true;
						if(getElement(y, x) == 'O') greenDone = true;
						break;
					}
				}
				
			somethingMoving |= moved;
		}
	}
	return !somethingMoving;
}

bool doneRolling(){
	int y, x, i;
	struct Object *object = NULL;
	struct Object *objectBelow = NULL;
	int previousPositions[numMovableObjects-1];
	int numRollingbludgers = 0;
	bool somethingMoving = false;
	bool moved = false;
	bool down, left, right;
	
	for(i = 0; i < numMovableObjects-1; i++)
		previousPositions[i] = -1;
		
	for(y = boardHeight - 1; y >= 0; y--){
		for(x = 0; x < boardWidth; x++){
			moved = false;
			object = getObject(y, x);
			if(object != NULL){
				if(object->symbol == 'P')
					continue;
				if(object->ignoreGravity)
					continue;
					
				down = true;
				left = true;
				right = true;
				for(i = 0; i < numRollingbludgers; i++){
					if(previousPositions[i] == ((y+1) * boardHeight + x))
						down = false;
					if(previousPositions[i] == ((y+1) * boardHeight + x-1))
						left = false;
					if(previousPositions[i] == ((y+1) * boardHeight + x+1))
						right = false;
				}
				// fall downward
				if(down)
					moved = moveObject(object, 1, 0);
					
				// roll if not falling
				if(!moved && objectIsAbludger(object)){
					objectBelow = getObject(y+1, x);
					if(objectBelow != NULL){
						if(objectIsAbludger(objectBelow)){
							if(left && getObject(y, x-1) == NULL && getObject(y+1, x-1) == NULL)
								if((getElement(y+1, x-1)==' ' || getElement(y+1, x-1)=='H' || getElement(y+1, x-1)=='v' || getElement(y, x-1)=='a' || getElement(y, x-1)=='o')
								&& (getElement(y, x-1)==' ' || getElement(y, x-1)=='H' || getElement(y, x-1)=='v' || getElement(y, x-1)=='a' || getElement(y, x-1)=='o'))
									moved = moveObject(object, 1, -1);
							
							if(!moved){
								if(right && getObject(y, x+1) == NULL && getObject(y+1, x+1) == NULL)
								if((getElement(y+1, x+1)==' ' || getElement(y+1, x+1)=='H' || getElement(y+1, x+1)=='v' || getElement(y, x+1)=='a' || getElement(y, x+1)=='o')
								&& (getElement(y, x+1)==' ' || getElement(y, x+1)=='H' || getElement(y, x+1)=='v' || getElement(y, x+1)=='a' || getElement(y, x+1)=='o'))
										moved = moveObject(object, 1, 1);
							}
						}
					}
				}
			}
			if(moved){
				previousPositions[numRollingbludgers] = y * boardHeight + x;
				numRollingbludgers++;
			}
			somethingMoving = somethingMoving | moved;
		}
	}
	return !somethingMoving;
}

bool doneGravity(){
	bool somethingMoving = false;
	int y, x;
	struct Object *object = NULL;
	
	for(y = boardHeight - 1; y >= 0; y--){
		for(x = 0; x < boardWidth; x++){
			object = getObject(y, x);
			if(object != NULL){
				if(object->symbol == 'P'){
					if(!object->ignoreGravity && getElement(y, x) != 'H' && getElement(y+1, x) != 'H')
						somethingMoving = somethingMoving | moveObject(object, 1, 0);
				}
				else{
					if(object->ignoreGravity == false)
						somethingMoving = somethingMoving | moveObject(object, 1, 0);
				}
			}
		}
	}
	return !somethingMoving;
}

void print(){
	printBoard();
	printObjects();
	refresh();
}

int getInput(){
	char c;
	int i;
	if(controlSequenceSize == 0)
		return getch();
	//return getch();
	else{
		c = controlSequence[0];
		for(i = 0; i < controlSequenceSize - 1 ; i++)
			controlSequence[i] = controlSequence[i+1];
		controlSequence[controlSequenceSize-1] = 0;
		controlSequenceSize--;
		controlSequence = realloc(controlSequence, controlSequenceSize);
		return c;
	}
}

// return true to advance, false to restart level
bool runLevel(){
	bool stable;
	controlSequenceSize = 0;
	blueMagnetActivated = false;
	redMagnetActivated = false;
	greenMagnetActivated = false;
	gameOver = false;
	print();
	displayMessage("      ");
	while(true){
		if(input == 'q'){
			endwin();
			exit(0);
		}
		if(numMovableObjects <= 1){
			displayMessage("you win!");
			input = getch();
			if(input == 'r')
				return false;
			return true;
		}
		if(gameOver){
			displayMessage("game over");
			input = getch();
			if(input == 'r')
				return false;
			endwin();
			exit(0);
		}		
		
		input = getInput();
		stable = false;
		if(controlSequenceSize > 0)
			usleep(300000);
			
		if(input == 'r')
			return false;
		handleInput();
		print();
		
		aboveHarry = getObject(getPlayer()->y-1, getPlayer()->x);
		
if(controlSequenceSize >0)displayMessage(controlSequence);refresh();

		while(!stable && !gameOver){
			while(doneSpell() == false && !gameOver){
				usleep(300000);
				print();
			}
			
			while(doneGravity() == false && !gameOver){
				usleep(300000);
				print();
			}
			
			while(doneRolling() == false && !gameOver){
				usleep(300000);
				print();
			}
			
			if(removeBoxesAndbludgers() == false && !gameOver)
				stable = true;
			print();
		}
	}
	free(movableObjects);
	free(controlSequence);
	free(board);
	return true;
}

void runGame(int argumentLength){
	int level;
	char *fileName;
	bool advanceLevel;
	fileName = malloc(12 + argumentLength);
	for(level = 1; level <= numLevels; level++){
		memset(fileName, 0, 12 + argumentLength);
		sprintf(fileName, "%s%d%s", "maps/map", level, ".txt");
		readMapFile(fileName);
		advanceLevel = runLevel();
		if(advanceLevel == false){
			level--;
		}
		clearBackGround();
	}
	free(fileName);
}

bool argumentIsValid(const int numArguments, char **const argument){
	int argumentLength, i;
	char *fileName;
	
	if(numArguments != 2){
		printf("ERROR: number of arguments is not 1\n");
		return false;
	}
	
	argumentLength = strlen(argument[1]);
	for(i = 0; i < argumentLength; i++)
		if(argument[1][i] < '0' || argument[1][i] > '9'){
			printf("ERROR: argument is not a number\n");
			return false;
		}
	numLevels = atoi(argument[1]);
	
	// fileName = "maps/mapN.txt"
	fileName = malloc(12 + argumentLength);
	for(i = 1; i <= numLevels; i++){
		memset(fileName, 0, 12 + argumentLength);
		sprintf(fileName, "%s%d%s", "maps/map", i, ".txt");
		if(!fileIsValid(fileName)){
			printf("ERROR: file \"%s\" does not exist or is not readable\n", fileName);
			return false;
		}
	}
	
	free(fileName);
	return true;
}

int main(int argc, char **const argv){
	if(!argumentIsValid(argc, argv))
		return -1;
	initscr();
	initializeNcurses();
	runGame(strlen(argv[1]));
	endwin();
	return 0;
}
