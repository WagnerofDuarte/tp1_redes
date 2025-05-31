#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024

GameMessage msg;
int csock;
int startNewConection = 1;

void printReplayChoice(int choice) {
    switch (choice) {
        case 0:
            printf("Cliente não deseja jogar novamente.");
            break;
        case 1: 
            printf("Cliente deseja jogar novamente.");
            break;
        default:
            // ERRO
            break;
    }
}

void sendRequestMsg() {
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
}

void recieveClientMsgResponse() {
    recv(csock, &msg, sizeof(msg), 0); // Recebe a jogada do cliente
    if (msg.type == MSG_RESPONSE) {
        printf("Cliente escolheu %d", msg.client_action);
    } else {
        // Erro ou protocolo quebrado
        logexit("Tipo de mensagem inesperado");
    }
}

int makeServerPlay() {
    srand(time(NULL)); // Inicializa a semente do gerador (só uma vez no programa)
    msg.server_action = rand() % 5;
    printf("Servidor escolheu aleatoriamente %d.", msg.server_action);

    int result = jokenBoomLogic(msg.client_action, msg.server_action);
    msg.result = result;
    switch (result) {
        case 1:
            msg.client_wins++;
            break;
        case 0:
            msg.server_wins++;
            break;
        default:
            break;
    }

    printf("Placar atualizado: Cliente %d x %d Servidor", msg.client_wins, msg.server_wins);
}

void sendGameResults() {
    msg.type = MSG_RESULT;
    send(csock, &msg, sizeof(msg), 0);
}

void sendPlayAgainRequest() {
    msg.type = MSG_PLAY_AGAIN_REQUEST;
    strcpy(msg.message,
        "Deseja jogar novamente?\n"
        "1 - Sim\n"
        "0 - Não\n"
    );
    send(csock, &msg, sizeof(msg), 0);
    printf("Perguntando se o cliente deseja jogar novamente.");
}

void recievePlayAgainResponse() {
    recv(csock, &msg, sizeof(msg), 0); // Recebe a resposta do cliente
    if (msg.type == MSG_PLAY_AGAIN_RESPONSE) {
        // OK
    } else {
        // Erro ou protocolo quebrado
        logexit("Tipo de mensagem inesperado");
    }
}

void sendFinalResults() {
    msg.type = MSG_END;
    snprintf(msg.message, MSG_SIZE,
         "Fim de Jogo!\n"
         "Placar final: Você %d x %d Servidor\n"
         "Obrigado por jogar!\n",
         msg.client_wins, msg.server_wins);
    send(csock, &msg, sizeof(msg), 0);
    printf("Enviando placar final.");
}

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

    while (1) { // Trocar por --> while msg.type != MSG_END
        
        if(startNewConection == 1) {
            struct sockaddr_storage cstorage;
            struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
            socklen_t caddrlen = sizeof(cstorage);

            csock = accept(s, caddr, &caddrlen);
            if (csock == -1) {
                logexit("accept");
            }
            printf("Cliente conectado.\n");
            startNewConection = 0;
        }

        sendRequestMsg(); // Envia requisição pro user jogar

        recieveClientMsgResponse(); // Recebe a jogada do user

        makeServerPlay(); // Fazer jogada

        sendGameResults(); // Enviar resultado pro cliente

        sendPlayAgainRequest();

        recievePlayAgainResponse();
        printReplayChoice(msg.client_action);

        switch (msg.client_action) {
            case 1:
                /* Jogar novamente */
                break;
            case 0:
                /* Sair do jogo e encerrar conexão */
                startNewConection = 1;
                sendFinalResults();
                printf("Encerrando conexão.");
                close(csock);
                printf("Cliente desconectado.");
                // Encerrar conexão
                break;
            default:
                msg.type = MSG_ERROR;
                strcpy(msg.message, "Por favor, digite 1 para jogar novamente ou 0 para encerrar.");
                send(csock, &msg, sizeof(msg), 0);
                printf("Erro: resposta inválida para jogar novamente.");
                sendPlayAgainRequest();
                break;
        }
    }
}