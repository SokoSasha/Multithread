#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>

using namespace std;


unsigned long long N, T;
volatile long long* arr;

volatile time_t times = 0, start = 0;
time_t GetTickCountMs();

pthread_mutex_t timeM;
sem_t sem_Q;
sem_t sem_S;
sem_t sem_merge;
int Q = 0, S = 0;

struct Node{
	unsigned long long low;
	unsigned long long mid;
	unsigned long long high;
	Node* next;
	Node* prev;
};

struct List{
	Node* head;
	Node* tail;
};

List* volatile queue, * volatile stack;

//////////////////////////
List* volatile init();
void push(unsigned long long low, unsigned long long mid, unsigned long long high, List* volatile list);
Node* getFront(List* volatile list);
Node* getBack(List* volatile list);
Node* popFront(List* volatile list);
Node* popBack(List* volatile list);
void printList(List* volatile list);
void pushOrd(unsigned long long low, unsigned long long mid, unsigned long long high, List* volatile list);
//////////////////////////

long long* mergeY(long long tmp[], unsigned long long l, unsigned long long m, unsigned long long r);
long long* mergeYourself(long long tmp[], unsigned long long low, unsigned long long high);

/////////////////////////////


void merge(unsigned long long low, unsigned long long mid, unsigned long long high){
	unsigned long long* left = new unsigned long long[mid - low + 1];
	unsigned long long* right = new unsigned long long[high - mid];

	unsigned long long n1 = mid - low + 1, n2 = high - mid, i, j;

	sem_wait(&sem_merge);
	for (i = 0; i < n1; i++)
		left[i] = arr[i + low];

	for (i = 0; i < n2; i++)
		right[i] = arr[i + mid + 1];

	unsigned long long k = low;
	i = j = 0;

	while (i < n1 && j < n2) {
		if (left[i] <= right[j])
			arr[k++] = left[i++];
		else
			arr[k++] = right[j++];
	}

	while (i < n1) {
		arr[k++] = left[i++];
	}

	while (j < n2) {
		arr[k++] = right[j++];
	}
	sem_post(&sem_merge);
}
bool fla = 1;
void* merge_sort(void* param){
	int id = (int)param;
	
	if (fla) {
		fla = 0;
		start = GetTickCountMs();
	}
	
	while (1) {
		cout << id + 1 << " blocked" << endl;
		sem_wait(&sem_Q);
		if (queue->head == NULL) {
			sem_post(&sem_Q);
			break;
		}
		Node* curT = getBack(queue);
		unsigned long long low = curT->low, mid = curT->low + (curT->high - curT->low) / 2, high = curT->high;
		queue->head = popBack(queue);
		if (high - low < 1000 && low < high){
			long long* tmp = new long long [high - low + 1];
			sem_wait(&sem_merge);
			for (unsigned long long i = 0; i <= high - low; i++)
				tmp[i] = arr[low + i];
			sem_post(&sem_merge);
			
			tmp = mergeYourself(tmp, 0, high - low);
			
			sem_wait(&sem_merge);
			for (unsigned long long i = 0; i <= high - low;i++)
				arr[low + i] = tmp[i];
			sem_post(&sem_merge);
			
			delete[] tmp;
		}
		else if (low < high) {
			push(low, 0, mid, queue);
			push(mid + 1, 0, high, queue);
			sem_wait(&sem_S);
			pushOrd(low, mid, high, stack);
			sem_post(&sem_S);
		}
		sem_post(&sem_Q);
	}
	while (1) {
		sem_wait(&sem_S);
		if (stack->head == NULL) {
			sem_post(&sem_S);
			break;
		}
		Node* curTS = getFront(stack);
		unsigned long long low = curTS->low, mid = curTS->mid, high = curTS->high;
		stack->head = popFront(stack);
		merge(low, mid, high);
		sem_post(&sem_S);
	}
	cout << id + 1 << " leave" << endl;
}

char get_file() {
	FILE* input = fopen("input.txt", "r");
	if (!input) {
		cout << "Could'n open input file!" << endl;
		return -1;
	}

	FILE* output = fopen("output.txt", "w");

	fscanf(input, "%llu\n%llu", &T, &N);
	fprintf(output, "%llu\n%llu\n", T, N);
	arr = new long long[N];
	
	for (unsigned long long i = 0; i < N; i++)
		fscanf(input, "%llu", &arr[i]);
	
	fclose(input);
	fclose(output);
	
	return 0;
}

