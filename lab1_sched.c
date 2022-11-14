/*
*	DKU Operating System Lab
*	    Lab1 (Scheduler Algorithm Simulator)
*	    Student id : 32184064
*	    Student name : Jung Dohyeon
*
*   lab1_sched.c :
*       - Lab1 source file.
*       - Must contains scueduler algorithm function'definition.
*
*/

#include <aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <asm/unistd.h>
#include <math.h>

#include "lab1_sched_types.h"

// initialize Queue
void InitializeQ(ReadyQueue *q, int n){
    q->buf = (int *)malloc(sizeof(int)*n);
    q->sizeOfQ = n;
    q->front = q->rear= 0;
    q->count = 0;
}

// check queue full
int FullCheck(ReadyQueue *q) {
    return q->count == q->sizeOfQ;
}

// check queue empty
int EmptyCheck(ReadyQueue *q) {
    return q->count == 0;
}

// Insert Queue
void Enqueue(ReadyQueue *q, int pid){
    if (FullCheck(q)){
        printf("Queue is FULL!\n");
    } else {
        q->buf[q->rear] = pid;
        q->rear = (q->rear+1)%(q->sizeOfQ);
        q->count++;
    }
}

// 맨 앞 원소를 꺼내 반환
int Dequeue(ReadyQueue *q){
    int pid=0;
    if(EmptyCheck(q)){
        printf("Queue is EMPTY!\n");
        return pid;
    }
    else {
        pid = q->buf[q->front];
        q->front = (q->front+1)%(q->sizeOfQ);
        q->count--;
        return pid;
    }
}

// 도착시간 순으로 정렬
void sort_arr(t_pro *p, int n) {
    int i, j;
    t_pro temp; // 임시 저장 공간
    for (i = 0; i<n-1; i++){
        for(j=0; j<n-i-1; j++){
            if(p[j].arr_t > p[j+1].arr_t){
                temp = p[j];
                p[j] = p[j+1];
                p[j+1] = temp;
            }
            // 같은 도착 시간이라면 알파벳 순으로 정렬
            else if ((p[j].arr_t == p[j+1].arr_t) && (p[j].p_name > p[j+1].p_name)){
                temp = p[j];
                p[j] = p[j+1];
                p[j+1] = temp;
            }
        }
    }
}

// 결과 출력 함수
void display(int** table, int n, int service_sum){
    int i, j = 0;
    for(i=0; i<n; i++){
        printf("%c : ", 65+i);  // 알파벳 순서로 나타내기 위해 ASCII Code 사용
        for(j=0; j<service_sum; j++){
            if(table[i][j] == 1){
                printf("■ ");    // 실행 중인 표시
            }
            else {
                printf("□ ");    // 미 실행 중 표시
            }
        }
        printf("\n");
    }
}

// FCFS
void fcfs (t_pro *tp, int n){
    int i, service_sum = 0;
    int time_s = 0; // 매 초 카운트할 변수
    ReadyQueue ready_q;   // 한 개의 레디 큐 생성
    ReadyQueue running_q;    // 실행되고 있는 큐 생성
    InitializeQ(&ready_q, n);  // 레디 큐는 n개 만큼 크기 규정
    InitializeQ(&running_q, 1);   // 실행되고 있는 프로세스는 1개로 한정
    t_pro p[n];
    
    // copy Struct process
    for(i=0; i<n; i++){
        p[i].p_name = tp[i].p_name;
        p[i].arr_t = tp[i].arr_t;
        p[i].service_t = tp[i].service_t;
    }
    
    // 총 서비스 시간의 합 구하기
    for(i=0; i<n; i++){
        service_sum += p[i].service_t;
    }
    
    // 표로 출력하기 위한 2차원 배열 동적 할당 (0으로 초기화)
    int** table = (int**)calloc(n,sizeof(int*));
    for(i=0; i<n; i++){
        table[i] = (int*)calloc(service_sum,sizeof(int));
    }
    
    int x = 0;
    for(time_s = 0; time_s <= service_sum; time_s++){
        // 도착 시간이 오면 레디 큐에서 대기
        while (x<n && p[x].arr_t == time_s){
            p[x].pid = p[x].p_name - 64;
            Enqueue(&ready_q, p[x].pid);
            x++;
        }
        
        // ReadyQueue에 대기하는 원소가 있고 현재 running프로세스가 없는 경우
        if ((!EmptyCheck(&ready_q)) && (EmptyCheck(&running_q))){
            int carry = Dequeue(&ready_q);
            Enqueue(&running_q, carry);
            p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
            table[carry-1][time_s] = 1;
        }
        // 러닝 프로세스가 있는 경우
        else if (!EmptyCheck(&running_q)){
            int run_pid = Dequeue(&running_q);
            p[run_pid-1].service_t--;     // decrease service_t 1
            if(p[run_pid-1].service_t > 0){
                table[run_pid-1][time_s] = 1;
                Enqueue(&running_q, p[run_pid-1].pid);
            } else if (p[run_pid-1].service_t==0){    // service_t가 0이면 terminated
                p[run_pid-1].state = "terminated";
                if(!EmptyCheck(&ready_q)){     // ready_queue에 대기 원소가 있다면 스케줄링
                    int carry = Dequeue(&ready_q);
                    Enqueue(&running_q, carry);
                    p[carry-1].state = "running";
                    table[carry-1][time_s] = 1;
                }
            }
        }
    }
    display(table, n, service_sum);
}

