#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int sock;
    char name[50];
    int iswritelog;
    FILE* logfile;
    int isrunning;
    int islogon;
} ThreadData;

void* recv_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    int sock = data->sock;
    char* name = data->name;
    int iswritelog = data->iswritelog;
    FILE* logfile = data->logfile;

    char buf[1024];

    while (data->isrunning == 1) {
        int n = read(sock, buf, sizeof(buf)-1);

        if (n <= 0) {
            printf("\n[System]:Connection closed\n");
            printf("[System]: Cleaning up...\n");
            close(sock);
            data->isrunning = 0;
            if (iswritelog == 1) {
                time_t now = time(NULL);
                struct tm* t = localtime(&now);

                pthread_mutex_lock(&lock);

                fprintf(logfile, "\n[System]:Session ended at: [%02d:%02d:%02d]\n",t->tm_hour, t->tm_min, t->tm_sec);

                fflush(logfile);

                pthread_mutex_unlock(&lock);
            }
            printf("[System]:Exiting... (press enter to exit)\n");
            exit(0);
            break;
        }

        buf[n] = '\0';
        
        printf("\r\033[K");
        printf("%s\n", buf);
        printf("[Input][%s]: ", name);
        fflush(stdout);

        if (iswritelog == 1 && data->islogon == 1) {
            pthread_mutex_lock(&lock);

            fprintf(logfile, "%s\n", buf);
            fflush(logfile);

            pthread_mutex_unlock(&lock);
        }
    }

    return NULL;
}

