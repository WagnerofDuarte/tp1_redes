#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

GameMessage msg;
int s;
int playLoop = 1;

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

void recieveRequestMsg() {
	recv(s, &msg, sizeof(msg), 0);
	if (msg.type == MSG_REQUEST) {
		printf("%s\n", msg.message);
	} else {
		// MSG INVALIDA
	}
}

void sendPlayChoseResponse() {
	int usersChoice;
	scanf("%d", &usersChoice);
	msg.type = MSG_RESPONSE;
	msg.client_action = usersChoice;
	send(s, &msg, sizeof(msg), 0); 
	printf("Você escolheu: %s\n", getMoveName(usersChoice));
}

int recieveGameResults() {
	recv(s, &msg, sizeof(msg), 0);
	if (msg.type == MSG_RESULT) {
		printf("Servidor escolheu: %s\n", getMoveName(msg.server_action));
		printf("Resultado: %s!\n", getResultName(msg.result));
		return msg.result;
	} else {
		// MSG INVALIDA
		return -2;
	}
}

void recievePlayAgainRequest() {
	recv(s, &msg, sizeof(msg), 0);
	if (msg.type == MSG_PLAY_AGAIN_REQUEST) {
		printf("%s\n", msg.message);
	} else {
		// MSG INVALIDA
	}
}

int sendPlayAgainResponse() {
	int usersChoice;
	scanf("%d", &usersChoice);
	msg.type = MSG_PLAY_AGAIN_RESPONSE;
	msg.client_action = usersChoice;
	send(s, &msg, sizeof(msg), 0);
	return usersChoice; 
}

void recieveFinalResults() {
	recv(s, &msg, sizeof(msg), 0);
	if (msg.type == MSG_END) {
		printf("%s\n", msg.message);
	} else {
		// MSG INVALIDA
	}
}

void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

#define BUFSZ 1024

int main(int argc, char **argv) {
	if (argc < 3) {
		usage(argc, argv);
	}

	struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage)) {
		usage(argc, argv);
	}

	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if (s == -1) {
		logexit("socket");
	}
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(s, addr, sizeof(storage))) {
		logexit("connect");
	}
	printf("Conectado ao servidor.\n");

	// char addrstr[BUFSZ];
	// addrtostr(addr, addrstr, BUFSZ);

	// TODO: Criar função que lida com todos os tipos de mensagem

	while (playLoop) {

		int isTie = -1;

		while(isTie == -1){
			recieveRequestMsg(); // Recebe a mensagem de escolher a jogada e mostra na tela
			sendPlayChoseResponse(); // Lê e envia para o server a escolha do user
			isTie = recieveGameResults(); // Recebe, printa escolha servidor, printa resultado
		}

		recievePlayAgainRequest();

		int startNewGame = sendPlayAgainResponse();

		if(startNewGame == 0) {
			recieveFinalResults();
			playLoop = 0;
		} else {
			// Tratar caso mensagem inválida
		}
	}

	close(s);
	exit(EXIT_SUCCESS);

	/* char buf[BUFSZ];
	memset(buf, 0, BUFSZ);
	printf("mensagem> ");
	fgets(buf, BUFSZ-1, stdin);
	size_t count = send(s, buf, strlen(buf)+1, 0);
	if (count != strlen(buf)+1) {
		logexit("send");
	}*/

	/* memset(buf, 0, BUFSZ);
	unsigned total = 0;
	while(1) {
		count = recv(s, buf + total, BUFSZ - total, 0);
		if (count == 0) {
			// Connection terminated.
			break;
		}
		total += count;
	}
	close(s);

	printf("received %u bytes\n", total);
	puts(buf);
	

	exit(EXIT_SUCCESS);
	*/
}