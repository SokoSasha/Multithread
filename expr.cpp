#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <windows.h>
#include <time.h>
#include <math.h>

using namespace std;

volatile unsigned long long P = 0, i = 0, j = 0, N = 0, T = 0;
volatile long long** inWork;
CRITICAL_SECTION cs, tCs, csP;
time_t times = 0;
time_t start = 0;

struct Node {
	int n;
	int k;
	Node* next;
	Node* prev;
};

struct List {
	Node* head;
	Node* tail;
};

List* volatile NK;

//////////////////////////
List* volatile init();
void push(int n, int k, List* volatile list);
Node* getFront(List* volatile list);
Node* getBack(List* volatile list);
Node* popFront(List* volatile list);
Node* popBack(List* volatile list);
void printList(List* volatile list);
//////////////////////////

bool fla = 1;
DWORD WINAPI thread_entry(void* param) {

	int id = (int)param;

	if (fla) {
		fla = 0;
		start = GetTickCount64();
	}

	while (1) {
		//printf("%lld: #%d blocked\n", GetTickCount64() - start, id + 1);
		EnterCriticalSection(&cs);
		//printf("%lld: #%d ready\n", GetTickCount64() - start, id + 1);
		if (NK->head == NULL) {
			LeaveCriticalSection(&cs);
			//printf("#%d leave\n", id + 1);
			return 0;
		}
		Node* tmp = getBack(NK);
		int n = tmp->n, k = tmp->k;
		popBack(NK);

		j++;
		if (n == 0 && k == 0) {
			LeaveCriticalSection(&cs);

			EnterCriticalSection(&csP);
			P++;
			LeaveCriticalSection(&csP);
			continue;
		}
		if (k == 0 && n != 0) {
			LeaveCriticalSection(&cs);
			continue;
		}
		if (k <= n) {
			push(n, k - 1, NK);
			push(n - k, k, NK);
			LeaveCriticalSection(&cs);
			continue;
		}
		else {
			push(n, n, NK);
			LeaveCriticalSection(&cs);
		}
	}

	return NULL;
}

int main(int argc, char** argv) {

	FILE* input = fopen("input.txt", "r");
	if (!input) {
		cout << "Could'n open input file!" << endl;
		return 0;
	}

	FILE* output = fopen("output.txt", "w");
	FILE* timef = fopen("time.txt", "w");

	fscanf(input, "%lld\n%lld", &T, &N);
	fprintf(output, "%lld\n%lld\n", T, N);

	if (argc == 3) {
		T = atoi(argv[1]);
		N = atoi(argv[2]);
	}

	NK = init();
	push(N, N - 1, NK);
	inWork = new volatile long long* [T];
	for (unsigned long long i = 0; i < T; i++) {
		inWork[i] = new long long[2];
		inWork[i][0] = -1;
		inWork[i][1] = -1;
	}

	InitializeCriticalSection(&cs);
	InitializeCriticalSection(&csP);
	InitializeCriticalSection(&tCs);

	HANDLE* handles = new HANDLE[T];
	for (int l = 0; l < T; l++)
		handles[l] = CreateThread(0, 0, thread_entry, (void*)l, 0, 0);

	WaitForMultipleObjects(T, handles, TRUE, INFINITE);
	times = GetTickCount64() - start;

	for (int l = 0; l < T; l++)
		CloseHandle(handles[l]);

	DeleteCriticalSection(&cs);
	DeleteCriticalSection(&tCs);
	delete NK;
	delete[] handles;

	fprintf(output, "%lld", P);
	fprintf(timef, "%lld", times);
	printf("%.3f\n", (float)times / 1000);

	fclose(input);
	fclose(output);
	fclose(timef);
	return 0;
}

////////////////////////////////////////////
List* volatile init() {
	List* volatile list = new List;
	list->head = NULL;
	list->tail = NULL;
	return list;
}

void printList(List* volatile list) {
	Node* tmp = list->head;
	while (tmp) {
		printf("%d, %d\n", tmp->n, tmp->k);
		tmp = tmp->next;
	}
}

void push(int n, int k, List* volatile list) {
	Node* tmp = new Node;
	if (!tmp) {
		printf("Push error\n");
		return;
	}

	tmp->n = n;
	tmp->k = k;
	tmp->next = list->head;
	if (list->head)
		list->head->prev = tmp;
	if (list->tail == NULL)
		list->tail = tmp;

	list->head = tmp;
	if (!list->tail) list->tail = tmp;

}

Node* getFront(List* volatile list) {
	Node* tmp = list->head;
	return tmp;
}

Node* getBack(List* volatile list) {
	Node* tmp = list->tail;
	return tmp;
}

Node* popFront(List* volatile list) {
	Node* prev;
	if (list->head == NULL) {
		printf("popFront Error\n");
		return NULL;
	}

	prev = list->head;
	if (list->tail == list->head) {
		list->tail = NULL;
		list->head = NULL;
	}
	else list->head = list->head->next;

	delete prev;
	return list->head;
}

Node* popBack(List* volatile list) {
	Node* next;
	if (list->tail == NULL) {
		printf("popBack Error\n");
		return NULL;
	}

	next = list->tail;
	if (list->tail == list->head) {
		list->tail = NULL;
		list->head = NULL;
	}
	else list->tail = list->tail->prev;

	delete next;
	return list->head;
}