// Round-Robin
void RR (t_pro *tp, int n, int time_quantum){
    int i, service_sum = 0;    // 전체 서비스 시간
    int time_s = 0; // 매 초 카운트할 변수
    ReadyQueue ready_q;   // 한 개의 레디 큐 생성
    ReadyQueue running_q;    // 실행되고 있는 큐 생성
    InitializeQ(&ready_q, n);  // 레디 큐는 n개 만큼 크기 규정
    InitializeQ(&running_q, 1);   // 실행되고 있는 프로세스는 1개로 한정
    t_pro p[n];
    
    // copy Struct process
    for(i=0; i<n; i++){
        p[i].p_name = tp[i].p_name;
        p[i].arr_t = tp[i].arr_t;
        p[i].service_t = tp[i].service_t;
    }
    
    // 총 서비스 시간의 합 구하기
    for(i=0; i<n; i++){
        service_sum += p[i].service_t;
    }
    
    // 표로 출력하기 위한 2차원 배열 동적 할당 (0으로 초기화)
    int** tableRR = (int**)calloc(n,sizeof(int*));
    for(i=0; i<n; i++){
        tableRR[i] = (int*)calloc(service_sum,sizeof(int));
    }
    
    int x = 0;
    for(time_s = 0; time_s <= service_sum; time_s++){
        while (x<n && p[x].arr_t == time_s){
            p[x].pid = p[x].p_name - 64;
            p[x].run_tq = 0;    // 프로세스가 사용한 시간
            Enqueue(&ready_q, p[x].pid);
            x++;
        }
        // ReadyQueue에 대기하는 원소가 있고 현재 running프로세스가 없는 경우
        if ((!EmptyCheck(&ready_q)) && (EmptyCheck(&running_q))){
            int carry = Dequeue(&ready_q);
            Enqueue(&running_q, carry);
            p[carry-1].state = "running"; // running상태로 프로세스 스케줄링
            p[carry-1].run_tq = 1;
            tableRR[carry-1][time_s] = 1;
        }
        // 러닝 프로세스가 있는 경우
        else if (!EmptyCheck(&running_q)){
            int run_p = Dequeue(&running_q);
            p[run_p-1].service_t--;     // decrease 1 service_t
            // 서비스 타임이 아직 남아 있고 타임 퀀텀만큼 안 사용한 경우
            if((p[run_p-1].service_t > 0) && (p[run_p-1].run_tq < time_quantum)){
                tableRR[run_p-1][time_s] = 1;
                Enqueue(&running_q, p[run_p-1].pid);
                p[run_p-1].run_tq++;
            }   // 타임 퀀텀 만큼 사용 하고 서비스 타임은 남아 있는 경우
            else if((p[run_p-1].service_t > 0) && (p[run_p-1].run_tq == time_quantum)){
                Enqueue(&ready_q, p[run_p-1].pid);  // return to ready_queue
                if(!EmptyCheck(&ready_q)){     // ready_queue에 대기 원소가 있다면 다음 원소 스케줄링
                    int carry = Dequeue(&ready_q);
                    Enqueue(&running_q, carry);
                    p[carry-1].state = "running";
                    p[carry-1].run_tq = 1;
                    tableRR[carry-1][time_s] = 1;
                }
            }
            else if (p[run_p-1].service_t==0){    // service_t가 0이면 terminated
                p[run_p-1].state = "terminated";
                if(!EmptyCheck(&ready_q)){     // ready_queue에 대기 원소가 있다면 스케줄링
                    int carry = Dequeue(&ready_q);
                    Enqueue(&running_q, carry);
                    p[carry-1].state = "running";
                    p[carry-1].run_tq = 1;
                    tableRR[carry-1][time_s] = 1;
                }
            }
        }
    }
    display(tableRR, n, service_sum);
}

