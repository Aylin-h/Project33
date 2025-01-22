#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

// פונקציה לטיפול בתשובה מהשרת
void handle_response(int sig) {
    // קריאת התוצאה מהקובץ
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

    // מחיקת הקובץ
    unlink(filename);

    printf("Result received from server: %d\n", result);
    exit(0);  // סיום הלקוח
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("EX_FROM_ERROR\n");
        return 1;
    }

    // פרמטרים משורת הפקודה
    int server_pid = atoi(argv[1]); // PID של השרת
    int num1 = atoi(argv[2]);       // המספר הראשון
    int operation = atoi(argv[3]); // פעולה
    int num2 = atoi(argv[4]);       // המספר השני

    // כתיבה לקובץ `srv_to`
    FILE* file = fopen("srv_to", "w");
    if (!file) {
        printf("EX_FROM_ERROR\n");
        return 1;
    }
    fprintf(file, "%d %d %d %d\n", getpid(), num1, operation, num2);
    fclose(file);

    // שליחת סיגנל לשרת
    kill(server_pid, SIGUSR1);

    // המתנה לתשובה
    signal(SIGUSR1, handle_response);
    alarm(30);  // טיימר ל-30 שניות
    pause();

    // אם לא התקבלה תשובה לאחר 30 שניות
    printf("Client timeout: No response from server for 30 seconds.\n");
    return 1;
}
