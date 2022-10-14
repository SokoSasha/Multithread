#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

#define phN 5
#define left ((id + phN - 1) % phN)
#define right ((id + 1) % phN)
#define Th 0
#define H 1
#define Eat 2
#define sleep usleep(stateTime*1000)

volatile time_t stateTime, startTime, endTime, total;
sem_t sem[phN], prnt;
volatile char* Ph;
volatile int* quants;

time_t GetTickCountMs();

//Если соседни не едят, то семафор поднимается
void test(int id){
	if (Ph[id] == H && Ph[left] != Eat && Ph[right] != Eat){
		Ph[id] = Eat;
		sem_post(&sem[id]);
	}
}

//Пытаемся взять вилки, ждем пока оба соседа не будут в состоянии раздумья
void takeForks(int id){
	Ph[id] = H;
	test(id);
	//printf("%ld: #%d blocked\n", GetTickCountMs() - startTime, id + 1);
	sem_wait(&sem[id]);
	//printf("%ld: #%d ready\n", GetTickCountMs() - startTime, id + 1);
}

//Переходим в состояние раздумья, "даем разрешение" соседним философом, если есть такая возможность
void putForks(int id){
	Ph[id] = Th;
	test(left);
	test(right);
}

//Вывод информации
void printInfo(int id, const char* ch){
	sem_wait(&prnt);
	quants[id]++;
	cout << GetTickCountMs() - startTime << ":" << id + 1 << ":" << ch << endl;
	sem_post(&prnt);
}

bool fla = 1;
void* philosopher(void* param){
	int id = (int)param;
	
	if (fla){
		fla = 0;
		startTime = GetTickCountMs();
		endTime = startTime + total;
	}
	
	while(1){
		sleep; //Изначально все философы спят
		takeForks(id); //Пробуем поесть
		if (GetTickCountMs() >= endTime){ //Проверяем, не закончилось ли время, пока мы ждали свой очереди
			putForks(id);
			return 0;
		}
		
		printInfo(id, "T->E");
		
		sleep; //кушоем
		putForks(id); //освобождаем вилки
		
		printInfo(id, "E->T");
	}
	return 0;
}

int main(int argc, char **argv){
	stateTime = (clock_t)atoi(argv[2]);
	total = (time_t)atoi(argv[1]);
	Ph = new char[phN];
	for (int i = 0; i < phN; i++){
		Ph[i] = 1;
		sem_init(&sem[i], 0, 0);
	}
	
	quants = new int[phN];
	for (int i = 0; i < phN; i++)
		quants[i] = 0;
	
	sem_init(&prnt, 0, 1); //нужен чтобы не напечаталось одновременно 2 строки
	
	for (int i = 0; i < phN; i++)
		Ph[i] = Th;
	
	pthread_t phil[phN];
	
	for (int i = 0; i < phN; i++)
		pthread_create(&phil[i], NULL, philosopher, (void*)i);
	
	
	for (int i = 0; i < phN; i++)
		pthread_join(phil[i], NULL);
		
	/*for (int i = 0; i < phN; i++)
		printf("%d - %d\n", i, quants[i]);*/
		
	sem_destroy(&prnt);
	for(int i = 0;i < phN; i++)
		sem_destroy(&sem[i]);
	delete[] Ph;
	delete[] quants;
	
	return 0;
}

//Функция получения времени и миллисекундах
time_t GetTickCountMs()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000 + (time_t)ts.tv_nsec / 1000000);
}