// SPN (SJF)
void SPN (t_pro *tp, int n){
    int i, j, service_sum = 0;
    int time_s = 0; // 매 초 카운트할 변수
    ReadyQueue ready_q;   // 한 개의 레디 큐 생성
    ReadyQueue running_q;    // 실행되고 있는 큐 생성
    InitializeQ(&ready_q, n);  // 레디 큐는 n개 만큼 크기 규정
    InitializeQ(&running_q, 1);   // 실행되고 있는 프로세스는 1개로 한정
    t_pro p[n];
    
    // copy Struct process
    for(i=0; i<n; i++){
        p[i].p_name = tp[i].p_name;
        p[i].arr_t = tp[i].arr_t;
        p[i].service_t = tp[i].service_t;
    }
    
    // 총 서비스 시간의 합 구하기
    for(i=0; i<n; i++){
        service_sum += p[i].service_t;
    }
    
    // 표로 출력하기 위한 2차원 배열 동적 할당 (0으로 초기화)
    int** table = (int**)calloc(n,sizeof(int*));
    for(i=0; i<n; i++){
        table[i] = (int*)calloc(service_sum,sizeof(int));
    }
    
    int* compareArr = (int*)malloc(n);
    
    int x = 0;
    for(time_s = 0; time_s <= service_sum; time_s++){
        // 도착 시간이 되면 ready queue에서 대기
        while(x<n && p[x].arr_t == time_s){
            p[x].pid = p[x].p_name - 64;
            Enqueue(&ready_q, p[x].pid);
            x++;
        }
        
        // ReadyQueue에 대기하는 원소가 있고 현재 running프로세스가 없는 경우
        if ((!EmptyCheck(&ready_q)) && (EmptyCheck(&running_q))){
            int numberOfQ = ready_q.count;
            for(i=0; i<numberOfQ; i++){
                compareArr[i] = Dequeue(&ready_q);
            }
            int temp; // 임시 저장 공간
            // 현재 큐 안에서 실행 시간 짧은 순으로 정렬
            for (i = 0; i<numberOfQ-1; i++){
                for(j=0; j<numberOfQ-i-1; j++){
                    if(p[compareArr[j]-1].service_t > p[compareArr[j+1]-1].service_t){
                        temp = compareArr[j];
                        compareArr[j] = compareArr[j+1];
                        compareArr[j+1] = temp;
                    }
                }
            }
            // 실행 시간 짧은 순서로 큐에 삽입
            for(i = 0; i<numberOfQ; i++){
                Enqueue(&ready_q, compareArr[i]);
            }
            // 가장 짧은 것을 꺼내서 실행 시켜준다.
            int carry = Dequeue(&ready_q);
            Enqueue(&running_q, carry);
            p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
            table[carry-1][time_s] = 1;
        }
        // 러닝 프로세스가 있는 경우
        else if (!EmptyCheck(&running_q)){
            int run_pid = Dequeue(&running_q);
            p[run_pid-1].service_t--;     // decrease 1 service_t
            if(p[run_pid-1].service_t > 0){
                table[run_pid-1][time_s] = 1;
                Enqueue(&running_q, p[run_pid-1].pid);
            } else if (p[run_pid-1].service_t==0){    // service_t가 0이면 terminated
                p[run_pid-1].state = "terminated";
                if(!EmptyCheck(&ready_q)){     // ready_queue에 대기 원소가 있다면 스케줄링
                    int numberOfQ = ready_q.count;
                    for(i=0; i<numberOfQ; i++){
                        compareArr[i] = Dequeue(&ready_q);
                    }
                    int temp; // 임시 저장 공간
                    for (i = 0; i<numberOfQ-1; i++){
                        for(j=0; j<numberOfQ-i-1; j++){
                            if(p[compareArr[j]-1].service_t > p[compareArr[j+1]-1].service_t){
                                temp = compareArr[j];
                                compareArr[j] = compareArr[j+1];
                                compareArr[j+1] = temp;
                            }
                        }
                    }
                    for(i = 0; i<numberOfQ; i++){
                        Enqueue(&ready_q, compareArr[i]);
                    }
                    int carry = Dequeue(&ready_q);
                    Enqueue(&running_q, carry);
                    p[carry-1].state = "running";
                    table[carry-1][time_s] = 1;
                }
            }
        }
    }
    
    display(table, n, service_sum);
}

