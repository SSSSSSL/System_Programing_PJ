#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <locale.h>
#include <ncursesw/curses.h>
#include "bounce.h"
#include <aio.h>
#include <signal.h>
#include <time.h>

#define SPACE 32
#define LEFT 68
#define RIGHT 67

#define STR(i, j, m) {move(i, j); addstr(m);}
#define MAXNUM(a, b) ((a) > (b)) ? (a) : (b)
#define MINNUM(a, b) ((a) < (b)) ? (a) : (b)


typedef struct _BLOCK
{
    int  nLife;
    int  nX, nY;
} BLOCK;


void  set_up();
void  wrap_up();
int count = 0;

void  set_up()  {
    void ball_move(int);
    void enable_kdb_signals();
    void setup_aio_buffer();
    void on_input(int);

    // setup the ball's state
    the_ball.y_pos = Y_INIT;        
    the_ball.x_pos = X_INIT;
    the_ball.y_ttg = the_ball.y_ttm = Y_TTM;
    the_ball.x_ttg = the_ball.x_ttm = X_TTM;
    the_ball.y_dir = 1;
    the_ball.x_dir = 1;
    the_ball.symbol = DFL_SYMBOL;

    done = 0;

//    initscr();                    // setup the screen
    noecho();
    crmode();
    clear();
    
    // setup the player's state and draw the player
    the_player.x_pos = LEFT_EDGE;
    the_player.y_pos = BOT_ROW-1;
    the_player.symbol = PL_SYMBOL;
    draw_player();

    // draw the wall
    draw_wall();

    // setup and draw the score
    score = 0;
    draw_score();

    signal(SIGINT,SIG_IGN);
    mvaddch(the_ball.y_pos,the_ball.x_pos,the_ball.symbol);
    refresh();

    signal(SIGIO,on_input);                // install a handler
    
    // use O_ASYNC
//    enable_kdb_signals();                // turn on kbd signals
    
    // use aio_read
    setup_aio_buffer();                // initialize aio ctrl buff
    aio_read(&kbcbuf);                // place a read quest

    signal(SIGALRM,ball_move);            // install alarm handler
    set_ticker(1000/TICKS_PER_SEC);            // send millisecs per tick
}

void  wrap_up()  {
    set_ticker(0);
    endwin();                    // put back normal
}

void  ball_move( int  signum)  {
    int y_cur,x_cur,moved;
    int bounce_or_lose(struct ppball *);

    signal(SIGALRM,ball_move);            // don't get caught now
    y_cur = the_ball.y_pos;                // old spot
    x_cur = the_ball.x_pos;
    moved = 0;
    count++;
	
    if(the_ball.y_ttm > 0 && --the_ball.y_ttg == 0) {
        the_ball.y_pos += the_ball.y_dir;    // move
        the_ball.y_ttg = the_ball.y_ttm;    // reset
        moved = 1;
    }

    if(the_ball.x_ttm > 0 && --the_ball.x_ttg == 0) {
        the_ball.x_pos += the_ball.x_dir;    // move
        the_ball.x_ttg = the_ball.x_ttm;    // reset
        moved = 1;
    }

    if(moved) {
        mvaddch(y_cur,x_cur,BLANK);
        // mvaddch(y_cur,x_cur,BLANK);
        mvaddch(the_ball.y_pos,the_ball.x_pos,the_ball.symbol);
        if(bounce_or_lose(&the_ball)) {
            ++score;
            draw_score();
		printFrame("frame");
            //draw_wall();
        }
        move(LINES - 1,COLS - 1);
        refresh();
    }
    signal(SIGALRM,ball_move);            //  for unreliable systems
}

int  bounce_or_lose( struct  ppball  * bp)  {
    int return_val = 0;

    if(bp -> y_pos <= TOP_ROW ) {
        bp -> y_dir = 1;
    } else if(bp -> y_pos >= BOT_ROW) {
        bp -> y_dir = -1;
    }

    if(bp -> x_pos <= LEFT_EDGE) {
        bp -> x_dir = 1;
    } else if(bp -> x_pos >= RIGHT_EDGE) {
        bp -> x_dir = -1;
    }

    if(bp -> y_pos >= BOT_ROW-2 && bp -> x_pos == the_player.x_pos)
	    bp -> y_dir = -1;
    if(bp -> y_pos >= BOT_ROW-2 && bp -> x_pos == the_player.x_pos+1)
            bp -> y_dir = -1;
    if(bp -> y_pos >= BOT_ROW-2 && bp -> x_pos == the_player.x_pos+2)
            bp -> y_dir = -1;
    if(bp -> y_pos >= BOT_ROW-2 && bp -> x_pos == the_player.x_pos+3)
            bp -> y_dir = -1;
    if(bp -> y_pos >= BOT_ROW-2 && bp -> x_pos == the_player.x_pos+4)
            bp -> y_dir = -1;
   

    /*if(bp -> x_pos == the_player.x_pos 
        && bp -> y_pos == the_player.y_pos)
        return_val = 1;*/

    return return_val;
}

