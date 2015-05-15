#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int read_line(int sock, char buffer[]){
    int length = 0;
    while (1){
        char data;
        int result = recv(sock, &data, 1, 0);
        if ((result <= 0) || (data == EOF)){
            perror("Connection closed");
            exit(1);
        }
        buffer[length] = data;
        length++;
        if (length >= 2 && buffer[length-2] == '\r' && buffer[length-1] == '\n'){
            buffer[length-2] = '\0';
            return length;
        }
    }
}

void log_with_date(char line[]){
    char date[50];
    struct tm *current_time;

    time_t now = time(0);
    current_time = gmtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", current_time);

    printf("[%s] %s\n", date, line);
}

void log_to_file(char line[], FILE *logfile){
    char date[50];
    struct tm *current_time;

    time_t now = time(0);
    current_time = gmtime(&now);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", current_time);

    fprintf(logfile, "[%s] %s\n", date, line);
    fflush(logfile);
}

char *get_config(char name[]){
    char *value = malloc(1024);
    FILE *configfile = fopen("config.txt", "r");
    value[0] = '\0';
    if (configfile != NULL){
        while (1){
            char configname[1024];
            char tempvalue[1024];
            int status = fscanf(configfile, " %1023[^= ] = %s ", configname, tempvalue); //Parse key=value
            if (status == EOF){
                break;
            }
            if (strcmp(configname, name) == 0){
                strncpy(value, tempvalue, strlen(tempvalue)+1);
                break;
            }
        }
        fclose(configfile);
    }
    return value;
}

char *get_prefix(char line[]){
    char *prefix = malloc(512);
    char clone[512];
    strncpy(clone, line, strlen(line)+1);
    if (line[0] == ':'){
        char *splitted = strtok(clone, " ");
        if (splitted != NULL){
            strncpy(prefix, splitted+1, strlen(splitted)+1);
        }else{
            prefix[0] = '\0';
        }
    }else{
        prefix[0] = '\0';
    }
    return prefix;
}

char *get_username(char line[]){
    char *username = malloc(512);
    char clone[512];
    strncpy(clone, line, strlen(line)+1);
    if (strchr(clone, '!') != NULL){
        char *splitted = strtok(clone, "!");
        if (splitted != NULL){
            strncpy(username, splitted+1, strlen(splitted)+1);
        }else{
            username[0] = '\0';
        }
    }else{
        username[0] = '\0';
    }
    return username;
}

char *get_command(char line[]){
    char *command = malloc(512);
    char clone[512];
    strncpy(clone, line, strlen(line)+1);
    char *splitted = strtok(clone, " ");
    if (splitted != NULL){
        if (splitted[0] == ':'){
            splitted = strtok(NULL, " ");
        }
        if (splitted != NULL){
            strncpy(command, splitted, strlen(splitted)+1);
        }else{
            command[0] = '\0';
        }
    }else{
        command[0] = '\0';
    }
    return command;
}

char *get_last_argument(char line[]){
    char *argument = malloc(512);
    char clone[512];
    strncpy(clone, line, strlen(line)+1);
    char *splitted = strstr(clone, " :");
    if (splitted != NULL){
        strncpy(argument, splitted+2, strlen(splitted)+1);
    }else{
        argument[0] = '\0';
    }
    return argument;
}

char *get_argument(char line[], int argno){
    char *argument = malloc(512);
    char clone[512];
    strncpy(clone, line, strlen(line)+1);
    
    int current_arg = 0;
    char *splitted = strtok(clone, " ");
    while (splitted != NULL){
        if (splitted[0] != ':'){
            current_arg++;
        }
        if (current_arg == argno+1){
            strncpy(argument, splitted, strlen(splitted)+1);
            return argument;
        }
        splitted = strtok(NULL, " ");
    }
    
    if (current_arg != argno){
        argument[0] = '\0';
    }
    return argument;
}

void set_nick(int sock, char nick[]){
    char nick_packet[512];
    sprintf(nick_packet, "NICK %s\r\n", nick);
    send(sock, nick_packet, strlen(nick_packet), 0);
}

void send_user_packet(int sock, char nick[]){
    char user_packet[512];
    sprintf(user_packet, "USER %s 0 * :%s\r\n", nick, nick);
    send(sock, user_packet, strlen(user_packet), 0);
}

void join_channel(int sock, char channel[]){
    char join_packet[512];
    sprintf(join_packet, "JOIN %s\r\n", channel);
    send(sock, join_packet, strlen(join_packet), 0);
}

void send_pong(int sock, char argument[]){
    char pong_packet[512];
    sprintf(pong_packet, "PONG :%s\r\n", argument);
    send(sock, pong_packet, strlen(pong_packet), 0);
}

void send_message(int sock, char to[], char message[]){
    char message_packet[512];
    sprintf(message_packet, "PRIVMSG %s :%s\r\n", to, message);
    send(sock, message_packet, strlen(message_packet), 0);
}

int main() {
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1){
       perror("Could not create socket");
       exit(1);
    }
    
    char *ip = get_config("server");
    char *port = get_config("port");

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));

    free(ip);
    free(port);

    if (connect(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Could not connect");
        exit(1);
    }
    
    char *nick = get_config("nick");
    char *channels = get_config("channels");

    set_nick(socket_desc, nick);
    send_user_packet(socket_desc, nick);
    join_channel(socket_desc, channels);

    free(nick);
    free(channels);

    FILE *logfile = fopen("bot.log.txt", "a+");

    while (1){
        char line[512];
        read_line(socket_desc, line);
        
        char *prefix = get_prefix(line);
        char *username = get_username(line);
        char *command = get_command(line);
        char *argument = get_last_argument(line);

        if (strcmp(command, "PING") == 0){
            send_pong(socket_desc, argument);
            log_with_date("Got ping. Replying with pong...");
        }else if (strcmp(command, "PRIVMSG") == 0){
            char logline[512];
            char *channel = get_argument(line, 1);

            sprintf(logline, "%s/%s: %s", channel, username, argument);
            log_with_date(logline);

            char filename[500];
            sprintf(filename, "%s.log.txt", channel);
            freopen(filename, "a+", logfile);
            log_to_file(logline, logfile);
            free(channel);
        }else if (strcmp(command, "JOIN") == 0){
            char logline[512];
            char *channel = get_argument(line, 1);
            sprintf(logline, "%s joined %s.", username, channel);
            log_with_date(logline);
            
            char filename[500];
            sprintf(filename, "%s.log.txt", channel);
            freopen(filename, "a+", logfile);
            log_to_file(logline, logfile);
            free(channel);
        }else if (strcmp(command, "PART") == 0){
            char logline[512];
            char *channel = get_argument(line, 1);
            sprintf(logline, "%s left %s.", username, channel);
            log_with_date(logline);
            
            char filename[500];
            sprintf(filename, "%s.log.txt", channel);
            freopen(filename, "a+", logfile);
            log_to_file(logline, logfile);
            free(channel);
        }

        free(prefix);
        free(username);
        free(command);
        free(argument);
    }
}