// SRT
void SRT(t_pro *tp, int n){
    int i, j, service_sum = 0;
    int time_s = 0; // 매 초 카운트할 변수
    ReadyQueue ready_q;   // 한 개의 레디 큐 생성
    ReadyQueue running_q;    // 실행되고 있는 큐 생성
    InitializeQ(&ready_q, n);  // 레디 큐는 n개 만큼 크기 규정
    InitializeQ(&running_q, 1);   // 실행되고 있는 프로세스는 1개로 한정
    t_pro p[n];
    
    // copy Struct process
    for(i=0; i<n; i++){
        p[i].p_name = tp[i].p_name;
        p[i].arr_t = tp[i].arr_t;
        p[i].service_t = tp[i].service_t;
    }
    
    // 총 서비스 시간의 합 구하기
    for(i=0; i<n; i++){
        service_sum += p[i].service_t;
    }
    
    // 표로 출력하기 위한 2차원 배열 동적 할당 (0으로 초기화)
    int** table = (int**)calloc(n,sizeof(int*));
    for(i=0; i<n; i++){
        table[i] = (int*)calloc(service_sum,sizeof(int));
    }
    
    int* compareArr = (int*)malloc(n);  // 비교를 위한 배열 생성
    
    int x = 0;
    for(time_s = 0; time_s <= service_sum; time_s++){
        // 도착 시간이 되면 ready queue에서 대기
        while(x<n && p[x].arr_t == time_s){
            p[x].pid = p[x].p_name - 64;
            Enqueue(&ready_q, p[x].pid);
            x++;
        }
        
        // ReadyQueue에 대기하는 원소가 있고 현재 running프로세스가 없는 경우
        if ((!EmptyCheck(&ready_q)) && (EmptyCheck(&running_q))){
            int numberOfQ = ready_q.count;
            for(i=0; i<numberOfQ; i++){
                compareArr[i] = Dequeue(&ready_q);
            }
            int temp; // 임시 저장 공간
            // 현재 큐 안에서 실행 시간 짧은 순으로 정렬
            for (i = 0; i<numberOfQ-1; i++){
                for(j=0; j<numberOfQ-i-1; j++){
                    if(p[compareArr[j]-1].service_t > p[compareArr[j+1]-1].service_t){
                        temp = compareArr[j];
                        compareArr[j] = compareArr[j+1];
                        compareArr[j+1] = temp;
                    }
                }
            }
            // 실행 시간 짧은 순서로 큐에 삽입
            for(i = 0; i<numberOfQ; i++){
                Enqueue(&ready_q, compareArr[i]);
            }
            // 가장 짧은 것을 꺼내서 실행 시켜준다.
            int carry = Dequeue(&ready_q);
            Enqueue(&running_q, carry);
            p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
            table[carry-1][time_s] = 1;
        }
        
        // 러닝 프로세스가 있는 경우
        else if (!EmptyCheck(&running_q)){
            int run_pid = Dequeue(&running_q);
            p[run_pid-1].service_t--;     // decrease 1 service_t
            // 아직 서비스 시간이 남았으나 선점형으로 구현 하기 위해 레디 큐로 옮겨서 나머지 원소와 비교
            if(p[run_pid-1].service_t > 0){
                Enqueue(&ready_q, p[run_pid-1].pid);
                int numberOfQ = ready_q.count;
                for(i=0; i<numberOfQ; i++){
                    compareArr[i] = Dequeue(&ready_q);
                }
                int temp; // 임시 저장 공간
                for (i = 0; i<numberOfQ-1; i++){
                    for(j=0; j<numberOfQ-i-1; j++){
                        if(p[compareArr[j]-1].service_t > p[compareArr[j+1]-1].service_t){
                            temp = compareArr[j];
                            compareArr[j] = compareArr[j+1];
                            compareArr[j+1] = temp;
                        }
                    }
                }
                // 짧게 남은 순서로 큐에 삽입
                for(i = 0; i<numberOfQ; i++){
                    Enqueue(&ready_q, compareArr[i]);
                }
                int carry = Dequeue(&ready_q);
                Enqueue(&running_q, carry);
                p[carry-1].state = "running";
                table[carry-1][time_s] = 1;
            }
            else if (p[run_pid-1].service_t==0){    // service_t가 0이면 terminated
                p[run_pid-1].state = "terminated";
                if(!EmptyCheck(&ready_q)){     // ready_queue에 대기 원소가 있다면 스케줄링
                    int numberOfQ = ready_q.count;
                    for(i=0; i<numberOfQ; i++){
                        compareArr[i] = Dequeue(&ready_q);
                    }
                    int temp; // 임시 저장 공간
                    for (i = 0; i<numberOfQ-1; i++){
                        for(j=0; j<numberOfQ-i-1; j++){
                            if(p[compareArr[j]-1].service_t > p[compareArr[j+1]-1].service_t){
                                temp = compareArr[j];
                                compareArr[j] = compareArr[j+1];
                                compareArr[j+1] = temp;
                            }
                        }
                    }
                    for(i = 0; i<numberOfQ; i++){
                        Enqueue(&ready_q, compareArr[i]);
                    }
                    int carry = Dequeue(&ready_q);
                    Enqueue(&running_q, carry);
                    p[carry-1].state = "running";
                    table[carry-1][time_s] = 1;
                }
            }
        }
    }
    display(table, n, service_sum);
}

