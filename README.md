# 50005Lab1: Process Management

vars: bool assigned

The bool is used as a flag to set the task being assigned and not assigned. At the initial state of every while loop, 
the assigned var is set as false. And it will be assigned true when the task status is 0 (no job or job cleared) and 
when waitpid is 0 (child process is available). The other case in which assigned is set true is when a new process is
forked() (status == 9).
The update function is called to update the counter (i), action and num, and calls the sem_post function.

To signal the termination process, a loop iterates through the process and checks the waitpid is 0, and if there are no
tasks done by the child_process, we proceed to call the update function and updates the shmPTR_jobs_buffer[i] with
action with 'z' and number 0 to signal the process to be killed.

-- Done by: Tan Li Yuan | 1004326 | CI04 --
