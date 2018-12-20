#include <string.h>
#include <locale.h>
#include <aio.h>
#include <curses.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ncursesw/curses.h>

#define MAXNUM(a, b) ((a) > (b)) ? (a) : (b)
#define MINNUM(a, b) ((a) < (b)) ? (a) : (b)

#define WORD(a) a+48
#define NUMBER(a) a-48
#define TRUE 1
#define FALSE 0
//볼 define=============================================================

#define ball_h

#define X_INIT 5
#define Y_INIT 13

#define TOP_ROW 1
#define BOT_ROW 20

#define BALL_SYMBOL "o"
#define BALL_BLANK " "

//플레이어 define====================================================

#define player_h

#define LEFT_EDGE 1
#define RIGHT_EDGE 24

#define PL_SYMBOL '*'
#define  BLANK ' '

//스크린 define=======================================================
#define screen_h

#define STR(i, j, m) {move(i, j); addstr(m);}

typedef struct BALL
{
	int y, x, y_dir, x_dir, life;
        char symbol[2];
} BALL;


typedef struct PLAYER
{
	int y, x;
	char symbol;
} PLAYER;

struct BLOCK
{
	int nLife;
	int x1;
	int x2;
	int y;
	char symbol[3];
};

struct BALL ball;
struct PLAYER player;
struct aiocb kbcbuf;
struct BLOCK block[6][30];

int TICKS_PER_SEC = 10;

// 기초 설정
void sigOn();
void setBuffer();
void inputKey(int);

void setUp();
void wrapUp();

void ballMove(int signum);
int bounceBall(struct BALL *bp);

void movePlayer(char);
void drawPlayer();

void printCredit();

int stagenum = 1;

int set_ticker(int);

int score = 0;
int count = 0;
int done = 0;
int total_score=0;

FILE *f;
void drawFrame(char *name);

void ctrlTitle();
void drawTitle();

void drawStage(int stageNum);
int makeBlock(int x, int y, char buffer[]);

void drawScore(int num);
void drawLife(int num);
void drawClear();
void drawBlank();

void set_color();

int  main()
{	
	int i=0;
	
	setlocale(LC_CTYPE, "ko_KR.utf-8");
	initscr();
	noecho();
	crmode();
	clear();

	// 타이틀
	drawFrame("frame");
	ctrlTitle();
	clear();

	// 게임 시작
	setUp();

	signal(SIGALRM, ballMove);
	while (!done)
	{
		pause();
	}

	// 마무리
	wrapUp();

	return 0;
}


// 게임 초기 설정
void setUp()
{
	void ballMove(int);
	ball.y = Y_INIT;
	ball.x = X_INIT;
	ball.y_dir = -1;
	ball.x_dir = 1;
	strcpy(ball.symbol, BALL_SYMBOL);
	ball.life = 3;
    
	player.x = LEFT_EDGE;
	player.y = BOT_ROW;
	player.symbol = PL_SYMBOL;
	drawPlayer();

	drawFrame("frame");
	drawStage(stagenum);
	drawScore(score);

	signal(SIGINT, SIG_IGN);
	mvaddstr(ball.y, ball.x, ball.symbol);
	refresh();
	signal(SIGIO, inputKey);
	setBuffer();
	aio_read(&kbcbuf);
	
	set_ticker(1000 / TICKS_PER_SEC);
}


// 게임 종료 설정
void wrapUp()
{
	set_ticker(0);
	endwin();
}


void printCredit()
{
	static char t_score[5];
	sprintf(t_score, "%d", total_score);

	STR(5, 5, "시스템 프로그래밍");
	STR(7, 5, "Total Score : ");
	mvaddstr(7, 19, t_score);
	STR(9,4, "2014018036  황정인");
	STR(11, 4, "2015110533  정원웅");
	STR(13, 4, "2015113262  이승수");
	STR(15, 4, "2017113020  박효빈");
}

//공 함수========================================================================

