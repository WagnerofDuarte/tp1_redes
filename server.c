#include "server.h"

GameMessage msg;
int csock;
int startNewConection = 1;
int errorFlag = 0;

void recieveMsgAsServer() {
    MessageType nextMsgType = MSG_ERROR;
    recv(csock, &msg, sizeof(msg), 0);
    if(msg.type == MSG_RESPONSE) {
        printf("\nCliente escolheu %d\n", msg.client_action);
        if(checkForErrors()) {
            sendMsgAsServer(nextMsgType);
            return;
        }
        int matchResult = jokenBoomMatchResult();
        sendMsgAsServer(MSG_RESULT);
        if(matchResult != -1) { 
            nextMsgType = MSG_PLAY_AGAIN_REQUEST;
        } else {
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
                "\nEscolha sua jogada:\n"
                "0 - Nuclear Attack\n"
                "1 - Intercept Attack\n"
                "2 - Cyber Attack\n"
                "3 - Drone Strike\n"
                "4 - Bio Attack\n"
            );
            printf("\nApresentando as opções para o cliente.\n");
			break;
		case MSG_RESULT:
			snprintf(msg.message, MSG_SIZE,
                "\nVocê escolheu: %s\n"
                "Servidor escolheu: %s\n"
                "Resultado: %s!\n",
                getMoveName(msg.client_action), getMoveName(msg.server_action), getResultName(msg.result));
			break;
		case MSG_PLAY_AGAIN_REQUEST:
			strcpy(msg.message,
                "\nDeseja jogar novamente?\n"
                "1 - Sim\n"
                "0 - Não\n"
            );
            printf("\nPerguntando se o cliente deseja jogar novamente.\n");
			break;
		case MSG_END:
			snprintf(msg.message, MSG_SIZE,
                "\nFim de Jogo!\n"
                "Placar final: Você %d x %d Servidor\n"
                "Obrigado por jogar!\n",
                msg.client_wins, msg.server_wins);
            printf("\nEnviando placar final.\n");
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
        msg.result = -1;
    } else if ((msg.client_action == 0 && (msg.server_action == 2 || msg.server_action == 3)) ||
               (msg.client_action == 1 && (msg.server_action == 0 || msg.server_action == 4)) ||
               (msg.client_action == 2 && (msg.server_action == 1 || msg.server_action == 3)) ||
               (msg.client_action == 3 && (msg.server_action == 1 || msg.server_action == 4)) ||
               (msg.client_action == 4 && (msg.server_action == 0 || msg.server_action == 2))) {
        msg.result = 1;
    } else {
        msg.result = 0;
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
        printf("Placar atualizado: Cliente %d x %d Servidor\n", msg.client_wins, msg.server_wins);
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
                printf("\nErro: opção inválida de jogada.\n");
                strcpy(msg.message, "\nPor favor, selecione um valor de 0 a 4.");
                errorFlag = 0;
                return 1;
            }
            return 0;
        case MSG_PLAY_AGAIN_RESPONSE:
            if (msg.client_action < 0 || msg.client_action > 1) {
                printf("\nErro: resposta inválida para jogar novamente.\n");
                strcpy(msg.message, "\nPor favor, digite 1 para jogar novamente ou 0 para encerrar.");
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
        default:
            return "Jogada inválida";
    }
}

const char* getResultName(int move) {
    switch (move) {
        case 0:
            return "Derrota";
        case 1:
            return "Vitória";
        case -1:
            return "Empate";
		default:
			return "Resultado inválido";
    }
}

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
        printf("\nServidor iniciado em modo IPv4 na porta %s. Aguardando conexão...\n", argv[2]);
    } else {
        printf("\nServidor iniciado em modo IPv6 na porta %s. Aguardando conexão...\n", argv[2]);
    }

    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }
        printf("\nCliente conectado.\n");

        sendMsgAsServer(MSG_REQUEST);
    }
}