// HRRN
void HRRN (t_pro *tp, int n){
    int i, j, service_sum = 0;
    int time_s = 0; // 매 초 카운트할 변수
    ReadyQueue ready_q;   // 한 개의 레디 큐 생성
    ReadyQueue running_q;    // 실행되고 있는 큐 생성
    InitializeQ(&ready_q, n);  // 레디 큐는 n개 만큼 크기 규정
    InitializeQ(&running_q, 1);   // 실행되고 있는 프로세스는 1개로 한정
    t_pro p[n];
    
    // copy Struct process
    for(i=0; i<n; i++){
        p[i].p_name = tp[i].p_name;
        p[i].arr_t = tp[i].arr_t;
        p[i].service_t = tp[i].service_t;
    }
    
    // 총 서비스 시간의 합 구하기
    for(i=0; i<n; i++){
        service_sum += p[i].service_t;
    }
    
    // 표로 출력하기 위한 2차원 배열 동적 할당 (0으로 초기화)
    int** table = (int**)calloc(n,sizeof(int*));
    for(i=0; i<n; i++){
        table[i] = (int*)calloc(service_sum,sizeof(int));
    }
    
    int* compareArr = (int*)malloc(n);  // 비교를 위한 배열 생성
    
    int x = 0;
    for(time_s = 0; time_s <= service_sum; time_s++){
        // 도착 시간이 되면 ready queue에서 대기
        while(x<n && p[x].arr_t == time_s){
            p[x].pid = p[x].p_name - 64;
            Enqueue(&ready_q, p[x].pid);
            x++;
        }
        // ReadyQueue에 대기하는 원소가 있고 현재 running프로세스가 없는 경우
        if ((!EmptyCheck(&ready_q)) && (EmptyCheck(&running_q))){
            int numberOfQ = ready_q.count;
            for(i=0; i<numberOfQ; i++){
                compareArr[i] = Dequeue(&ready_q);
            }
            int temp; // 정렬을 위한 임시 저장 공간
            // 현재 큐 안에서 응답 시간이 큰 순서로 정렬
            for (i = 0; i<numberOfQ-1; i++){
                for(j=0; j<numberOfQ-i-1; j++){
                    if((((time_s - p[compareArr[j]-1].arr_t) + p[compareArr[j]-1].service_t) / p[compareArr[j]-1].service_t) < (((time_s - p[compareArr[j+1]-1].arr_t) + p[compareArr[j+1]-1].service_t) / p[compareArr[j+1]-1].service_t)){
                        temp = compareArr[j];
                        compareArr[j] = compareArr[j+1];
                        compareArr[j+1] = temp;
                    }
                }
            }
            // 응답시간이 큰 순서로 큐에 삽입
            for(i = 0; i<numberOfQ; i++){
                Enqueue(&ready_q, compareArr[i]);
            }
            // 가장 응답시간이 큰 것을 꺼내서 실행 시켜준다.
            int carry = Dequeue(&ready_q);
            Enqueue(&running_q, carry);
            p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
            table[carry-1][time_s] = 1;
        }
        // 러닝 프로세스가 있는 경우
        else if (!EmptyCheck(&running_q)){
            int run_pid = Dequeue(&running_q);
            p[run_pid-1].service_t--;     // decrease 1 service_t
            if(p[run_pid-1].service_t > 0){
                table[run_pid-1][time_s] = 1;
                Enqueue(&running_q, p[run_pid-1].pid);
            } else if (p[run_pid-1].service_t==0){    // service_t가 0이면 terminated
                p[run_pid-1].state = "terminated";
                if(!EmptyCheck(&ready_q)){     // ready_queue에 대기 원소가 있다면 스케줄링
                    int numberOfQ = ready_q.count;
                    for(i=0; i<numberOfQ; i++){
                        compareArr[i] = Dequeue(&ready_q);
                    }
                    int temp; // 임시 저장 공간
                    for (i = 0; i<numberOfQ-1; i++){
                        for(j=0; j<numberOfQ-i-1; j++){
                            if((((time_s - p[compareArr[j]-1].arr_t) + p[compareArr[j]-1].service_t) / p[compareArr[j]-1].service_t) < (((time_s - p[compareArr[j+1]-1].arr_t) + p[compareArr[j+1]-1].service_t) / p[compareArr[j+1]-1].service_t)){
                                temp = compareArr[j];
                                compareArr[j] = compareArr[j+1];
                                compareArr[j+1] = temp;
                            }
                        }
                    }
                    for(i = 0; i<numberOfQ; i++){
                        Enqueue(&ready_q, compareArr[i]);
                    }
                    int carry = Dequeue(&ready_q);
                    Enqueue(&running_q, carry);
                    p[carry-1].state = "running";
                    table[carry-1][time_s] = 1;
                }
            }
        }
    }
    display(table, n, service_sum);
}

