#include "player.h"
#include "ball.h"
#include "screen.h"
#include <locale.h>
//#include <ncursesw/curses.h>

void setUp();
void wrapUp();

struct BLOCK block[50];

int done;
int score;

int  main()
{	
	int i=0;
	// 기초 설정
	void sigOn();
	void setBuffer();
	void inputKey(int);

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
	ball.y_dir = 1;
	ball.x_dir = 1;
	ball.symbol = BALL_SYMBOL;
	ball.life = 3;
    
	player.x = LEFT_EDGE;
	player.y = BOT_ROW;
	player.symbol = PL_SYMBOL;
	drawPlayer();
	done = 0;

	drawFrame("frame");
	drawStage("stage3");
	drawScore(score);

	signal(SIGINT, SIG_IGN);
	mvaddch(ball.y, ball.x, ball.symbol);
	refresh();

	signal(SIGIO, inputKey);
	setBuffer();
	aio_read(&kbcbuf);
	set_ticker(1000 / TICKS_PER_SEC);
}

// 게임 종료 행동 모음.
void wrapUp()
{
	set_ticker(0);
	endwin();
}

