#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

void handle_response(int sig) {
    char filename[50];
    sprintf(filename, "%d_client_to", getpid());
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("EX_FROM_ERROR\n");
        exit(1);
    }

    int result;
    fscanf(file, "%d", &result);
    fclose(file);
    unlink(filename);
    printf("Result received from server: %d\n", result);
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("EX_FROM_ERROR\n");
        return 1;
    }

    int server_pid = atoi(argv[1]);
    int num1 = atoi(argv[2]);
    int operation = atoi(argv[3]);
    int num2 = atoi(argv[4]);

    FILE* file = fopen("srv_to", "w");
    if (!file) {
        printf("EX_FROM_ERROR\n");
        return 1;
    }
    fprintf(file, "%d %d %d %d\n", getpid(), num1, operation, num2);
    fclose(file);
    kill(server_pid, SIGUSR1);
    signal(SIGUSR1, handle_response);
    alarm(30);
    pause();
    printf("Client timeout: No response from server for 30 seconds.\n");
    return 1;
}