// MLFQ (각 레디 큐에 부여된 시간이 모두 mlfq_time의 제곱 수로 커질 경우)
void MLFQ (t_pro *tp, int n, int mlfq_time){
    int i, service_sum = 0;
    int time_s = 0; // 매 초 카운트할 변수
    
    ReadyQueue ready_q0;   // 0단계 레디 큐 생성
    ReadyQueue ready_q1;   // 1단계 레디 큐 생성
    ReadyQueue ready_q2;   // 2단계 레디 큐 생성
    ReadyQueue running_q;    // 실행되고 있는 큐 생성
    
    InitializeQ(&ready_q0, n);    // 각 레디 큐들 n개 만큼 크기 규정 (0단계 큐 생성)
    InitializeQ(&ready_q1, n);    // (1단계 큐 생성)
    InitializeQ(&ready_q2, n);    // (2단계 큐 생성 - 최고 레벨 큐)
    InitializeQ(&running_q, 1);   // 실행되고 있는 프로세스는 1개로 한정
    t_pro p[n];
    
    // copy Struct process
    for(i=0; i<n; i++){
        p[i].p_name = tp[i].p_name;
        p[i].arr_t = tp[i].arr_t;
        p[i].service_t = tp[i].service_t;
    }
    
    // 총 서비스 시간의 합 구하기
    for(i=0; i<n; i++){
        service_sum += p[i].service_t;
    }
    
    // 표로 출력하기 위한 2차원 배열 동적 할당 (0으로 초기화)
    int** table = (int**)calloc(n,sizeof(int*));
    for(i=0; i<n; i++){
        table[i] = (int*)calloc(service_sum,sizeof(int));
    }
    
    int x = 0;
    for(time_s = 0; time_s <= service_sum; time_s++){
        // 도착 시간이 되면 ready queue에서 대기
        while (x<n && p[x].arr_t == time_s){
            p[x].pid = p[x].p_name - 64;
            Enqueue(&ready_q2, p[x].pid);
            p[x].run_tq = 0;
            x++;
        }
        
        // 러닝 프로세스가 없는 경우
        if (EmptyCheck(&running_q)){
            // 2단계 큐에 원소가 있는 경우 2단계 큐 부터 실행 시킨다.
            if (!EmptyCheck(&ready_q2)){
                int carry = Dequeue(&ready_q2);
                Enqueue(&running_q, carry);
                p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
                p[carry-1].run_tq = (int)pow(mlfq_time, 0);
                p[carry-1].Qfrom = 2;   // 2단계 큐에서 실행 되었다는 정보 저장
                table[carry-1][time_s] = 1;
            }
            // q2가 비었고 q1에 원소가 있는 경우
            else if (EmptyCheck(&ready_q2) && (!EmptyCheck(&ready_q1))){
                int carry = Dequeue(&ready_q1);
                Enqueue(&running_q, carry);
                p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
                p[carry-1].run_tq = (int)pow(mlfq_time, 1);
                p[carry-1].Qfrom = 1;   // 1단계 큐에서 실행 되었다는 정보 저장
                table[carry-1][time_s] = 1;
            }
            // q2, q1이 비었고 q0에 원소가 있는 경우
            else if (EmptyCheck(&ready_q2) && EmptyCheck(&ready_q1) && (!EmptyCheck(&ready_q0))){
                int carry = Dequeue(&ready_q0);
                Enqueue(&running_q, carry);
                p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
                p[carry-1].run_tq = (int)pow(mlfq_time, 2);
                p[carry-1].Qfrom = 0;   // 0단계 큐에서 실행 되었다는 정보 저장
                table[carry-1][time_s] = 1;
            }
        }
        
        // 러닝 프로세스가 있는 경우
        else if (!EmptyCheck(&running_q)){
            int run_pid = Dequeue(&running_q);
            p[run_pid-1].service_t--;
            p[run_pid-1].run_tq--;
            // 서비스 타임이 남아있는 경우
            if(p[run_pid-1].service_t > 0){
                if(p[run_pid-1].run_tq == 0){
                    // 2레벨 큐에서 온 경우
                    if(p[run_pid-1].Qfrom == 2){
                        // 만약 레디큐 전체에 아무 원소도 없다면 최상위큐 2 레벨에서 재 실행
                        if((EmptyCheck(&ready_q2)) && (EmptyCheck(&ready_q1)) && (EmptyCheck(&ready_q0))){
                            Enqueue(&ready_q2, p[run_pid-1].pid);
                        }
                        else {
                            Enqueue(&ready_q1, p[run_pid-1].pid);
                        }
                        // 다시 2단계 큐에 원소부터 탐색 시작
                        if (!EmptyCheck(&ready_q2)){
                            int carry = Dequeue(&ready_q2);
                            Enqueue(&running_q, carry);
                            p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
                            p[carry-1].run_tq = (int)pow(mlfq_time, 0);
                            p[carry-1].Qfrom = 2;   // 2단계 큐에서 실행 되었다는 정보 저장
                            table[carry-1][time_s] = 1;
                        }
                        // q2가 비었고 q1에 원소가 있는 경우
                        else if (EmptyCheck(&ready_q2) && (!EmptyCheck(&ready_q1))){
                            int carry = Dequeue(&ready_q1);
                            Enqueue(&running_q, carry);
                            p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
                            p[carry-1].run_tq = (int)pow(mlfq_time, 1);
                            p[carry-1].Qfrom = 1;   // 1단계 큐에서 실행 되었다는 정보 저장
                            table[carry-1][time_s] = 1;
                        }
                        // q2, q1이 비었고 q0에 원소가 있는 경우
                        else if (EmptyCheck(&ready_q2) && EmptyCheck(&ready_q1) && (!EmptyCheck(&ready_q0))){
                            int carry = Dequeue(&ready_q0);
                            Enqueue(&running_q, carry);
                            p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
                            p[carry-1].run_tq = (int)pow(mlfq_time, 2);
                            p[carry-1].Qfrom = 0;   // 0단계 큐에서 실행 되었다는 정보 저장
                            table[carry-1][time_s] = 1;
                        }
                        // 0과 1레벨 큐에서 온 경우 모두 0레벨로 삽입
                    } else if (p[run_pid-1].Qfrom == 1){
                        // 만약 레디큐 전체에 아무 원소도 없다면 최상위큐 2 레벨에서 재 실행
                        if((EmptyCheck(&ready_q2)) && (EmptyCheck(&ready_q1)) && (EmptyCheck(&ready_q0))){
                            Enqueue(&ready_q2, p[run_pid-1].pid);
                        }
                        else {
                            Enqueue(&ready_q0, p[run_pid-1].pid);
                        }
                        // 다시 2단계 큐에 원소부터 탐색 시작
                        if (!EmptyCheck(&ready_q2)){
                            int carry = Dequeue(&ready_q2);
                            Enqueue(&running_q, carry);
                            p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
                            p[carry-1].run_tq = (int)pow(mlfq_time, 0);
                            p[carry-1].Qfrom = 2;   // 2단계 큐에서 실행 되었다는 정보 저장
                            table[carry-1][time_s] = 1;
                        }
                        // q2가 비었고 q1에 원소가 있는 경우
                        else if (EmptyCheck(&ready_q2) && (!EmptyCheck(&ready_q1))){
                            int carry = Dequeue(&ready_q1);
                            Enqueue(&running_q, carry);
                            p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
                            p[carry-1].run_tq = (int)pow(mlfq_time, 1);
                            p[carry-1].Qfrom = 1;   // 1단계 큐에서 실행 되었다는 정보 저장
                            table[carry-1][time_s] = 1;
                        }
                        // q2, q1이 비었고 q0에 원소가 있는 경우
                        else if (EmptyCheck(&ready_q2) && EmptyCheck(&ready_q1) && (!EmptyCheck(&ready_q0))){
                            int carry = Dequeue(&ready_q0);
                            Enqueue(&running_q, carry);
                            p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
                            p[carry-1].run_tq = (int)pow(mlfq_time, 2);
                            p[carry-1].Qfrom = 0;   // 0단계 큐에서 실행 되었다는 정보 저장
                            table[carry-1][time_s] = 1;
                        }
                    } else  if(p[run_pid-1].Qfrom == 0){
                        // 만약 레디큐 전체에 아무 원소도 없다면 최상위큐 2 레벨에서 재 실행
                        if((EmptyCheck(&ready_q2)) && (EmptyCheck(&ready_q1)) && (EmptyCheck(&ready_q0))){
                            Enqueue(&ready_q2, p[run_pid-1].pid);
                        }
                        else {
                            Enqueue(&ready_q0, p[run_pid-1].pid);
                        }
                        // 다시 2단계 큐에 원소부터 탐색 시작
                        if (!EmptyCheck(&ready_q2)){
                            int carry = Dequeue(&ready_q2);
                            Enqueue(&running_q, carry);
                            p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
                            p[carry-1].run_tq = (int)pow(mlfq_time, 0);
                            p[carry-1].Qfrom = 2;   // 2단계 큐에서 실행 되었다는 정보 저장
                            table[carry-1][time_s] = 1;
                        }
                        // q2가 비었고 q1에 원소가 있는 경우
                        else if (EmptyCheck(&ready_q2) && (!EmptyCheck(&ready_q1))){
                            int carry = Dequeue(&ready_q1);
                            Enqueue(&running_q, carry);
                            p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
                            p[carry-1].run_tq = (int)pow(mlfq_time, 1);
                            p[carry-1].Qfrom = 1;   // 1단계 큐에서 실행 되었다는 정보 저장
                            table[carry-1][time_s] = 1;
                        }
                        // q2, q1이 비었고 q0에 원소가 있는 경우
                        else if (EmptyCheck(&ready_q2) && EmptyCheck(&ready_q1) && (!EmptyCheck(&ready_q0))){
                            int carry = Dequeue(&ready_q0);
                            Enqueue(&running_q, carry);
                            p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
                            p[carry-1].run_tq = (int)pow(mlfq_time, 2);
                            p[carry-1].Qfrom = 0;   // 0단계 큐에서 실행 되었다는 정보 저장
                            table[carry-1][time_s] = 1;
                        }
                    }
                }
                // 제공된 시간만큼 사용하지 못한 경우 다시 러닝 시켜준다.
                else if (p[run_pid-1].run_tq > 0) {
                    Enqueue(&running_q, p[run_pid-1].pid);
                    p[run_pid-1].state = "running";
                    table[run_pid-1][time_s] = 1;
                }
            }
            // service_t을 모두 사용한 경우 terminated
            else if (p[run_pid-1].service_t == 0){
                p[run_pid-1].state = "terminated";
                if (!EmptyCheck(&ready_q2)){
                    int carry = Dequeue(&ready_q2);
                    Enqueue(&running_q, carry);
                    p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
                    p[carry-1].run_tq = (int)pow(mlfq_time, 0);
                    p[carry-1].Qfrom = 2;   // 2단계 큐에서 실행 되었다는 정보 저장
                    table[carry-1][time_s] = 1;
                }
                // q2가 비었고 q1에 원소가 있는 경우
                else if (EmptyCheck(&ready_q2) && (!EmptyCheck(&ready_q1))){
                    int carry = Dequeue(&ready_q1);
                    Enqueue(&running_q, carry);
                    p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
                    p[carry-1].run_tq = (int)pow(mlfq_time, 1);
                    p[carry-1].Qfrom = 1;   // 1단계 큐에서 실행 되었다는 정보 저장
                    table[carry-1][time_s] = 1;
                }
                // q2, q1이 비었고 q0에 원소가 있는 경우
                else if (EmptyCheck(&ready_q2) && EmptyCheck(&ready_q1) && (!EmptyCheck(&ready_q0))){
                    int carry = Dequeue(&ready_q0);
                    Enqueue(&running_q, carry);
                    p[carry-1].state = "running";     // running상태로 프로세스 스케줄링
                    p[carry-1].run_tq = (int)pow(mlfq_time, 2);
                    p[carry-1].Qfrom = 0;   // 0단계 큐에서 실행 되었다는 정보 저장
                    table[carry-1][time_s] = 1;
                }
            }
        }
    }
    display(table, n, service_sum);
}

