#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

// פונקציה לאתחול הטיימר של השרת
void reset_timer() {
    alarm(60);  // מאתחל טיימר ל-60 שניות
}

// פונקציה לסגירת השרת במקרה של טיימאוט
void handle_timeout(int sig) {
    printf("Server timeout: No requests received in the last 60 seconds. Closing server.\n");
    exit(0);
}

// פונקציה לטיפול בסיגנלים שמגיעים מהלקוח
void handle_signal(int sig, siginfo_t* info, void* context) {
    pid_t client_pid = info->si_pid;  // מזהה הלקוח
    printf("Signal received from client PID: %d\n", client_pid);

    // יצירת תהליך ילד לטיפול בבקשה
    pid_t pid = fork();
    if (pid == 0) {
        // קוד תהליך הילד
        printf("Child process started for client PID: %d\n", client_pid);

        // קריאת נתונים מתוך `srv_to`
        FILE* file = fopen("srv_to", "r");
        if (!file) {
            printf("Error: srv_to file not found.\n");
            exit(1);
        }

        int client_pid_file, num1, num2, operation;
        fscanf(file, "%d %d %d %d", &client_pid_file, &num1, &operation, &num2);
        fclose(file);

        // מחיקת הקובץ `srv_to`
        unlink("srv_to");

        // ביצוע החישוב
        int result;
        switch (operation) {
        case 1: result = num1 + num2; break;  // חיבור
        case 2: result = num1 - num2; break;  // חיסור
        case 3: result = num1 * num2; break;  // כפל
        case 4:
            if (num2 == 0) {
                printf("Division by zero error\n");
                exit(1);
            }
            result = num1 / num2;
            break;  // חילוק
        default:
            printf("Invalid operation\n");
            exit(1);
        }

        // כתיבת התוצאה לקובץ ייחודי
        char filename[50];
        sprintf(filename, "%d_client_to", client_pid_file);
        FILE* result_file = fopen(filename, "w");
        if (!result_file) {
            printf("Error creating result file\n");
            exit(1);
        }
        fprintf(result_file, "%d\n", result);
        fclose(result_file);

        // שליחת סיגנל ללקוח שהתוצאה מוכנה
        kill(client_pid_file, SIGUSR1);

        // סיום תהליך הילד
        printf("Child process finished for client PID: %d\n", client_pid_file);
        exit(0);
    }
    else {
        // תהליך האב: ממשיך להמתין לבקשות נוספות
        reset_timer();  // מאתחל את הטיימר
    }
}

// פונקציה להתחלת השרת
void start_server() {
    printf("Server is running...\n");

    // רישום סיגנלים
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_signal;
    sigaction(SIGUSR1, &sa, NULL);  // סיגנל מהלקוח

    // רישום טיימר
    signal(SIGALRM, handle_timeout);
    reset_timer();

    // לולאה להמתנה לסיגנלים
    while (1) {
        pause();  // מחכה לסיגנלים
    }
}

int main() {
    // מחיקת קובץ זמני `srv_to` אם קיים
    unlink("srv_to");

    // התחלת השרת
    start_server();

    return 0;
}
