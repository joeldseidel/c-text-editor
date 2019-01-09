#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

//Original terminal flags to revert to
struct termios orig_termios;

/**
 * Print the error message on death
**/
void die(const char *s){
	//Display error message that caused the death
	perror(s);
	//Die 'gracefully'
	exit(1);
}

/**
 * Toggle the terminal echo flag back on at the end
**/
void disableRawMode(){
	//Reset the terminal flags
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1){
		//Error occurred, handle in die method
		die("tcsetattr");
	}
}

/**
 * Toggle the terminal echo flag to off
**/
void enableRawMode(){
	//Get the current terminal and set to struct
	if(tcgetattr(STDIN_FILENO, &orig_termios) == -1){
		//Error occurred, handle in die method
		die("tcgetattr");
	}
	//Set raw mode off toggle to run at exit
	atexit(disableRawMode);
	struct termios raw = orig_termios;
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

int main(){
	enableRawMode();
	//Input loop
	while(1) {
		char c = '\0';
		//Read keyboard input to character
		if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN){
			//Error occurred in reading, handle in die method
			die("read");
		}
		if(iscntrl(c)){
			//Print ctrl-c key value line
			printf("%d\r\n", c);
		} else {
			//Print entered key number and character line
			printf("%d ('%c')\r\n", c, c);
		}
		//Exit on q character
		if(c == 'q') break;
	}
	return 0;
}