/* 
 * intstall a handler, tell kernel who to notify on input, enable signals
 */
void  enable_kdb_signals()  {
    int fd_flags;

    fcntl(0,F_SETOWN,getpid());
    fd_flags = fcntl(0,F_GETFL);
    fcntl(0,F_SETFL,(fd_flags | O_ASYNC));
}

/*
 * handler called to when aio_read() has stuff to read
 * First check for any errors codes, and if ok, then get the return code
 */
void  on_input( int  signum)  {
    int c;

    while((c = getchar()) != 'Q') {
        if(c == 'f') --the_ball.x_ttm;
        else if(c == 's') ++the_ball.x_ttm;
        else if(c == 'F') --the_ball.y_ttm;
        else if(c == 'S') ++the_ball.y_ttm;
        else if(c == 'z') 
            the_ball.x_dir = -the_ball.x_dir;
        else if(c == 'x')
            the_ball.y_dir = -the_ball.y_dir;
        else if(c == 'i' || c == 'j' || c == 'k' || c == 'l')
            move_player(c);
    }

    done = 1;
}

/*
 * set memebers of struct
 * First specify args like those for read(fd,buf,num) and offset
 * Then specify what to do(send signal) and what signal(SIGIO)
 */
void  setup_aio_buffer()  {
    static char input[1];                // 1 char of input

    // describe what to read
    kbcbuf.aio_fildes = 0;                // standard input
    kbcbuf.aio_buf = input;                // buffer
    kbcbuf.aio_nbytes = 1;                // number to read
    kbcbuf.aio_offset = 0;                // offset in file

    // describe what to do when read is ready
    kbcbuf.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    kbcbuf.aio_sigevent.sigev_signo = SIGIO;    // send SIGIO
}

/*
 * draw the wall when setup.
 */
void  draw_wall()  {
    int i,j;

	printFrame("frame");
    // draw four corner
/*	STR(TOP_ROW - 1,LEFT_EDGE - 1,"┏");
	STR(BOT_ROW + 1,LEFT_EDGE - 1,"┗");
	STR(TOP_ROW - 1,RIGHT_EDGE + 1,"┓");
	STR(BOT_ROW + 1,RIGHT_EDGE + 1,"┛");

    //mvaddch(TOP_ROW - 1,LEFT_EDGE - 1,"┏");
    //mvaddch(BOT_ROW + 1,LEFT_EDGE - 1,'┗');
    //mvaddch(TOP_ROW - 1,RIGHT_EDGE + 1,'┓');
    //mvaddch(BOT_ROW + 1,RIGHT_EDGE + 1,'┛');

    // draw top row
    for(i = LEFT_EDGE,j = TOP_ROW - 1; i <= RIGHT_EDGE; ++i) {
       STR(j,i,"━");
    }

    // draw left column
    for(i = LEFT_EDGE - 1,j = TOP_ROW; j <= BOT_ROW; ++j) {
        STR(j,i,"┃");
    }

    // draw botton row
    for(i = LEFT_EDGE,j = BOT_ROW + 1; i <= RIGHT_EDGE; ++i) {
        STR(j,i,"━");
    }

    // draw right column
    for(i = RIGHT_EDGE + 1,j = TOP_ROW; j <= BOT_ROW; ++j) {
        STR(j,i,"┃");
    }*/
}

/*
 * draw score on the left top corner
 */
void  draw_score()  {
    char score_string[100];
    sprintf(score_string,"score:%d",score);
    move(30,1);
    addstr(score_string);
}

/*
 * draw the player
 */
void  draw_player()  {
    mvaddch(the_player.y_pos,the_player.x_pos,the_player.symbol);	
    mvaddch(the_player.y_pos,the_player.x_pos+1,the_player.symbol);
    mvaddch(the_player.y_pos,the_player.x_pos+2,the_player.symbol);
    mvaddch(the_player.y_pos,the_player.x_pos+3,the_player.symbol);
    mvaddch(the_player.y_pos,the_player.x_pos+4,the_player.symbol);
}

/*
 * move the player
 */
