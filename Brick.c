#include <string.h>
#include <locale.h>
#include <aio.h>
#include <curses.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <ncursesw/curses.h>
#include <stdlib.h>

//볼 define=============================================================

#define ball_h

#define X_INIT 6
#define Y_INIT 11

#define TOP_ROW 2
#define BOT_ROW 20

#define BALL_SYMBOL "o"
#define BALL_BLANK " "
#define  TICKS_PER_SEC 7

//플레이어 define====================================================

#define player_h

#define LEFT_EDGE 1
#define RIGHT_EDGE 24

#define PL_SYMBOL '*'
#define  BLANK ' '

//스크린 define=======================================================
#define screen_h


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
struct BLOCK block[50];

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

int set_ticker(int);
int score = 0;
int count = 0;
int done = 0;

int makeBlock(int x, int y, char[]);
//void drawFrame(char*);
FILE *f;

void drawFrame(char *name);
void drawTitle();
void screenTitle();
void drawStage(char *name);
int makeBlock(int x, int y, char buffer[]);
void drawScore(int num);
void drawLife(int num);
void drawClear();
void drawOver();
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
	screenTitle();
	clear();

	// 게임 시작
	setUp();
	signal(SIGALRM, ballMove);
	while (!done)
	{
		pause();
		if (count > 10 * 50) break;
		drawFrame("frame");
	}

	// 마무리
	wrapUp();

	return 0;
}


// 게임 시작 행동 모음
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
	drawStage("stage1");
	drawScore(score);

	signal(SIGINT, SIG_IGN);
	mvaddstr(ball.y, ball.x, ball.symbol);
	refresh();

	signal(SIGIO, inputKey);
	setBuffer();
	aio_read(&kbcbuf);
	set_ticker(1000 / TICKS_PER_SEC);
}

// 게임 종료 행동
void wrapUp()
{
	set_ticker(0);
        move(LINES-1,0);
	refresh();
        sleep(3);
        clear();
        endwin();
        exit(0);
}

//공 함수========================================================================
// 공 움직이기
void ballMove(int signum)
{
	int yy, xx;
	int bounceBall(struct BALL *);
	int collision = 0;

	yy = ball.y;
	xx = ball.x;
	
	collision = bounceBall(&ball);

	if (collision == 2)
		mvaddch(yy, xx-1, BLANK);
	else
		mvaddch(yy, xx, BLANK);
	
	mvaddstr(yy, xx, BALL_BLANK);
	ball.y += ball.y_dir;
	ball.x += ball.x_dir;
	mvaddstr(ball.y, ball.x, ball.symbol);

	drawScore(score);
	drawLife(ball.life);

	if (score == 12) {
                drawClear();
                wrapUp();
        }

        if (ball.life == 0) {
                drawOver();
                wrapUp();
        }

	count++;
	move(LINES-1, 0);

	refresh();
}


// 공 튕기기
int bounceBall(struct BALL *bp)
{
	int i;
	int val = 0;
		
	for (i = 0; i < 50; i++){
		if (block[i].nLife)
			if ( bp->y == block[i].y )
				if ( ( block[i].x1 == bp->x ) ){
					block[i].nLife = 0;
					score++;
					bp->y_dir = -(bp->y_dir);
					val = 1;
				}
				else if ( ( block[i].x2 == bp->x ) ){
					block[i].nLife = 0;
					score++;
					bp->y_dir = -(bp->y_dir);
					val = 2;
				}
	}
	if (bp->y < TOP_ROW || bp->y > BOT_ROW)
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

void drawOver()
{
        mvaddstr(10, 5, "****************");
        mvaddstr(12, 5, "G A M E O V E R");
        mvaddstr(14, 5, "****************");
}



// 타이틀 그리기
void drawTitle()
{
	set_color();
	attron(COLOR_PAIR(3));
	mvaddstr(5, 5, "<<<벽돌 깨기>>>");
	attroff(COLOR_PAIR(3));
	attron(COLOR_PAIR(5));
	mvaddstr(20, 3, "v. 시스템 프로그래밍");
	attroff(COLOR_PAIR(5));
	mvaddstr(12, 7, "Space로 시작");
}


// 타이틀 액션 (메인 코드에 넣는게 더 적절하지만, 일단 여기에 구현 해둠.)
void screenTitle()
{
	int key;
	key = 0;

	while (key != 32)
	{
		drawTitle();
		refresh();

		fflush(stdin);
		move (LINES-1, 0);
		key = getch();
	}
}


// 스테이지 그리기
void drawStage(char *name)
{
	int rowA, colA, rowB, colB, i;
	char buffer[3];
	
	set_color();

	attron(COLOR_PAIR(1));
	
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
	
	attroff(COLOR_PAIR(1));
}


// 블록 만들기
int makeBlock(int x, int y, char buffer[])
{
	static int i = 0;

	block[i].nLife = 1;
	block[i].x1 = y;
	block[i].x2 = y + 1;
	block[i].y = x;
	strcpy(block[i].symbol, buffer);

	if (block[i].nLife)
	{
		mvaddstr(block[i].y, block[i].x1, block[i].symbol);
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
