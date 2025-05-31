#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/types.h>

void sendMsgAsServer(MessageType type);
void recieveMsgAsServer();
int jokenBoomMatchResult();
void makeServerPlay();
void jokenBoomLogic();
int checkForErrors();
void endGame();
void resetMsg();
const char* getMoveName(int move);
const char* getResultName(int result);

#define BUFSZ 1024

GameMessage msg;
int csock;
int startNewConection = 1;
int errorFlag = 0;

void recieveMsgAsServer() {
    MessageType nextMsgType = MSG_ERROR;
    recv(csock, &msg, sizeof(msg), 0);
    if(msg.type == MSG_RESPONSE) {
        printf("Cliente escolheu %d\n", msg.client_action);
        if(checkForErrors()) {
            sendMsgAsServer(nextMsgType);
            return;
        }
        int matchResult = jokenBoomMatchResult();
        sendMsgAsServer(MSG_RESULT);
        if(matchResult != -1) { // Nao empatou
            nextMsgType = MSG_PLAY_AGAIN_REQUEST;
        } else { // Empatou
            nextMsgType = MSG_REQUEST;
        }
    } else if (msg.type == MSG_PLAY_AGAIN_RESPONSE) {
        if(checkForErrors()) {
            sendMsgAsServer(nextMsgType);
            return;
        }
        if (msg.client_action) {
            nextMsgType = MSG_REQUEST;
            printf("Cliente deseja jogar novamente.\n");
        } else {
            nextMsgType = MSG_END;
            printf("Cliente não deseja jogar novamente.\n");
        }
    }
    sendMsgAsServer(nextMsgType);
}

void sendMsgAsServer(MessageType type) {
    msg.type = type;
    switch(type) {
        case MSG_REQUEST:
			strcpy(msg.message,
                "Escolha sua jogada:\n"
                "0 - Nuclear Attack\n"
                "1 - Intercept Attack\n"
                "2 - Cyber Attack\n"
                "3 - Drone Strike\n"
                "4 - Bio Attack\n"
            );
            printf("Apresentando as opções para o cliente.\n");
			break;
		case MSG_RESULT:
			snprintf(msg.message, MSG_SIZE,
                "Você escolheu: %s\n"
                "Servidor escolheu: %s\n"
                "Resultado: %s!\n",
                getMoveName(msg.client_action), getMoveName(msg.server_action), getResultName(msg.result));
			break;
		case MSG_PLAY_AGAIN_REQUEST:
			strcpy(msg.message,
                "Deseja jogar novamente?\n"
                "1 - Sim\n"
                "0 - Não\n"
            );
            printf("Perguntando se o cliente deseja jogar novamente.\n");
			break;
		case MSG_END:
			snprintf(msg.message, MSG_SIZE,
                "Fim de Jogo!\n"
                "Placar final: Você %d x %d Servidor\n"
                "Obrigado por jogar!\n",
                msg.client_wins, msg.server_wins);
            printf("Enviando placar final.\n");
			break;
		default:
			break;
    }
    send(csock, &msg, sizeof(msg), 0);
    if(type == MSG_REQUEST || type == MSG_PLAY_AGAIN_REQUEST) {
        recieveMsgAsServer();
    } else if (type == MSG_ERROR) {
        if(errorFlag){
            sendMsgAsServer(MSG_PLAY_AGAIN_REQUEST);
        } else {
            sendMsgAsServer(MSG_REQUEST);
        }
    } else if (type == MSG_END) {
        endGame();
    }
}

void jokenBoomLogic() {
    if (msg.client_action == msg.server_action) {
        msg.result = -1; // Empate
    } else if ((msg.client_action == 0 && (msg.server_action == 2 || msg.server_action == 3)) ||
               (msg.client_action == 1 && (msg.server_action == 0 || msg.server_action == 4)) ||
               (msg.client_action == 2 && (msg.server_action == 1 || msg.server_action == 3)) ||
               (msg.client_action == 3 && (msg.server_action == 1 || msg.server_action == 4)) ||
               (msg.client_action == 4 && (msg.server_action == 0 || msg.server_action == 2))) {
        msg.result = 1; // Cliente vence
    } else {
        msg.result = 0; // Servidor vence
    }
}

void makeServerPlay() {
    msg.server_action = rand() % 5;
    printf("Servidor escolheu aleatoriamente %d.\n", msg.server_action);
}

int jokenBoomMatchResult() {
    makeServerPlay();
    jokenBoomLogic();
    if (msg.result == 1) {
        msg.client_wins++;
        printf("Placar atualizado: Cliente %d x %d Servidor\n", msg.client_wins, msg.server_wins); // Tirar um dos prints
    } else if (msg.result == 0) {
        msg.server_wins++;
        printf("Placar atualizado: Cliente %d x %d Servidor\n", msg.client_wins, msg.server_wins);
    } else {
        printf("Jogo empatado.\nSolicitando ao cliente mais uma escolha.\n");
    }
    return msg.result;
}