void  move_player( char  input)  {

    switch(input) {
        /*case 'i':
            if(the_player.y_pos > TOP_ROW)
                --the_player.y_pos;
            break;
        case 'k':
            if(the_player.y_pos < BOT_ROW)
                ++the_player.y_pos;
            break;*/
        case 'j':
	    mvaddch(the_player.y_pos,the_player.x_pos+4,BLANK);
            if(the_player.x_pos > LEFT_EDGE)
                --the_player.x_pos;
            break;
        case 'l':
	    mvaddch(the_player.y_pos,the_player.x_pos,BLANK);
            if(the_player.x_pos+4 < RIGHT_EDGE)
                ++the_player.x_pos;
            break;
    }
    draw_player();
}



void printFrame(char*);
void printTitle(int);
void printCredit();
void printStage(char*);

int screenTitle();
int screenCredit();
int screenStage(char*);

int makeBlock(int x, int y, char[]);

FILE *f;
BLOCK g_sBlock[45];


void main()
{
	int room = 3;

	setlocale(LC_CTYPE, "ko_KR.utf-8");
	initscr();
	clear();
	
	while (room > 0)
	{
		if (room == 3) room = screenTitle();
		if (room == 2) room = screenStage("stage3");
		if (room == 1) room = screenCredit();
	}
	endwin();
}


void printFrame(char *name)
{
	char buffer[3];
	int rowA, colA, rowB, colB, i;

	f = fopen(name, "r");
	
	while (!feof(f))
	{
		fscanf(f, "%d %d", &rowA, &colA);
		fscanf(f, "%d %d", &rowB, &colB);
		fscanf(f, "%s", buffer);

		for (rowA; rowA <= rowB; rowA++)
		{
			for (i = colA; i <= colB; i++)
				STR(rowA, i, buffer);
		}
	}
	fclose(f);
}


void printTitle(int i)
{
	STR(5, 5, "벽돌 깨기 타이틀");
	STR(20, 3, "v. 시스템 프로그래밍");
	STR(12, 6, "◁");
	STR(12, 18, "▷");

	switch(i)
	{
		case 0: STR(12, 9, "게임 종료"); break;
		case 1: STR(12, 10, "크레딧"); break;
		case 2: STR(12, 9, "게임 시작"); break;
	}
}


void printCredit()
{
	STR(5, 5, "시스템 프로그래밍");
	STR(9,4, "2018112100  홍길동");
	STR(11, 4, "2018112101  박철수");
	STR(13, 4, "2018112102  안영희");
	STR(15, 4, "2018112103  김민수");
	STR(20, 5, "<SPACE> 돌아가기");
}


void printStage(char *name)
{
	int rowA, colA, rowB, colB, i;
	char buffer[3];

	f = fopen(name, "r");

	while (!feof(f))
	{
		fscanf(f, "%d %d", &rowA, &colA);
		fscanf(f, "%d %d", &rowB, &colB);
		fscanf(f, "%s", buffer);
		colA = colA * 2 + 1;		
		colB = colB * 2 + 1;

		for (rowA; rowA <= rowB; rowA++)
		{
			for (i = colA; i <= colB; i += 2)
				makeBlock(rowA, i, buffer);
		}
	}
	fclose(f);
}


int screenTitle()
{
	int key, i;
	key = 0;	
	i = 2;

	while (key != SPACE)
	{
		printFrame("frame");
		printTitle(i);
		refresh();

		fflush(stdin);
		move (LINES-1, 0);
		key = getch();
		if (key == LEFT) i = MINNUM(i+1, 2);
		if (key == RIGHT) i = MAXNUM(i-1, 0);
		clear();
	}

	return i;
}


int screenCredit()
{
	int key;

	while (key != SPACE)
	{
		printFrame("frame");
		printCredit();
		refresh();

		fflush(stdin);
		move (LINES-1, 0);
		key = getch();
		clear();
	}

	return 3;
}


int makeBlock(int x, int y, char buffer[])
{
	int i = 0;

	g_sBlock[i].nLife = 1;
	g_sBlock[i].nX = x;
	g_sBlock[i].nY = y;

	if (g_sBlock[i].nLife)
	{
		STR(g_sBlock[i].nX, g_sBlock[i].nY, buffer);
		refresh();
		i++;
	}
}


int screenStage(char* name)
{
	int key;

	while (key != SPACE)
	{
		
		refresh();
		
		set_up();
		clear();

	    	while(!done){    // the main loop
			printFrame("frame");
			printStage(name);        		pause();
			if(count > 10*50)
		break;	
    		}
    
		wrap_up();

		return 0;		

		fflush(stdin);
		move (LINES-1, 0);
		key = getch();
		clear();
	}

	return 3;
}
