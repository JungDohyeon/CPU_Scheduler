/*
*	DKU Operating System Lab
*	    Lab1 (Scheduler Algorithm Simulator)
*	    Student id : 32184064
*	    Student name : Jung Dohyeon
*
*   lab1_sched.c :
*       - Lab1 source file.
*       - Must contains scueduler algorithm test code.
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

#include "lab1_sched_types.h"

int main(int argc, char *argv[]){
    int n, i;
    
    printf("======== DKU 2022 Operating System LAB. 1 ========\n");
    printf("    NAME: JUNG DO HYEON, Student ID: 32184064\n");
    printf("\n");
    printf("사용할 프로세스 개수를 입력하시오: ");
    scanf("%d", &n); // 전체 프로세스 개수 입력 창
    t_pro p[n]; // n개의 프로세스 생성
    
    for(i = 0; i<n; i++){
        printf("enter %d process name, arrival time and Service time : ", i+1);
        scanf("%s %d %d",&p[i].p_name, &p[i].arr_t, &p[i].service_t);
    }   // 프로세스 각 내용 입력 받기!
    
    sort_arr(p, n); // 도착 시간 순으로 정렬
    printf("================= 1. FCFS =================\n");
    fcfs(p,n);  // fcfs 알고리즘 실행
    printf("\n");
    printf("=============== 2. RR (q = 1) ===============\n");
    RR(p,n,1);
    printf("\n");
    printf("=============== 3. RR (q = 4) ===============\n");
    RR(p,n,4);
    printf("\n");
    printf("=============== 4. SPN (SJF) ===============\n");
    SPN(p,n);
    printf("\n");
    printf("================== 5. SRT ==================\n");
    SRT(p,n);
    printf("\n");
    printf("================== 6. HRRN ==================\n");
    HRRN(p,n);
    printf("\n");
    printf("=============== 7. MLFQ q = 1 ===============\n");
    MLFQ(p, n, 1);
    printf("\n");
    printf("============== 8. MLFQ q = 2^i ==============\n");
    MLFQ(p, n, 2);
    printf("\n");
    printf("======== BONUS : Lottery scheduling! ========\n");
    Lottery(p,n);
}

