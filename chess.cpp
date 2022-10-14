#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <windows.h>

using namespace std;

#define set 1
#define reset -1

CRITICAL_SECTION cs, csT;
time_t times = 0, start = 0;
unsigned long long S = 0;

int T = 0, K = 0;
volatile int emptyCount = 0, N = 0, L = 0;

volatile int** Empty, ** Board;

void inBoard(int** tmpBoard, int x, int y, int flag) {
	if (x >= 0 && x < N && y >= 0 && y < N) {
		tmpBoard[x][y] += flag;
	}
}

void SetBoard(int** tmpBoard, int x, int y, int flag)
{
	tmpBoard[x][y] += flag;
	inBoard(tmpBoard, x - 1, y, flag);
	inBoard(tmpBoard, x + 1, y, flag);
	inBoard(tmpBoard, x, y - 1, flag);
	inBoard(tmpBoard, x, y + 1, flag);
	inBoard(tmpBoard, x - 1, y - 1, flag);
	inBoard(tmpBoard, x - 1, y + 1, flag);
	inBoard(tmpBoard, x + 1, y - 1, flag);
	inBoard(tmpBoard, x + 1, y + 1, flag);
}

unsigned long long findKing(int** persBoard, unsigned long long persS, int x, int y, int l)
{
	if (l == L)
	{
		persS++;
		return persS;
	}

	for (int i = x; i < N; i++) {
		for (int j = y; j < N; j++)
			if (persBoard[i][j] <= 0)
			{
				SetBoard(persBoard, i, j, set);
				persS = findKing(persBoard, persS, i, j + 1, l + 1);
				SetBoard(persBoard, i, j, reset);
			}
		y = 0;
	}

	return persS;
}

bool fla = 1;
DWORD WINAPI King(void* param)
{
	int id = (int)param;

	if (fla) {
		fla = 0;
		start = GetTickCount64();
	}
	int** persBoard = new int* [N];
	for (int i = 0; i < N; i++)
	{
		persBoard[i] = new int[N];
		for (int j = 0; j < N; j++)
			persBoard[i][j] = Board[i][j];
	}

	unsigned long long persS = 0;
	while (1) {
		//printf("%lld: #%d blocked\n", GetTickCount64() - start, id + 1);
		EnterCriticalSection(&cs);
		//printf("%lld: #%d ready\n", GetTickCount64() - start, id + 1);

		if (emptyCount == 0)
		{
			S += persS;
			LeaveCriticalSection(&cs);

			for (int i = 0; i < N; i++)
				delete[] persBoard[i];
			delete[] persBoard;

			break;
		}

		int x = Empty[emptyCount - 1][0];
		int y = Empty[emptyCount - 1][1];
		emptyCount--;

		LeaveCriticalSection(&cs);

		SetBoard(persBoard, x, y, set);
		persS = findKing(persBoard, persS, x, y + 1, 1);
		SetBoard(persBoard, x, y, reset);
	}

	return 0;
}

int main(int argc, char** argv)
{
	FILE* input = fopen("input.txt", "r"), * output, * timeFile;

	if (!input) {
		cout << "File error!" << endl;
		return -1;
	}

	fscanf(input, "%d%d%d%d", &T, &N, &L, &K);
	if (argc == 5) {
		T = atoi(argv[1]);
		N = atoi(argv[2]);
		L = atoi(argv[3]);
		K = atoi(argv[4]);
		printf("%d: %d %d %d\n", T, N, L, K);
	}

	Board = new volatile int* [N];
	for (int i = 0; i < N; i++) {
		Board[i] = new int[N];
		for (int j = 0; j < N; j++)
			Board[i][j] = 0;
	}

	emptyCount = N * N;
	for (int i = 0; i < K; i++) {
		int x = 0, y = 0;
		fscanf(input, "%d%d", &x, &y);
		if (Board[x][y] == 0) {
			Board[x][y] += set;
			emptyCount--;
		}
		if (x - 1 >= 0 && x - 1 < N && y >= 0 && y < N && Board[x - 1][y] == 0) {
			Board[x - 1][y] += set;
			emptyCount--;
		}
		if (x + 1 >= 0 && x + 1 < N && y >= 0 && y < N && Board[x + 1][y] == 0) {
			Board[x + 1][y] += set;
			emptyCount--;
		}
		if (x >= 0 && x < N && y - 1 >= 0 && y - 1 < N && Board[x][y - 1] == 0) {
			Board[x][y - 1] += set;
			emptyCount--;
		}
		if (x >= 0 && x < N && y + 1 >= 0 && y + 1 < N && Board[x][y + 1] == 0) {
			Board[x][y + 1] += set;
			emptyCount--;
		}
		if (x - 1 >= 0 && x - 1 < N && y - 1 >= 0 && y - 1 < N && Board[x - 1][y - 1] == 0) {
			Board[x - 1][y - 1] += set;
			emptyCount--;
		}
		if (x - 1 >= 0 && x - 1 < N && y + 1 >= 0 && y + 1 < N && Board[x - 1][y + 1] == 0) {
			Board[x - 1][y + 1] += set;
			emptyCount--;
		}
		if (x + 1 >= 0 && x + 1 < N && y - 1 >= 0 && y - 1 < N && Board[x + 1][y - 1] == 0) {
			Board[x + 1][y - 1] += set;
			emptyCount--;
		}
		if (x + 1 >= 0 && x + 1 < N && y + 1 >= 0 && y + 1 < N && Board[x + 1][y + 1] == 0) {
			Board[x + 1][y + 1] += set;
			emptyCount--;
		}
	}

	fclose(input);

	Empty = new volatile int* [emptyCount];
	for (int i = 0; i < emptyCount; i++)
		Empty[i] = new int[2];

	int k = 0;
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			if (Board[i][j] <= 0) {
				Empty[k][0] = i;
				Empty[k++][1] = j;
			}

	InitializeCriticalSection(&cs);
	InitializeCriticalSection(&csT);
	HANDLE* handles = new HANDLE[T];
	for (int i = 0; i < T; i++)
		handles[i] = CreateThread(0, 0, King, (void*)i, 0, 0);

	WaitForMultipleObjects(T, handles, TRUE, INFINITE);
	times = GetTickCount64() - start;

	for (int i = 0; i < T; i++)
		CloseHandle(handles[i]);

	DeleteCriticalSection(&cs);
	DeleteCriticalSection(&csT);

	for (int i = 0; i < N; i++)
		delete[] Board[i];
	delete[] Board;

	for (int i = 0; i < k; i++)
		delete[] Empty[i];
	delete[] Empty;

	output = fopen("output.txt", "w");
	fprintf(output, "%llu", S);
	fclose(output);

	timeFile = fopen("time.txt", "w");
	fprintf(timeFile, "%lld", times);
	fclose(timeFile);

	//printf("%.3f\n", (float)times / 1000);

	return 0;
}