// 공 움직이기
void ballMove(int signum)
{
	char clear[50] = "";
	int yy, xx;
	int bounceBall(struct BALL *);
	int collision = 0;
	static char n_clear[5];

	yy = ball.y;
	xx = ball.x;
	
	collision = bounceBall(&ball);

	if (collision == 2){
		mvaddch(yy, xx-1, BLANK);}
	else{
		mvaddch(yy, xx, BLANK);
	}
	
	if (yy != 0)
	mvaddstr(yy, xx, BALL_BLANK);

	ball.y += ball.y_dir;
	ball.x += ball.x_dir;
	mvaddstr(ball.y, ball.x, ball.symbol);
	
	if (score > 19) {
		total_score += score;
		stagenum++;
		switch(stagenum){
			case 1:
			case 2:
				TICKS_PER_SEC=10;
				break;
			case 3:
			case 4:
				TICKS_PER_SEC=12;
				break;
			case 5:
				TICKS_PER_SEC=14;
				break;
		}

		if(stagenum>5){
			clear();
			drawFrame("frame");
			printCredit();
			set_ticker(0);
			refresh();
			sleep(3);	
			endwin();
			exit(0);
		}
		set_ticker(0);		
		score=0;
		drawClear();
		move(LINES-1,0);
		refresh();
		sleep(2);
		drawBlank();
		sprintf(n_clear, "%d", stagenum);
		STR(12,9, "Stage");
		STR(12,15, n_clear);
		move(LINES-1,0);
		refresh();
		sleep(2);
		clear();
		setUp();
		
	}

	if(ball.life <= 0){
		set_ticker(0);
		clear();
		drawFrame("frame");
		mvaddstr(12, 7, "Game Over");
		move(LINES-1,0);
		refresh();
		sleep(2);
		wrapUp();
		exit(0);
	}

	drawScore(score);
	drawLife(ball.life);

	count++;
	move(LINES-1, 0);

	refresh();
}


// 공 튕기기
int bounceBall(struct BALL *bp)
{
	int i;
	int val = 0;
		
	for (i = 0; i < 30; i++){
		if (block[stagenum][i].nLife)
			if ( bp->y == block[stagenum][i].y )
				if ( ( block[stagenum][i].x1 == bp->x ) ){
					block[stagenum][i].nLife = 0;
					score++;
					bp->y_dir = -(bp->y_dir);
					printf("\a");
					val = 1;
				}
				else if ( ( block[stagenum][i].x2 == bp->x ) ){
					block[stagenum][i].nLife = 0;
					score++;
					bp->y_dir = -(bp->y_dir);
					printf("\a");
					val = 2;
				}
	}
	if (bp->y <= TOP_ROW || bp->y > BOT_ROW)
		bp->y_dir = -(bp->y_dir);
	if (bp->x <= LEFT_EDGE || bp->x >= RIGHT_EDGE)
		bp->x_dir = -(bp->x_dir);

	for (i = 0; i < 5; i++)
	{
		if (bp->x == player.x+i && bp->y == player.y - 1)
			bp->y_dir = -(bp->y_dir);
	}

	if(bp->y == BOT_ROW){		
		if(--(bp->life) <= 0)
			done = 1;
		bp->x = X_INIT;
		bp->y = Y_INIT;
		ball.y_dir = -1;
		ball.x_dir = 1;
		sleep(1);
	}
	return val;
}

//플레이어 함수===========================================================================

// 시그널 활성화
void sigOn()
{
	int fd_flags;

	fcntl(0, F_SETOWN, getpid());
	fd_flags = fcntl(0, F_GETFL);
	fcntl(0, F_SETFL, (fd_flags | O_ASYNC));
}

void setBuffer()
{
	static char input[1];
	kbcbuf.aio_fildes = 0;		// standard input
	kbcbuf.aio_buf = input;		// buffer
	kbcbuf.aio_nbytes = 1;		// number to read
	kbcbuf.aio_offset = 0;		// offset in file

	// describe what to do when read is ready
	kbcbuf.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	kbcbuf.aio_sigevent.sigev_signo = SIGIO;	// send SIGIO
}


// 키 인식
void inputKey(int signum)
{
	int c;

	while ((c = getchar()) != 'Q')
	{
		if(c == 'j' || c == 'l')
			movePlayer(c);
	}
}


// 플레이어 이동
void movePlayer(char input)
{
	switch(input)
	{
		case 'j':
			mvaddch(player.y, player.x+4, BLANK);
			if (player.x > LEFT_EDGE) --player.x;
			break;
		case 'l':
			mvaddch(player.y, player.x, BLANK);
			if (player.x+4 < RIGHT_EDGE) ++player.x;
			break;
	}
	drawPlayer();
}


