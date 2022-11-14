/*
*	DKU Operating System Lab
*	    Lab1 (Scheduler Algorithm Simulator)
*	    Student id : 32184064
*	    Student name : Jung Dohyeon
*
*   lab1_sched_types.h :
*       - lab1 header file.
*       - must contains scueduler algorithm function's declations.
*
*/

#ifndef _LAB1_HEADER_H
#define _LAB1_HEADER_H

// definition of process structure
typedef struct {
    char p_name;    // 프로세스 이름
    char *state;    // 상태 저장
    int arr_t, service_t, run_tq, wait_t;
    int pid;    // process ID
    int Qfrom;  // MLFQ 사용 시 몇 번째 큐에 있는지 확인
    int ticket; // lottery ticket
} t_pro;

// definition of queue structure
typedef struct {
    int *buf;   // 큐 저장소
    int front;  // front
    int rear;   // rear
    int sizeOfQ;
    int count;  // 큐 내부 개수
    int ticket;
} ReadyQueue;

void InitializeQ(ReadyQueue *q, int n);
int FullCheck(ReadyQueue *q);
int EmptyCheck(ReadyQueue *q);
void Enqueue(ReadyQueue *q, int pid);
int Dequeue(ReadyQueue *q);
void sort_arr(t_pro *p, int n);
void display(int** table, int n, int service_sum);
void fcfs (t_pro *tp, int n);
void RR (t_pro *tp, int n, int time_quantum);
void SPN (t_pro *tp, int n);
void SRT(t_pro *tp, int n);
void HRRN (t_pro *tp, int n);
void MLFQ (t_pro *tp, int n, int mlfq_time);
void Lottery (t_pro *tp, int n);

#endif /* LAB1_HEADER_H*/



