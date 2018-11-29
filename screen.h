#ifndef screen_h
#define screen_h

#include <string.h>
#include <ncursesw/curses.h>

struct BLOCK
{
	int nLife;
	int x1;
	int x2;
	int y;
	char symbol[3];
};

struct BLOCK block[50];

int makeBlock(int x, int y, char[]);
void drawFrame(char*);
FILE *f;


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


// 타이틀 그리기
void drawTitle()
{
	mvaddstr(5, 5, "벽돌 깨기 타이틀");
	mvaddstr(20, 3, "v. 시스템 프로그래밍");
	mvaddstr(12, 3, "Space로 시작");
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
	mvaddstr(1, 27, "Score:");
	mvaddstr(1, 34, s_score);
}

// 점수 그리기
void drawLife(int num)
{
	static char s_life[5];
	sprintf(s_life, "%d", num); 
	mvaddstr(2, 27, "Life:");
	mvaddstr(2, 34, s_life);
}

#endif
