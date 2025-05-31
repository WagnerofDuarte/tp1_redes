#include "client.h"

GameMessage msg;
int s;

void recieveMsgAsClient() {
	recv(s, &msg, sizeof(msg), 0);
	switch (msg.type) {
		case MSG_REQUEST:
			printf("%s\n", msg.message);
			sendMsgAsClient(MSG_RESPONSE);
			break;
		case MSG_RESULT:
			printf("%s\n", msg.message);
			recieveMsgAsClient();
			break;
		case MSG_PLAY_AGAIN_REQUEST:
			printf("%s\n", msg.message);
			sendMsgAsClient(MSG_PLAY_AGAIN_RESPONSE);
			break;
		case MSG_ERROR:
			printf("%s\n", msg.message);
			recieveMsgAsClient();
			break;
		case MSG_END:
			printf("%s\n", msg.message);
			break;
		default:
			break;
	}
}

void sendMsgAsClient(MessageType type) {
	msg.type = type;
	switch (type) {
		case MSG_RESPONSE:
			scanf("%d", &msg.client_action);
			break;
		case MSG_PLAY_AGAIN_RESPONSE:
			scanf("%d", &msg.client_action);
			break;
		default:
			break;
	}
	send(s, &msg, sizeof(msg), 0);
	recieveMsgAsClient();
}

void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

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

	recieveMsgAsClient();
	close(s);
	exit(EXIT_SUCCESS);
}