// Lottery
void Lottery (t_pro *tp, int n){
    int i, m, l, service_sum = 0;
    int time_s = 0; // 매 초 카운트할 변수
    int sumOfTicket = 0;    // 총 티켓 수를 보관할 곳
    ReadyQueue ready_q;   // 한 개의 레디 큐 생성
    InitializeQ(&ready_q, n);  // 레디 큐는 n개 만큼 크기 규정
    t_pro p[n];
    
    // copy Struct process
    for(i=0; i<n; i++){
        p[i].p_name = tp[i].p_name;
        p[i].arr_t = tp[i].arr_t;
        p[i].service_t = tp[i].service_t;
    }
    
    // 총 서비스 시간의 합 구하기
    for(i=0; i<n; i++){
        service_sum += p[i].service_t;
    }
    
    // 표로 출력하기 위한 2차원 배열 동적 할당 (0으로 초기화)
    int** table = (int**)calloc(n,sizeof(int*));
    for(i=0; i<n; i++){
        table[i] = (int*)calloc(service_sum,sizeof(int));
    }
    
    for(time_s=0; time_s<=service_sum; time_s++){
        for(m = 0; m<n; m++){
            if(p[m].arr_t == time_s){
                p[m].pid = p[m].p_name - 64;
                p[m].ticket = p[m].service_t;   // 각 프로세스의 티켓은 서비스 타임과 비례하게 부여
                Enqueue(&ready_q, p[m].pid);
            }
        }
        
        // 각 티켓 구하기
        for (l = 0; l < ready_q.count; l++) {
            int k = Dequeue(&ready_q);
            sumOfTicket += p[k-1].ticket;
            Enqueue(&ready_q, k);
        }
        
        srand((unsigned int)time(NULL));
        int randNum = rand()%sumOfTicket;
        
        // ready_q에 원소가 있는 경우
        if(!EmptyCheck(&ready_q)){
            int currentValue = 0;
            while(1){
                int select = Dequeue(&ready_q);
                currentValue += p[select - 1].ticket;
                if (currentValue > randNum){
                    table[select-1][time_s] = 1;
                    p[select - 1].service_t--;  // decrease 1 service_time
                    if(p[select-1].service_t > 0){
                        Enqueue(&ready_q, select);  // 서비스 타임이 남아있다면 다시 ready_q로 삽입
                    } else {
                        p[select-1].state = "terminated";   // 서비스 타임이 남아있지 않다면 terminated.
                    }
                    break;
                } else {
                    Enqueue(&ready_q, select);  // 당첨되지 않은 프로세스는 다시 레디 큐로 삽입
                }
            }
        }
    }
    display (table, n, service_sum);
}