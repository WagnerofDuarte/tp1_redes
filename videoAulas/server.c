#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024

GameMessage msg;

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);

    if (strcmp(argv[1], "v4") == 0) {
        printf("Servidor iniciado em modo IPv4 na porta %s. Aguardando conexão...\n", argv[2]);
    } else {
        printf("Servidor iniciado em modo IPv6 na porta %s. Aguardando conexão...\n", argv[2]);
    }

    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }
        printf("Cliente conectado.\n");

        msg.type = MSG_REQUEST;
        strcpy(msg.message,
            "Escolha sua jogada:\n"
            "0 - Nuclear Attack\n"
            "1 - Intercept Attack\n"
            "2 - Cyber Attack\n"
            "3 - Drone Strike\n"
            "4 - Bio Attack\n"
        );
        send(csock, &msg, sizeof(msg), 0);
        printf("Apresentando as opções para o cliente.");

        /* Receber escolha do cliente */
        recv(csock, &msg, sizeof(msg), 0);
        if (msg.type != MSG_RESPONSE) {
            // Erro ou protocolo quebrado
            logexit("Tipo de mensagem inesperado");
        }
        /* Printar escolha do cliente "Cliente escolheu X." */

        /* Escolha aleatória do servidor */
        /* Printar a escolha aleatória do servidor */

        /* Chamar regras do jogo pra decidir vencedor */
        /* Atualizar placar e printar na tela o placar --> "Placar atualizado: Cliente A x B Servidor "*/

        /* Enviar pro cliente a escolha do servidor e o resultado da jogada */

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[log] connection from %s\n", caddrstr);

        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);
        size_t count = recv(csock, buf, BUFSZ - 1, 0);
        printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

        sprintf(buf, "remote endpoint: %.1000s\n", caddrstr);
        count = send(csock, buf, strlen(buf) + 1, 0);
        if (count != strlen(buf) + 1) {
            logexit("send");
        }
        close(csock);
    }

    exit(EXIT_SUCCESS);
}