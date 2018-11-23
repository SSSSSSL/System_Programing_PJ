#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <locale.h>
#include <ncursesw/curses.h>

#define SPACE 32
#define LEFT 68
#define RIGHT 67

#define STR(i, j, m) {move(i, j); addstr(m);}
#define MAXNUM(a, b) ((a) > (b)) ? (a) : (b)
#define MINNUM(a, b) ((a) < (b)) ? (a) : (b)

FILE *f;

void printFrame();
void printTitle(int);
int screenTitle();
int screenCredit();
void printCredit();


void main()
{
	int room = 3;

	setlocale(LC_CTYPE, "ko_KR.utf-8");
	initscr();
	clear();
	
	while (room > 0)
	{
		if (room == 3) room = screenTitle();
		if (room == 2) break;
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
		key = getch();
		clear();
	}

	return 3;
}
