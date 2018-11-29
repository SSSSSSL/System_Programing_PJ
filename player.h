#ifndef player_h
#define player_h

#define LEFT_EDGE 1
#define RIGHT_EDGE 24

#define PL_SYMBOL '*'
#define  BLANK ' '

#include <aio.h>
#include <curses.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

typedef struct PL
{
	int y, x;
	char symbol;
} PL;

struct PL player;
struct aiocb kbcbuf;

void movePlayer(char);
void drawPlayer();


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

#endif