// 플레이어 그리기
void drawPlayer()
{
	int i;
	for (i = 0; i < 5; i++)
		mvaddch(player.y, player.x+i, '*');
}


//스크린 함수=====================================================================

// 틀 그리기
void drawFrame(char *name)
{
	char buffer[3];
	char temp = '\0';
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
				mvaddstr(rowA, i, buffer);
		}
	}
	fclose(f);
}


void drawClear()
{
	mvaddstr(10, 5, "***************");
	mvaddstr(12, 7, "C L E A R !");
	mvaddstr(14, 5, "***************");
}

void drawBlank()
{
	mvaddstr(10, 5, "               ");
	mvaddstr(12, 7, "               ");
	mvaddstr(14, 5, "               ");
}

// 타이틀 액션
void ctrlTitle()
{
	void drawTitle();
	char c = 0;

	drawFrame("frame");
	drawTitle();
	mvaddch(12, 16, WORD(stagenum));
	move (LINES-1, 0);
	refresh();

	while ((c = getchar()) != ' ')
	{
		if (c == 'j') stagenum = MAXNUM(1, stagenum-1);
		if (c == 'l') stagenum = MINNUM(5, stagenum+1);
		mvaddch(12, 16, WORD(stagenum));
		move (LINES-1, 0);
		refresh();
	}
}


// 타이틀 그리기
void drawTitle()
{
	set_color();
	
	mvaddstr(5, 6, "<<<벽돌 깨기>>>");
	
	attron(COLOR_PAIR(5));
	mvaddstr(10, 6, "스테이지  선택");
	mvaddstr(20, 3, "v. 시스템 프로그래밍");
	mvaddstr(12, 6, "◁");
	mvaddstr(12, 18, "▷");
	mvaddstr(12, 10, "Stage");

	attroff(COLOR_PAIR(5));
	attron(COLOR_PAIR(3));
	mvaddstr(14, 7, "Space로 시작");
	attroff(COLOR_PAIR(3));
}


// 스테이지 그리기
void drawStage(int stageNum)
{
	int rowA, colA, rowB, colB, i;
	char buffer[3];
	char stage[10];

	set_color();

	attron(COLOR_PAIR(1));

	stage[0]=stagenum+'0';
	stage[1]='\0';

	f = fopen(stage, "r");

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

	attroff(COLOR_PAIR(1));
}


// 블록 만들기
int makeBlock(int x, int y, char buffer[])
{
	static int i = 0;
	static int mem = 1;

	if(mem != stagenum){
		i = 0;
		mem = stagenum;
	}

	block[stagenum][i].nLife = 1;
	block[stagenum][i].x1 = y;
	block[stagenum][i].x2 = y + 1;
	block[stagenum][i].y = x;
	strcpy(block[stagenum][i].symbol, buffer);

	if (block[stagenum][i].nLife)
	{
		mvaddstr(block[stagenum][i].y, block[stagenum][i].x1, block[stagenum][i].symbol);
		refresh();
		i++;
	}
}


// 점수 그리기
void drawScore(int num)
{
	static char s_score[5];
	sprintf(s_score, "%d", num);
        set_color();
	attron(COLOR_PAIR(2));	
	mvaddstr(1, 27, "Score:");
	attroff(COLOR_PAIR(2));
	mvaddstr(1, 34, s_score);
}


// LIFE 그리기
void drawLife(int num)
{
	static char s_life[5];
	sprintf(s_life, "%d", num);
        set_color();
	attron(COLOR_PAIR(3));	
	mvaddstr(2, 27, "Life:");
	attroff(COLOR_PAIR(3));
	mvaddstr(2, 34, s_life);
}


// 색깔 넣기
void set_color()
{
	start_color();
	init_pair(1,COLOR_RED,COLOR_BLACK);
	init_pair(2,COLOR_GREEN,COLOR_BLACK);
	init_pair(3,COLOR_BLUE,COLOR_BLACK);
	init_pair(4,COLOR_WHITE,COLOR_BLACK);
	init_pair(5,COLOR_YELLOW,COLOR_BLACK);
}
