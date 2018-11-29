#ifndef ball_h
#define ball_h

#define X_INIT 6
#define Y_INIT 11

#define TOP_ROW 1
#define BOT_ROW 20

#define BALL_SYMBOL '0'
#define  TICKS_PER_SEC 10

#include "player.h"
#include "screen.h"

typedef struct BALL
{
	int y, x, y_dir, x_dir, life;
        char symbol;
} BALL;

struct BALL ball;

int set_ticker(int);
int score = 0;
int count = 0;
int done  = 1;


// 공 움직이기
void ballMove(int signum)
{
	int yy, xx;
	int bounceBall(struct BALL *);

	yy = ball.y;
	xx = ball.x;

	if (bounceBall(&ball)) ++score;
	mvaddch(yy, xx, BLANK);
	ball.y += ball.y_dir;
	ball.x += ball.x_dir;
	mvaddch(ball.y, ball.x, ball.symbol);

	count++;
	move(LINES-1, 0);
	drawScore(score);
	drawLife(ball.life);
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
				if ( ( block[i].x1 <= bp->x ) && ( bp->x <= block[i].x2 ) ){
					block[i].nLife = 0;
					score++;
					bp->y_dir = -(bp->y_dir);
				}
	}
	if (bp->y <= TOP_ROW || bp->y >= BOT_ROW)
		bp->y_dir = -(bp->y_dir);
	if (bp->x <= LEFT_EDGE || bp->x >= RIGHT_EDGE)
		bp->x_dir = -(bp->x_dir);

	for (i = 0; i < 5; i++)
	{
		if (bp->x == player.x+i && bp->y == player.y - 1)
			bp->y_dir = -(bp->y_dir);
	}

	if(bp->y == BOT_ROW){		
		if(--bp->life <= 0)
			done = 1;
		bp->x = X_INIT;
		bp->y = Y_INIT;
		sleep(1);
	}
	return val;
}

#endif

