#include <strings.h>
#include "processManagement_lab.h"

/**
 * The task function to simulate "work" for each worker process
 * TODO#3: Modify the function to be multiprocess-safe 
 * */
void task(long duration)
{
    // simulate computation for x number of seconds
    usleep(duration*TIME_MULTIPLIER);

    // TODO: protect the access of shared variable below
    // update global variables to simulate statistics
    ShmPTR_global_data->sum_work += duration;
    ShmPTR_global_data->total_tasks ++;
    if (duration % 2 == 1) {
        ShmPTR_global_data->odd++;
    }
    if (duration < ShmPTR_global_data->min) {
        ShmPTR_global_data->min = duration;
    }
    if (duration > ShmPTR_global_data->max) {
        ShmPTR_global_data->max = duration;
    }
}


/**
 * The function that is executed by each worker process to execute any available job given by the main process
 * */
void job_dispatch(int i){

    // TODO#3:  a. Always check the corresponding shmPTR_jobs_buffer[i] for new  jobs from the main process
    //          b. Use semaphore so that you don't busy wait
    //          c. If there's new job, execute the job accordingly: either by calling task(), usleep, exit(3) or kill(getpid(), SIGKILL)
    //          d. Loop back to check for new job 


    printf("Hello from child %d with pid %d and parent id %d\n", i, getpid(), getppid());
    exit(0); 

}

/** 
 * Setup function to create shared mems and semaphores
 * **/
void setup(){

    // TODO#1:  a. Create shared memory for global_data struct (see processManagement_lab.h)
    ShmID_global_data = shmget(IPC_PRIVATE, sizeof(global_data), IPC_CREAT | 0666);
    isSharedMemFail(ShmID_global_data);

    //          b. When shared memory is successfully created, set the initial values of "max" and "min" of the global_data struct in the shared memory accordingly
    // To bring you up to speed, (a) and (b) are given to you already. Please study how it works. 
    ShmPTR_global_data = (global_data *) shmat(ShmID_global_data, NULL, 0);
    isMemAttachFail((int) ShmPTR_global_data);

    //          c. Create semaphore of value 1 which purpose is to protect this global_data struct in shared memory
    while ((sem_global_data = sem_open("semglobaldata", O_CREAT,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
            1)) == SEM_FAILED)
    {
        printSemFail();
        sem_unlink("semglobaldata");
    }

    //          d. Create shared memory for number_of_processes job struct (see processManagement_lab.h)
    ShmID_jobs = shmget(IPC_PRIVATE, sizeof(job), IPC_CREAT | 0666);
    isSharedMemFail(ShmID_jobs);

    //          e. When shared memory is successfully created, setup the content of the structs (see handout)
    shmPTR_jobs_buffer = (job *) shmat(ShmID_jobs, NULL, 0);
    isMemAttachFail((int) shmPTR_jobs_buffer);

    //          f. Create number_of_processes semaphores of value 0 each to protect each job struct in the shared memory. Store the returned pointer by sem_open in sem_jobs_buffer[i]
    char semjobsi[] = "semjobsi";
    for (int i = 0; i < number_of_processes; i++) {
        semjobsi[7] = i + '0';
        while ((sem_jobs_buffer[i] = sem_open(semjobsi, O_CREAT,
                                              S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                                              0)) == SEM_FAILED)
        {
            printSemFail();
            sem_unlink(semjobsi);
        }
    }

    //          g. Return to main

    //set global data min and max
    ShmPTR_global_data->max = -1;
    ShmPTR_global_data->min = INT_MAX;
    
    return;

}

void printSemFail()
{
    printf("Failed to initialize semaphore\n");
}

void isSharedMemFail(int id)
{
    if (id == -1)
    {
        printf("Failed to create shared memory\n");
        exit(EXIT_FAILURE);
    }
}