int checkForErrors() {
    switch (msg.type) {
        case MSG_RESPONSE:
            if (msg.client_action < 0 || msg.client_action > 4) {
                printf("Erro: opção inválida de jogada.\n");
                strcpy(msg.message, "Por favor, selecione um valor de 0 a 4.\n");
                errorFlag = 0;
                return 1;
            }
            return 0;
        case MSG_PLAY_AGAIN_RESPONSE:
            if (msg.client_action < 0 || msg.client_action > 1) {
                printf("Erro: resposta inválida para jogar novamente.\n");
                strcpy(msg.message, "Por favor, digite 1 para jogar novamente ou 0 para encerrar.\n");
                errorFlag = 1;
                return 1;
            }
            return 0;
        default:
            return 0;
    }
}

void resetMsg() {
    msg.type = 0;
    msg.client_action = -1;
    msg.server_action = -1;
    msg.result = -1;
    msg.client_wins = 0;
    msg.server_wins = 0;
    memset(msg.message, 0, MSG_SIZE);
}

void endGame() {
    printf("Encerrando conexão.\n");
    close(csock);
    printf("Cliente desconectado.\n");
    resetMsg();
    startNewConection = 1;
}

const char* getMoveName(int move) {
    switch (move) {
        case 0:
            return "Nuclear Attack";
        case 1:
            return "Intercept Attack";
        case 2:
            return "Cyber Attack";
        case 3:
            return "Drone Strike";
        case 4:
            return "Bio Attack";
        default: // Tentar remover esse
            return "Jogada inválida";
    }
}

const char* getResultName(int move) {
    switch (move) {
        case 0:
            return "Derrota";
        case 1:
            return "Vitória";
        case -1: // Tentar remover esse
            return "Empate";
		default:
			return "Resultado inválido";
    }
}
/*
void printReplayChoice(int choice) {
    switch (choice) {
        case 0:
            printf("Cliente não deseja jogar novamente.\n");
            break;
        case 1: 
            printf("Cliente deseja jogar novamente.\n");
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
    printf("Apresentando as opções para o cliente.\n");
}

void recieveClientMsgResponse() {
    recv(csock, &msg, sizeof(msg), 0); // Recebe a jogada do cliente
    printf("Cliente escolheu %d\n", msg.client_action);
}

int makeServerPlay1() {
    msg.server_action = rand() % 5;
    printf("Servidor escolheu aleatoriamente %d.\n", msg.server_action);
}

int jokenBoomMatchResult() {
    makeServerPlay1();
    msg.result = jokenBoomLogic(msg.client_action, msg.server_action);
    if (msg.result == 1) {
        msg.client_wins++;
        printf("Placar atualizado: Cliente %d x %d Servidor\n", msg.client_wins, msg.server_wins); // Tirar um dos prints
    } else if (msg.result == 0) {
        msg.server_wins++;
        printf("Placar atualizado: Cliente %d x %d Servidor\n", msg.client_wins, msg.server_wins);
    } else {
        printf("Jogo empatado.\nSolicitando ao cliente mais uma escolha.\n");
    }
    return msg.result;
}

int makeServerPlay() {
    msg.server_action = rand() % 5;
    printf("Servidor escolheu aleatoriamente %d.\n", msg.server_action);

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
            printf("Jogo empatado.\nSolicitando ao cliente mais uma escolha.\n");
            break;
    }
    printf("Placar atualizado: Cliente %d x %d Servidor\n", msg.client_wins, msg.server_wins);
    return result;
}

void sendGameResults() {
    msg.type = MSG_RESULT;
    snprintf(msg.message, MSG_SIZE,
         "Você escolheu: %s\n"
         "Servidor escolheu: %s\n"
         "Resultado: %s!\n",
         msg.client_action, msg.server_action, msg.result);
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
    printf("Perguntando se o cliente deseja jogar novamente.\n");
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
    printf("Enviando placar final.\n");
}

*/
void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    srand(time(NULL));

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

        csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }
        printf("Cliente conectado.\n");

        sendMsgAsServer(MSG_REQUEST); // Envia requisição pro cliente jogar

        /*

        if(startNewConection){
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

        int isTie = -1;

        while (isTie == -1) {
            sendRequestMsg(); // Envia requisição pro user jogar
            recieveClientMsgResponse(); // Recebe a jogada do user
            while(checkForErrors()) {
                sendErrorMsg();
                sendRequestMsg(); // Envia requisição pro user jogar
                recieveClientMsgResponse(); // Recebe a jogada do user
            }
            isTie = makeServerPlay(); // Fazer jogada
            sendGameResults(); // Enviar resultado pro cliente
        }

        sendPlayAgainRequest();
        recievePlayAgainResponse();
        printReplayChoice(msg.client_action);

        switch (msg.client_action) {
            case 1:
                // Jogar novamente
                break;
            case 0:
                // Sair do jogo e encerrar conexão
                startNewConection = 1;
                sendFinalResults();
                printf("Encerrando conexão.\n");
                close(csock);
                printf("Cliente desconectado.\n");
                // Encerrar conexão
                break;
            default:
                msg.type = MSG_ERROR;
                strcpy(msg.message, "Por favor, digite 1 para jogar novamente ou 0 para encerrar.\n");
                send(csock, &msg, sizeof(msg), 0);
                printf("Erro: resposta inválida para jogar novamente.\n");
                sendPlayAgainRequest();
                break;
        }*/
    }
}