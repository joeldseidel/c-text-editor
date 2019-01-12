#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

#define CTRL_KEY(k) ((k) & 0x1f)

struct editorConfig {
	//Screen dimensions
	int screenrows;
	int screencols;
	//Original terminal flags to revert to
	struct termios orig_termios;
};

struct editorConfig E;

/**
 * Print the error message on death
**/
void die(const char *s){
	//Display error message that caused the death
	perror(s);
	//Clear screen before exit
	//Clear the terminal
	write(STDOUT_FILENO, "\x1b[2J", 4);
	//Reset the cursor
	write(STDOUT_FILENO, "\x1b[H", 3);
	//Die 'gracefully'
	exit(1);
}

/**
 * Toggle the terminal echo flag back on at the end
**/
void disableRawMode(){
	//Reset the terminal flags
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1){
		//Error occurred, handle in die method
		die("tcsetattr");
	}
}

/**
 * Toggle the terminal echo flag to off
**/
void enableRawMode(){
	//Get the current terminal and set to struct
	if(tcgetattr(STDIN_FILENO, &E.orig_termios) == -1){
		//Error occurred, handle in die method
		die("tcgetattr");
	}
	//Set raw mode off toggle to run at exit
	atexit(disableRawMode);
	struct termios raw = E.orig_termios;
	//Flip the ctrl-s/q flag
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	//Flip output processing flag
	raw.c_iflag &= ~(OPOST);
	raw.c_iflag &= ~(CS8);
	//Flip the echo flag and canonical mode and ctrl-c/z/v flag
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	//Set read time out
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	//Set the new flag to the terminal
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1){
		//Error occurred, handle in die method
		die("tcsetattr");
	}
}

/**
 * Read the key pressed into a character
**/
char editorReadKey(){
	int nread;
	char c;
	//Get the character
	while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if (nread == -1 && errno != EAGAIN){
			//Error occurred in the read
			die("read");
		}
	}
	return c;
}

/**
 * Get current size of the terminal window
**/
int getWindowSize(int *rows, int *cols){
	struct winsize ws;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
		return -1;
	} else {
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
}

/**
 * Draw rows of chars to left of screen
**/
void editorDrawRows(){
	int y;
	for(y = 0; y < E.screenrows; y++){
		//Write tildes to screen
		write(STDOUT_FILENO, "~\r\n", 3);
	}
}

/**
 * Refresh the screen and clear terminal
**/
void editorRefreshScreen(){
	//Clear the terminal
	write(STDOUT_FILENO, "\x1b[2J", 4);
	//Reposition to cursor
	write(STDOUT_FILENO, "\x1b[H", 3);
	editorDrawRows();
	write(STDOUT_FILENO, "\x1b[H", 3);
}


/**
 * Get the key press and validate if it should do anything
**/
void editProcessKeypress(){
	//Get entered character
	char c = editorReadKey();
	switch(c){
		case CTRL_KEY('q'):
			//Entered character was ctrl q, exit the application
			//Refresh the screen before exit
			//Clear the terminal
			write(STDOUT_FILENO, "\x1b[2J", 4);
			//Reset the cursor
			write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;
	}
}

void initEditor(){
	if(getWindowSize(&E.screenrows, &E.screencols) == -1){
		die("getWindowSize");
	}
}

int main(){
	enableRawMode();
	initEditor();
	//Input loop
	while(1) {
		editorRefreshScreen();
		editProcessKeypress();
	}
	return 0;
}