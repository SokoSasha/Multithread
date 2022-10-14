#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <windows.h>
#include <time.h>

using namespace std;

#define ll long long
#define ull unsigned long long

volatile ll* arr;
volatile ll** inWork;
volatile ll N = 0, T = 0;

struct list {
    ll start;
    ll end;
    list* next;
    list* prev;
};

list* volatile queueue, * volatile tail;
HANDLE event;
HANDLE mutex;
time_t times = 0, startT = 0;

#define locke WaitForSingleObject(event, INFINITE)
#define unlocke SetEvent(event)
#define lockm WaitForSingleObject(mutex, INFINITE)
#define unlockm ReleaseMutex(mutex)


////// Функции для списка ///////
void init(ll st, ll fin);
void add_elem(ll st, ll fin);
void delete_head();
/////////////////////////////////

ll partition(ll start, ll end)
{
    ll pivot = arr[start];

    ll count = 0;
    for (ll i = start + 1; i <= end; i++) {
        if (arr[i] <= pivot)
            count++;
    }

    ll pivotIndex = start + count;
    swap(arr[pivotIndex], arr[start]);

    ll i = start, j = end;

    while (i < pivotIndex && j > pivotIndex) {


        while (arr[i] <= pivot) {
            i++;
        }

        while (arr[j] > pivot) {
            j--;
        }

        if (i < pivotIndex && j > pivotIndex) {
            swap(arr[i++], arr[j--]);
        }
    }

    return pivotIndex;
}

ll partitionY(ll tmp[], ll start, ll end)
{
    ll pivot = tmp[start];

    ll count = 0;
    for (ll i = start + 1; i <= end; i++) {
        if (tmp[i] <= pivot)
            count++;
    }

    ll pivotIndex = start + count;
    swap(tmp[pivotIndex], tmp[start]);

    ll i = start, j = end;

    while (i < pivotIndex && j > pivotIndex) {


        while (tmp[i] <= pivot) {
            i++;
        }

        while (tmp[j] > pivot) {
            j--;
        }

        if (i < pivotIndex && j > pivotIndex) {
            swap(tmp[i++], tmp[j--]);
        }
    }

    return pivotIndex;
}

void solve_youself(ll tmp[], ll start, ll end)
{
    if (start >= end || start >= N || end >= N)
        return;

    ll p = partitionY(tmp, start, end);

    solve_youself(tmp, start, p - 1);

    solve_youself(tmp, p + 1, end);
}

bool fla = 1;
DWORD WINAPI Creation(void* param)
{
    int id = (int)param;
    if (fla) {
        fla = 0;
        startT = GetTickCount64();
    }

    while (1) {
        //printf("%lld: #%d blocked\n", GetTickCount64() - startT, id+1);
        locke;
        //printf("%lld: #%d ready\n", GetTickCount64() - startT, id+1);
        if (queueue == NULL) {
            unlocke;
            //printf("%lld: #%d leave\n", GetTickCount64() - startT, id)+1;
            return 0;
        }
        ll start = queueue->start, end = queueue->end;
        ll st_el = arr[start], end_el = arr[end];
        bool fll = 0;
        for (ll i = 0; i < T; i++) {
            if (i == id) continue;
            if (inWork[i][0] == start && inWork[i][1] == end) {
                fll = 1;
                break;
            }
        }
        if (fll) {
            unlocke;
            continue;
        }
        inWork[id][0] = start;
        inWork[id][1] = end;
        unlocke;

        if (start >= end) {
            //printf("%lld: #%d blocked\n", GetTickCount64() - startT, id + 1);
            locke;
            //printf("%lld: #%d ready\n", GetTickCount64() - startT, id + 1);
            delete_head();
            unlocke;
            continue;
        }

        if (end - start < 1000) {
            //printf("%lld: #%d blocked\n", GetTickCount64() - startT, id + 1);
            locke;
            //printf("%lld: #%d ready\n", GetTickCount64() - startT, id + 1);
            delete_head();
            unlocke;

            ll leng = end - start + 1;
            ll* tmp = new ll[leng];

            lockm;
            for (ll i = 0; i < leng; i++)
                tmp[i] = arr[i + start];
            unlockm;

            solve_youself(tmp, 0, leng - 1);

            lockm;
            for (ll i = 0; i < leng; i++)
                arr[i + start] = tmp[i];
            unlockm;

            delete[] tmp;
            continue;
        }
        lockm;
        ll p = partition(start, end);
        unlockm;
        //printf("%lld: #%d blocked\n", GetTickCount64() - startT, id + 1);
        locke;
        //printf("%lld: #%d ready\n", GetTickCount64() - startT, id + 1);
        delete_head();
        add_elem(start, p - 1);
        add_elem(p + 1, end);
        unlocke;
    }
    return 0;
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
    arr = new ll[N];
    init(0, N - 1);
    for (ll i = 0; i < N; i++)
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

    fprintf(timef, "%lld", times);
    fclose(output);
    fclose(timef);
}

int main()
{
    char boo = get_file();
    if (boo == -1) return -1;
    inWork = new volatile ll * [T];
    for (ull i = 0; i < T; i++)
        inWork[i] = new ll[2];

    event = CreateEvent(NULL, FALSE, TRUE, NULL);
    mutex = CreateMutex(NULL, FALSE, NULL);
    HANDLE* handles = new HANDLE[T];

    for (int i = 0; i < T; i++)
        handles[i] = CreateThread(0, 0, Creation, (void*)i, 0, 0);

    WaitForMultipleObjects(T, handles, TRUE, INFINITE);
    times = GetTickCount64() - startT;

    for (int i = 0; i < T; i++)
        CloseHandle(handles[i]);

    CloseHandle(event);
    CloseHandle(mutex);

    put_files();

    //printf("%lld %lld - %.3f\n", T, N, (float)times / 1000);

    for (int i = 0; i < N - 1; i++)
        if (arr[i] > arr[i + 1]) {
            printf("Not sorted! %d\n", i);
            break;
        }

    delete[] handles;
    delete[] arr;
    return 0;
}


/////////////////// Фукнции для работы со списком //////////////////////////
void init(ll st, ll fin)
{
    list* lst = new list;
    lst->start = st;
    lst->end = fin;
    lst->next = NULL;
    lst->prev = NULL;
    queueue = lst;
    tail = lst;
}

void add_elem(ll st, ll fin)
{
    if (!tail) {
        init(st, fin);
        return;
    }
    list* temp = new list, * p = tail->next;

    tail->next = temp;
    temp->start = st;
    temp->end = fin;
    temp->next = p;
    temp->prev = tail;
    if (p != NULL)
        p->prev = temp;
    tail = temp;
}

void delete_head()
{
    if (queueue == tail) {
        delete queueue;
        queueue = NULL;
        tail = NULL;
        return;
    }
    list* temp = queueue->next;
    if (!temp) {
        delete queueue;
        queueue = temp;
        return;
    }

    temp->prev = NULL;
    delete queueue;
    queueue = temp;
}