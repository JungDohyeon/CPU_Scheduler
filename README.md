# make CPU scheduler
make cpu scheduler program

## 🎯 Programming Goal 
Implementation CPU Scheduling <br>
<strong>FCFS, RR (Round-Robin), SPN, SRT, MLFQ, Lottery Scheduling (with C code) </strong>

<br>

### FCFS (FIFO)
&nbsp; The FCFS scheduling policy is <strong>non-preemptive</strong> so cannot be affected by another process until one running process finishes. Therefore, by checking the process arrival time every second, the arriving processes are put in the ready queue in the order of arrival, and then the ready queue is checked only when there is no running process according to the non-preemptive method, and the process that arrived first is moved to the running queue and executed.<br>
&nbsp; After that, if the running process is terminated, the running queue is emptied, and if there are elements left in the ready queue, it is programmed to be scheduled in the order of arrival first.

<br>

### RR (Round-Robin)
&nbsp; The RR scheduling policy is a preemptive type, and it is designed so that the user can arbitrarily assign time quantum by receiving the time quantum factor to be used at this time. <br>
&nbsp; First of all, in the scheduling method, if there is no running process, run the process that came first from the ready queue, check the time quantum given to each process every second, and if the service ends before the time quantum, run the next process, and if there is still service time I made it go to the back of the ready queue.

<br>

### SPN (Shortest Process Next) == SJF (Shortest Job First)
&nbsp; The SPN scheduling policy is a non-preemptive policy that prioritizes the process with the shortest execution time. <br>
&nbsp; According to the non-preemptive method, only when there is no running process, the processes in the ready queue are sorted as those with the shortest running time, so that the process with the shortest running time is executed first, and if there was an original running process, it is programmed to go to the back of the ready queue.

<br>

### SRT (Shortest Remaining Time)

&nbsp; The SRT scheduling policy introduces a preemptive method from the SPN scheduling technique. <br>
&nbsp; Therefore, I compared all the processes in the ready queue and the running queue every second and moved the process with the shortest remaining service time to the running queue and executed it.

<br>

### HRRN (Highest Response Ratio Next)

&nbsp; This is a technique that was developed to prevent the process from waiting for a long service time. It is a non-preemptive method in which the process with the highest response rate is scheduled first. <br>
&nbsp; Therefore, according to the non-preemptive method, only when there is no execution process, the elements in the ready queue are sorted in the order of the highest response rate, and then the first element is run. <br>
<Response rate: (service time + waiting time)/ service time>

<br>

### MLFQ (Multi Level Feedback Queue)
&nbsp; When implementing the MLFQ, the total number of queues was 3 (level 0, level 1, level 2). At this time, the time given to each level is set to increase to mlfq_time^0, mlfq_time^1, and mlfq_time^2 using the mlfq_time received as the third argument. <br>
&nbsp; In addition, the level of the queue scheduled in each level was memorized, and if all the allotted time was used, each level was lowered.

<br>

### Lottery
Tickets for each process were set in proportion to the service time of the process, the number of ready queue elements was identified every second, a random number was created after adding the number of tickets for the elements, and scheduling was performed according to the number.

<br>

## 💻 Result
### 📌Case 1)
<h3> Input </h3>

| Process_name | Arrival_time (sec) | Service_Time (sec) |
|--------------|--------------------|--------------------|
| A            | 0                  | 3                  |
| B            | 2                  | 6                  |
| C            | 4                  | 4                  |
| D            | 6                  | 5                  |
| E            | 8                  | 2                  |

<h3> Output </h3>
<table>
  <tr>
    <td><img width="50%" src="https://user-images.githubusercontent.com/40754281/227770293-872f6d1d-7e36-4eab-ae7a-7f0d3805f201.png"></td>
    <td><img width="65%" src="https://user-images.githubusercontent.com/40754281/227770317-f945d6fb-369a-4b7a-baa1-df8f60717fc7.png"></td>
  <tr>
</table>

<hr>

### 📌Case 2)
<h3> Input </h3>

| Process_name | Arrival_time (sec) | Service_Time (sec) |
|--------------|--------------------|--------------------|
| A            | 0                  | 4                  |
| B            | 1                  | 3                  |
| C            | 4                  | 6                  |
| D            | 7                  | 2                  |
| E            | 9                  | 3                  |

<h3> Output </h3>
<table>
  <tr>
    <td><img width="50%" src="https://user-images.githubusercontent.com/40754281/227770394-646ed83e-3782-4f26-88ac-156b0b6fd03f.png"></td>
    <td><img width="60%" src="https://user-images.githubusercontent.com/40754281/227770570-1c74371f-5f30-4ebb-98fe-4dfd8af22822.png"></td>
  <tr>
</table>


 
