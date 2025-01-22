#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

void reset_timer() {
    alarm(60);
}

void handle_timeout(int sig) {
    printf("Server timeout: No requests received in the last 60 seconds. Closing server.\n");
    exit(0);
}

void handle_signal(int sig, siginfo_t* info, void* context) {
    pid_t client_pid = info->si_pid;
    pid_t pid = fork();
    if (pid == 0) {
        FILE* file = fopen("srv_to", "r");
        if (!file) {
            printf("Error: srv_to file not found.\n");
            exit(1);
        }

        int client_pid_file, num1, num2, operation;
        fscanf(file, "%d %d %d %d", &client_pid_file, &num1, &operation, &num2);
        fclose(file);
        unlink("srv_to");

        int result;
        switch (operation) {
        case 1: result = num1 + num2; break;
        case 2: result = num1 - num2; break;
        case 3: result = num1 * num2; break;
        case 4:
            if (num2 == 0) {
                printf("Division by zero error\n");
                exit(1);
            }
            result = num1 / num2;
            break;
        default:
            printf("Invalid operation\n");
            exit(1);
        }

        char filename[50];
        sprintf(filename, "%d_client_to", client_pid_file);
        FILE* result_file = fopen(filename, "w");
        if (!result_file) {
            printf("Error creating result file\n");
            exit(1);
        }
        fprintf(result_file, "%d\n", result);
        fclose(result_file);
        kill(client_pid_file, SIGUSR1);
        exit(0);
    }
    else {
        reset_timer();
    }
}

void start_server() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_signal;
    sigaction(SIGUSR1, &sa, NULL);
    signal(SIGALRM, handle_timeout);
    reset_timer();
    while (1) {
        pause();
    }
}

int main() {
    unlink("srv_to");
    start_server();
    return 0;
}