void* send_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    int sock = data->sock;
    char* name = data->name;
    FILE* logfile = data->logfile;
    int iswritelog = data->iswritelog;

    char buf[1024];
    char message[1024];

  while (data->isrunning == 1) {

        printf("[Input][%s]: ", name);
        fflush(stdout);

        fgets(buf, sizeof(buf), stdin);
        buf[strcspn(buf, "\n")] = 0;

        if (strlen(buf) == 0) {
            continue;
        }

        if (strcmp(buf,"/disconnect") == 0) {
            printf("\n[System]:Connection closed\n");
            printf("[System]:Cleaning up...\n");
            close(sock);
            data->isrunning = 0;
            if (iswritelog == 1) {
                time_t now = time(NULL);
                struct tm* t = localtime(&now);

                pthread_mutex_lock(&lock);

                fprintf(logfile, "\n[System]:Session ended at [%02d:%02d:%02d]\n",t->tm_hour, t->tm_min, t->tm_sec);

                fflush(logfile);

                pthread_mutex_unlock(&lock);;
            }
            printf("[System]:Exiting...\n");
            exit(0);
            break;
        } else if (strcmp(buf,"/help") == 0) {
            printf("\nCommands:\n");
            printf("/help        : Show this help message\n");
            printf("/disconnect  : Close the connection\n");
            printf("/logon       : Enable message logging (requires -w option)\n");
            printf("/logoff      : Disable message logging (requires -w option)\n");
        } else if  (strcmp(buf,"/logoff") == 0) {
            if (!iswritelog) {
                printf("[Error]: This command requires -w option!\n");
            } else if (data->islogon == 0) {
                printf("[Error]: Logging already disabled!\n");
            } else {
                data->islogon = 1;
                printf("[System]: Logging disabled\n");

                time_t now = time(NULL);
                struct tm* t = localtime(&now);

                pthread_mutex_lock(&lock);

                fprintf(logfile, "\n[%02d:%02d:%02d][System]: Logging disabled by client a\n",t->tm_hour, t->tm_min, t->tm_sec);

                fflush(logfile);

                pthread_mutex_unlock(&lock);
            }
        
        } else if  (strcmp(buf,"/logon") == 0) {
            if (!iswritelog) {
                printf("[Error]: This command requires -w option!\n");
            } else if (data->islogon == 1) {
                printf("[Error]: Logging already enabled!\n");
            } else {
                data->islogon = 1;
                printf("[System]: Logging enabled\n");

                time_t now = time(NULL);
                struct tm* t = localtime(&now);

                pthread_mutex_lock(&lock);

                fprintf(logfile, "\n[%02d:%02d:%02d][System]: Logging enabled by client a\n",t->tm_hour, t->tm_min, t->tm_sec);

                fflush(logfile);

                pthread_mutex_unlock(&lock);
            }
        } else {

            time_t now = time(NULL);
            struct tm* t = localtime(&now);

            snprintf(message, sizeof(message),"[%02d:%02d:%02d][%s]: %s",t->tm_hour, t->tm_min, t->tm_sec,name, buf);

            write(sock, message, strlen(message));
            printf("\033[A");
            printf("\r");
            printf("\033[K");
            printf("%s\n", message);

            if (iswritelog == 1 && data->islogon == 1) {
                pthread_mutex_lock(&lock);

                fprintf(logfile, "%s\n", message);
                fflush(logfile);

                pthread_mutex_unlock(&lock);
            }
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {

    int iswritelog = 0;
    FILE* logfile = NULL;

    if (argc == 1) {

    } else if (argc == 2 && strncmp(argv[1], "-h", 2) == 0) {

        printf("Usage: ./client-a [option]\n\n");
        printf("-h           : help\n");
        printf("-w <file>    : logging mode\n");
        printf("\nCommands:\n");
        printf("/help        : Show this help message\n");
        printf("/disconnect  : Close the connection\n");
        printf("/logon       : Enable message logging (requires -w option)\n");
        printf("/logoff      : Disable message logging (requires -w option)\n");
        return 0;

    } else if (argc == 2 && strncmp(argv[1], "-w", 2) == 0) {

        printf("[Error]: missing file\n");
        return 1;

    } else if (argc == 3 && strncmp(argv[1], "-w", 2) == 0) {

        iswritelog = 1;

        logfile = fopen(argv[2], "a");

        if (!logfile) {
            printf("[Error]:Cannot open file\n");
            return 1;
        }

    } else {
        printf("[Error]:Invalid arguments\n");
    }
    
    char port_buf[100];
    char ip[100];
    char messagebuf[300];
    char message[400];

    printf("[System]:Please enter Client B ip: ");
    fgets(ip, sizeof(ip), stdin);
    ip[strcspn(ip, "\n")] = 0;

    printf("[System]:Please enter Client B port: ");
    fgets(port_buf, sizeof(port_buf), stdin);
    port_buf[strcspn(port_buf, "\n")] = 0;
    int port = atoi(port_buf);

    printf("\nDebug log:\n");
    printf("\n[System]:Client B port: %d \n[System]:Client B ip: %s \n", port, ip);
    printf("[System]:Creating socket...\n");
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    printf("[System]:Setting up address...\n");
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) == 1) {
        printf("[System]:Valid ip address. Continue...\n");
        inet_pton(AF_INET, ip, &addr.sin_addr);
    } else {
        printf("[Error]:Invalid ip address, please try again with valid ip address.\n");
        return(1);
    }
    printf("[System]:Connecting to client B...\n");
    alarm(15);
    printf("[System]:Set timeout to 15 seconds\n");
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    alarm(0);
    printf("[System]:Successfully connected to Client B\n[System]:Starting threads...\n");


    ThreadData* data = malloc(sizeof(ThreadData));
    data->sock = sock;
    strcpy(data->name, "Client A");
    data->logfile = logfile;
    data->iswritelog = iswritelog;
    data->isrunning = 1;
    if (data->iswritelog == 1) {
        data->islogon = 1;
    } else {
        data->islogon = 0;
    }
    
    printf("[System]:Started session\n\n\n");

    pthread_t t1, t2;

    if (iswritelog == 1) {
        time_t now = time(NULL);
        struct tm* t = localtime(&now);

        pthread_mutex_lock(&lock);

        fprintf(logfile, "\n[System]:Session started at [%02d:%02d:%02d]\n",t->tm_hour, t->tm_min, t->tm_sec);

        fflush(logfile);

        pthread_mutex_unlock(&lock);
    }

    pthread_create(&t1, NULL, recv_thread, data);
    pthread_create(&t2, NULL, send_thread, data);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    free(data);

    if (iswritelog) fclose(logfile);

    return(0);
}