void isMemAttachFail(int id)
{
    if (id == -1)
    {
        perror("Fail to attach the memory to this address space\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * Function to spawn all required children processes
 **/
 
void createchildren(){
    // TODO#2:  a. Create number_of_processes children processes
    //          d. For the parent process, continue creating the next children
    //          e. After number_of_processes children are created, return to main
    pid_t pid;

    for (int i = 0; i < number_of_processes; ++i) {
        switch (pid = fork()) {
            case -1:
                fprintf(stderr, "Fork has failed. Exiting now");
                exit(1); // exit error
            case 0:
                //          b. Store the pid_t of children i at children_processes[i]
                children_processes[i] = pid;
                //          c. For child process, invoke the method job_dispatch(i)
                job_dispatch(i);
                printf("Hello from child %d with pid %d and parent id %d", i, pid, children_processes[0]);
                exit(0);
            default:
                children_processes[0] = pid;
        }
    }

    return;
}

/**
 * The function where the main process loops and busy wait to dispatch job in available slots
 * */
void main_loop(char* fileName){

    // load jobs and add them to the shared memory
    FILE* opened_file = fopen(fileName, "r");
    char action; //stores whether its a 'p' or 'w'
    long num; //stores the argument of the job 

    while (fscanf(opened_file, "%c %ld\n", &action, &num) == 2) { //while the file still has input

        //TODO#4: create job, busy wait
        //      a. Busy wait and examine each shmPTR_jobs_buffer[i] for jobs that are done by checking that shmPTR_jobs_buffer[i].task_status == 0. You also need to ensure that the process i IS alive using waitpid(children_processes[i], NULL, WNOHANG). This WNOHANG option will not cause main process to block when the child is still alive. waitpid will return 0 if the child is still alive. 
        //      b. If both conditions in (a) is satisfied update the contents of shmPTR_jobs_buffer[i], and increase the semaphore using sem_post(sem_jobs_buffer[i])
        //      c. Break of busy wait loop, advance to the next task on file 
        //      d. Otherwise if process i is prematurely terminated, revive it. You are free to design any mechanism you want. The easiest way is to always spawn a new process using fork(), direct the children to job_dispatch(i) function. Then, update the shmPTR_jobs_buffer[i] for this process. Afterwards, don't forget to do sem_post as well 
        //      e. The outermost while loop will keep doing this until there's no more content in the input file. 



    }
    fclose(opened_file);

    printf("Main process is going to send termination signals\n");

    // TODO#4: Design a way to send termination jobs to ALL worker that are currently alive 



    //wait for all children processes to properly execute the 'z' termination jobs
    int process_waited_final = 0;
    pid_t wpid;
    while ((wpid = wait(NULL)) > 0){
        process_waited_final ++;
    }
    
    // print final results
    printf("Final results: sum -- %ld, odd -- %ld, min -- %ld, max -- %ld, total task -- %ld\n", ShmPTR_global_data->sum_work, ShmPTR_global_data->odd, ShmPTR_global_data->min, ShmPTR_global_data->max, ShmPTR_global_data->total_tasks);
}

void cleanup(){
    //TODO#4: 
    // 1. Detach both shared memory (global_data and jobs)
    // 2. Delete both shared memory (global_data and jobs)
    // 3. Unlink all semaphores in sem_jobs_buffer
}

// Real main
int main(int argc, char* argv[]){

    //Check and parse command line options to be in the right format
    if (argc < 2) {
        printf("Usage: sum <infile> <numprocs>\n");
        exit(EXIT_FAILURE);
    }

    //Limit number_of_processes into 10.
    //If there's no third argument, set the default number_of_processes into 1.
    if (argc < 3){
        number_of_processes = 1;
    }
    else{
        if (atoi(argv[2]) < MAX_PROCESS) number_of_processes = atoi(argv[2]);
        else number_of_processes = MAX_PROCESS;
    }

    printf("Number of processes: %d\n", number_of_processes);
    printf("Main process pid %d \n", getpid());

    createchildren();

    for (int i = 0; i<number_of_processes; i++){
        printf("Child process %d created with pid: %d \n", i, children_processes[i]);
        wait(NULL);
    }


    printf("success\n");
    return 0;
}