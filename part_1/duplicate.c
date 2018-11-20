/**
*  SOFE3200 Systems Programming
*  Assignment 2 - Part 1
*  Created by: Ethan Elliott (100622168)
*  Duplicating a file
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define READ  0
#define WRITE 1

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("Invalid Arguments!\n");
        printf("Usage: duplicate <sourcefile> <targetfile>\n");
        exit(1);
    }

    char inputFilePath[20];
    char outputFilePath[20];
    strcpy(inputFilePath, argv[1]);
    strcpy(outputFilePath, argv[2]);

    int pipeItUp[2];
    int AckPipe[2];
    pipe(pipeItUp);
    pipe(AckPipe);

    time_t start_t, check_t, mark_t;
    time(&start_t);
    time(&mark_t);

    int pid = fork();
    if (pid > 0) {  
        /* parent process */
        int ack = 1, byteBuffer;
        long long byteCount = 0;
        close(pipeItUp[WRITE]);
        close(AckPipe[READ]);
        while(1) {
            write(AckPipe[WRITE],&ack,sizeof(ack));
            if (read(pipeItUp[READ], &byteBuffer, sizeof(byteBuffer)) > 0) {
                byteCount++;
                time(&check_t);
                int diff = difftime(check_t, mark_t);
                if (diff > 0 && diff % 1 == 0) {
                    time(&mark_t);
                    printf("still duplicating...\n");
                }
            } else {
                break;
            }
        }
        time(&check_t);
        int runtime = difftime(check_t, start_t);
        printf("\nCopied %lld bytes from %s to %s in %d seconds.\n", byteCount, inputFilePath, outputFilePath, runtime);
        close(pipeItUp[READ]);
        close(AckPipe[WRITE]);
    } else { 
        int childAck = 0;
        FILE *source = fopen(inputFilePath, "rb"),
             *target = fopen(outputFilePath, "wb");
        int byteBuffer;
        close(pipeItUp[READ]);
        close(AckPipe[WRITE]);
        while(1) {
            byteBuffer = fgetc(source);
            if (!feof(source)) {
                fputc(byteBuffer, target);
            } else {
                break;
            }
            if (read(AckPipe[READ],&childAck,sizeof(childAck)) <= 0) {
                break;
            }
            write(pipeItUp[1], &byteBuffer, sizeof(byteBuffer));
        }

        fclose(source);
        fclose(target);

        // Close all pipes when done
        close(pipeItUp[WRITE]);
        close(AckPipe[READ]);
    }
    // Done
    return 0;
}