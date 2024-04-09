#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main() {

    //User Input decides how many child processes 
    int NUM_CHILDREN;
    printf("Enter the number of child process: ");
    scanf("%d", &NUM_CHILDREN);

    //Open s the file for testing
    FILE *file;
    char filename[] = "file2.dat"; //file name
    char *line = NULL; // Pointer to store each line
    size_t len = 0; 
    int total = 0;
   
   //time progress for the file
    clock_t start,end ;
    start =clock();

    // Opens file
    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file.\n");
        return 1;
    }

    //scans lines, and creates a count
    int number;
    int total_numbers = 0;
    while (fscanf(file, "%d", &number) != EOF){
        total_numbers++;
    }
    rewind(file);//Pointer back at Beginning


    //divides the number of lines to blocks; depending on Children
    int block_size = total_numbers / NUM_CHILDREN;

    //creates the pipes between
    int pipes[NUM_CHILDREN][2];//Pipes
    for (int i = 0; i < NUM_CHILDREN; i++)
    {
        if(pipe(pipes[i]) == -1){
            perror("pipe");
            return 1;
        }
    }
    
    //child process
    for (int i = 0; i < NUM_CHILDREN; i++)
    {
        pid_t pid = fork();
        if(pid == -1){
            perror("fork");
            return 1;
        } else if (pid == 0){
            close(pipes[i][1]);
            int sum = 0;
            int num;
            //read sum of the block 
            while(read(pipes[i][0], &num, sizeof(int)) > 0){
                sum += num;
            }
            //prints the sum
            printf("Child %d: Sum of block = %d\n", getpid(), sum);

            //writes the sum to the pipe for the parent to read and add
            write(pipes[i][1], &sum, sizeof(int));
            close(pipes[i][0]);
            close(pipes[i][1]);
            exit(0);
        } else { // parent process
            close(pipes[i][0]);
        //sends block/chunck(lines) to child process
        //reads each line as a integer, saved to number
            int start_index = i * block_size;
            int end_index = (i== NUM_CHILDREN - 1) ? total_numbers : start_index+ block_size;
            for (int j = start_index; j < end_index; j++){
                fscanf(file, "%d", &number);
                write(pipes[i][1], &number, sizeof(int));
            }
            close(pipes[i][1]);
        }
    }
    
    //waits for child and parent to sync, allowing children to finish
    for(int i = 0; i< NUM_CHILDREN; i++){
        wait(NULL);
    }

    //should send all results from children processes
    int total_sum = 0;
    for (int i = 0; i < NUM_CHILDREN; i++){
        int child_sum;
        read(pipes[i][0], &child_sum, sizeof(int));
        total_sum += child_sum;
        close(pipes[i][0]);
    }
    //final total
    printf("Total sum of numbers: %d\n", total_sum);

    //time tracker
    end = clock();
    double time_taken = ((double)(end - start))/ CLOCKS_PER_SEC;
    printf("Time taken: %f seconds\n", time_taken);
    fclose(file);    

    return 0;
}