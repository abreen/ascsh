/*
 * ascsh.c
 * ascsh, the ASC shell
 *
 * The ASC shell allows a user to connect to an ASC daemon
 * and interact with it manually (e.g., to quit the daemon
 * or store learning data without running asc).
 */

#include <stdio.h>
#include <string.h>
#include <readline/readline.h>

#include "libascd.h"

#define PROMPT "> "


void help()
{
    printf("exit - disconnect from the daemon and exit the shell\n");
    printf("quit - instruct the daemon to shut down\n");
    printf("lookup <prog_name> - get learning data for a program\n");
}

void prompt(struct ascd_conn *conn)
{
    char *input;
    char *program_name;
    struct learning_data *data;
    struct program p;

    rl_bind_key('\t', rl_complete);

    while (1) {
        if (conn->status == DISCONNECTED) {
            printf("connection status changed to: disconnected\n");
            return;
        } else if (conn->status == FAILED) {
            printf("connection status changed to: error\n");
            perror("error");
            return;
        }

        input = readline(PROMPT);

        if (!input) {
            printf("\n");
            break;
        }

        add_history(input);

        if (strcasecmp(input, "exit") == 0) {
            free(input);
            return;

        } else if (strcasecmp(input, "help") == 0) {
            help();

        } else if (strcasecmp(input, "quit") == 0) {
            printf("sending quit message... ");

            if (ascd_quit(conn) < 0) {
                printf("failed\n");
                fprintf(stderr, "failed to quit daemon\n");
                perror("error");
            } else {
                printf("sent\n");
            }

        } else if (strcasestr(input, "lookup") == input) {
            program_name = "test";
            strcpy(p.name, program_name);
            p.any_regime = true;

            data = ascd_query(conn, &p);

            if (data == NULL) {
                printf("no learning data for this program\n");
            } else {
                printf("got learning data for this program\n");
                printf("regime dimension: %lu\n", data->regime.dim);
            }

            free(data);

        } else {
            printf("unrecognized command (try 'help' for help)\n");
        }

        free(input);
    }
}

int main(int argc, char *argv[])
{
    struct ascd_conn conn;
    char *sock_path;

    if (argc > 1) {
        sock_path = argv[1];
    } else {
        printf("no socket specified; using ./ascd.sock\n");
        sock_path = "./ascd.sock";
    }

    conn = ascd_connect(sock_path);
    if (conn.status == FAILED) {
        fprintf(stderr, "ascsh: failed to connect to daemon\n");
        perror("ascsh");
        return 1;
    } else {
        printf("connected to ascd socket '%s'\n", sock_path);
    }

    printf("to exit the shell, type 'exit' or send EOF\n");

    prompt(&conn);

    if (conn.status != DISCONNECTED) {
        if (ascd_disconnect(&conn) < 0) {
            fprintf(stderr, "ascsh: failed to disconnect\n");
            perror("ascsh");
            return 1;
        }
    }

    return 0;
}
