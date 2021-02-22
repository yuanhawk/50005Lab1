# 50005Lab1: Process Management

vars: bool assigned

The bool is used as a flag to set the task being assigned and not assigned. At the initial state of every while loop, 
the assigned var is set as false. And it will be assigned true when the task status is 0 (no job or job cleared) and 
when waitpid is 0 (child process is available). And after that, the update function is called to update the counter (i),
action and num.

If the waitpid is not 0 (child process is not available), call the createchildren function to make fork process. To signal
the termination process, a loop iterates through the process and checks the waitpid is 0, and if there are no tasks
done by the child_process, we proceed to update the counter(i), action with 'z' and number 0 to signal the process to be
killed.

-- Done by: Tan Li Yuan | 1004326 | CI04 --
