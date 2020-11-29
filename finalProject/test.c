#include <stdio.h>

#include <sys/types.h>

#include <sched.h>



int main()

{

struct sched_param param; 

struct sched_param new_param;

   printf("start policy = %d \n", sched_getscheduler(0)); // 현재 프로세스의 스케줄러 방식을 가져온다.

// 0 - SCHED_OTHER, 1 - SCHED_FIFO, 2 - SCHED_RR





   param.sched_priority = sched_get_priority_min(SCHED_RR); //(sched_get_priority_min(SCHED_RR) + sched_get_priority_max(SCHED_RR)/50);  ---> priority 값 설정.

   printf("max priority = %d, min priority = %d, my priority = %d \n", sched_get_priority_max(SCHED_RR), sched_get_priority_min(SCHED_RR),

                   param.sched_priority);



   if(sched_setscheduler(0, SCHED_RR, &param) != 0){   // SCHED_RR 로 스케줄러 type 변경. 기본은 SCHED_OHTER.

           perror("sched_setscheduler failed \n");

           return -1;

   }



return 0;

}