void put_files() {
	FILE* output = fopen("output.txt", "a");
	FILE* timef = fopen("time.txt", "w");

	for (int i = 0; i < N; i++)
		fprintf(output, "%llu ", arr[i]);

	fprintf(timef, "%ld", times);
	fclose(output);
	fclose(timef);
}

int main()
{
	if (get_file() < 0) return -1;
	
	pthread_t threads[T];
	pthread_mutex_init(&timeM, 0);
	sem_init(&sem_Q, 0, 1);
	sem_init(&sem_S, 0, 1);
	sem_init(&sem_merge, 0, 1);
	queue = init();
	stack = init();
	push(0, 0, N-1, queue);

	for (int i = 0; i < T; i++)
		pthread_create(&threads[i], NULL, merge_sort, (void*)i);

	for (int i = 0; i < T; i++)
		pthread_join(threads[i], NULL);
	times = GetTickCountMs() - start;
		
	sem_destroy(&sem_Q);
	sem_destroy(&sem_S);
	sem_destroy(&sem_merge);
	pthread_mutex_destroy(&timeM);

	put_files();
	
	for (int i = 0; i < N-1; i++)
		if (arr[i] > arr[i+1]){
			printf("Not sorted! %d\n", i);
			break;
		}

	cout << (float)times / 1000 << endl;
		
	delete[] arr;

	return 0;
}

//////////////////////////////////////////////
time_t GetTickCountMs()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000 + (time_t)ts.tv_nsec / 1000000);
}

//////////////////////////////////////////////
long long* mergeY(long long tmp[], unsigned long long l, unsigned long long m, unsigned long long r){
	unsigned long long i, j, k;
	unsigned long long n1 = m - l + 1;
	unsigned long long n2 = r - m;
	
	long long *L = new long long[n1], *R = new long long[n2];
	
	for (i = 0; i < n1; i++)
		L[i] = tmp[l + i];
	for (j = 0; j < n2; j++)
		R[j] = tmp[m + 1+ j];
	
	i = 0;
	j = 0;
	k = l;
	while (i < n1 && j < n2){
		if (L[i] <= R[j]){
			tmp[k] = L[i];
			i++;
		}
		else{
			tmp[k] = R[j];
			j++;
		}
		
		k++;
	}
	
	while (i < n1){
		tmp[k] = L[i];
		i++;
		k++;
	}
	
	while (j < n2){
		tmp[k] = R[j];
		j++;
		k++;
	}
	
	return tmp;
}

long long* mergeYourself(long long tmp[], unsigned long long low, unsigned long long high){
	unsigned long long mid = low + (high - low) / 2;
	if (low < high) {
		tmp = mergeYourself(tmp, low, mid);
		tmp = mergeYourself(tmp, mid + 1, high);
		tmp = mergeY(tmp, low, mid, high);
	}
	
	return tmp;
}

//////////////////////////////////////////////

List* volatile init(){
	List* volatile list = new List;
	list->head = NULL;
	list->tail = NULL;
	return list;
}

void printList(List* volatile list) {
	Node* tmp = list->head;
	while (tmp) {
		printf("	l: %02llu, m: %02llu, h: %02llu. Dif: %02llu\n", tmp->low, tmp->mid, tmp->high, tmp->high - tmp->low);
		tmp = tmp->next;
	}
}

void push(unsigned long long low, unsigned long long mid, unsigned long long high, List* volatile list) {
	Node* tmp = new Node;
	if (!tmp) {
		printf("Push error\n");
		return;
	}

	tmp->low = low;
	tmp->mid = mid;
	tmp->high = high;
	tmp->next = list->head;
	if (list->head)
		list->head->prev = tmp;
	if (list->tail == NULL)
		list->tail = tmp;
		
	list->head = tmp;
	if (!list->tail) list->tail = tmp;

}

void pushOrd(unsigned long long low, unsigned long long mid, unsigned long long high, List* volatile list) {
	Node* tmp = new Node;
	if (!tmp) {
		printf("Push error\n");
		return;
	}

	Node* cmp = list->head;
	if (!cmp) {
		push(low, mid, high, list);
		return;
	}
	while (high - low > cmp->high - cmp->low) {
		cmp = cmp->next;
		if (!cmp) break;
	}

	tmp->low = low;
	tmp->mid = mid;
	tmp->high = high;
	Node* prcmp = cmp->prev;
	tmp->prev = prcmp;
	tmp->next = cmp;
	cmp->prev = tmp;

	if (cmp == list->head)
		list->head = tmp;
	else prcmp->next = tmp;
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
