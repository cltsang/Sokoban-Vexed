#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct Object{
	char symbol;
	int y;
	int x;
};

const int verticalStep = 2;
const int horizontalStep = 4;
int numLevels;
int input;
char markRemoveAt;
char *board;
int boardWidth, boardHeight;
struct Object *movableObjects;
int numMovableObjects;
char *controlSequence;
int controlSequenceSize;

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
	return board[y * boardWidth + x];
}

void printGrid(int y, int x, char symbol){
	int cursorX = cursorXToConsoleX(x);
	int cursorY = cursorYToConsoleY(y);
	switch(symbol){
	case 'P': // Human
		attron(COLOR_PAIR(1));
		//mvaddch(cursorY, cursorX, ' ');
		mvaddch(cursorY, cursorX+1, ACS_LEQUAL);
		mvaddch(cursorY, cursorX+2, ACS_GEQUAL);
		//mvaddch(cursorY, cursorX+3, ' ');
		//mvaddch(cursorY+1, cursorX, ' ');
		mvaddch(cursorY+1, cursorX+1, '/');
		mvaddch(cursorY+1, cursorX+2, '\\');
		//mvaddch(cursorY+1, cursorX+3, ' ');
		attroff(COLOR_PAIR(1));
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
	init_pair(1, COLOR_BLACK, COLOR_WHITE); // empty space, human, ladder, rock
	init_pair(2, COLOR_WHITE, COLOR_BLUE); // blue box
	init_pair(3, COLOR_WHITE, COLOR_RED); // red box
	init_pair(4, COLOR_WHITE, COLOR_CYAN); // cyan box
	init_pair(5, COLOR_WHITE, COLOR_MAGENTA); // magenta box
	init_pair(6, COLOR_WHITE, COLOR_GREEN); // green box
	init_pair(7, COLOR_WHITE, COLOR_YELLOW); // yellow box
	init_pair(8, COLOR_WHITE, COLOR_BLACK); // black box, background
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
			if(element == '#' || element == 'H' || element == ' '){
				board[row*boardWidth + column] = element;
			}
			else{
				board[row*boardWidth + column] = ' ';
				numMovableObjects++;
				movableObjects = realloc(movableObjects, numMovableObjects * sizeof(struct Object));
				movableObjects[numMovableObjects-1].symbol = element;
				movableObjects[numMovableObjects-1].y = row;
				movableObjects[numMovableObjects-1].x = column;
			}
		}
	}

	fclose(mapFile);
	return true;
}

void removeBoxes(){
	int i;
	if(markRemoveAt < 0)
		return;
	for(i = 0; i < numMovableObjects; ){
		if(movableObjects[i].y * boardWidth + movableObjects[i].x == markRemoveAt){
			movableObjects[i].y = movableObjects[numMovableObjects-1].y;
			movableObjects[i].x = movableObjects[numMovableObjects-1].x;
			movableObjects[i].symbol = movableObjects[numMovableObjects-1].symbol;
			numMovableObjects--;
			movableObjects = realloc(movableObjects, numMovableObjects * sizeof(struct Object));
		}
		else
			i++;
	}
	markRemoveAt = 0;
}

bool moveObject(struct Object *source, int velocityY, int velocityX){
	bool blocked;
	int targetY = source->y + velocityY;
	int targetX = source->x + velocityX;
	char element = getElement(targetY, targetX);
	struct Object *sink = getObject(targetY, targetX);
	
	if(sink != NULL){
		if(sink->symbol == source->symbol){ // boxes with same color
			blocked = false;
			markRemoveAt = sink->y * boardWidth + sink->x;
		}
		else{
			if(source->symbol == 'P')
				blocked = !moveObject(sink, velocityY, velocityX);
			else // boxex with different colors
				blocked = true;
		}
	}
	else{
		if(element == '#')
			blocked = true;
		else //if(element == ' ' || element == 'H')
			blocked = false;
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
			if(getElement(player->y, player->x) == 'H' || getElement(player->y-1, player->x) == 'H')
				moveObject(player, -1, 0);
			break;
		case KEY_DOWN: case 'd':
			if(getElement(player->y, player->x) == 'H' || getElement(player->y+1, player->x) == 'H')
				moveObject(player, 1, 0);
			break;
		case KEY_LEFT: case 'l':
				moveObject(player, 0, -1);
			break;
		case KEY_RIGHT: case '>':
				moveObject(player, 0, 1);
			break;
		default:
			break;
	}
}

bool stablized(){
	bool somethingFalling = false;
	int y, x;
	struct Object *object = NULL;
	for(y = boardHeight - 1; y >= 0; y--){
		for(x = 0; x < boardWidth; x++){
			object = getObject(y, x);
			if(object != NULL){
				if(object->symbol == 'P'){
					if(getElement(object->y, object->x) != 'H' && getElement(object->y+1, object->x) != 'H')
						somethingFalling = somethingFalling | moveObject(object, 1, 0);
				}
				else
					somethingFalling = somethingFalling | moveObject(object, 1, 0);
			}
		}
	}
	return !somethingFalling;
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
	print();
	markRemoveAt = 0;
	controlSequenceSize = 0;
//displayMessage("new game!!");
	while(true){
		if(input == 'q'){
			endwin();
			exit(0);
		}
		if(numMovableObjects <= 1){
			input = getch();
			if(input == 'r')
				return false;
			return true;
		}
		
		input = getInput();
		if(input == 'r')
			return false;
		handleInput();
if(controlSequenceSize >0)displayMessage(controlSequence);refresh();
		while(true){
			removeBoxes();
			print();
			if(controlSequenceSize > 0)
				usleep(300000);
			if(stablized())
				break;
			usleep(300000);
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