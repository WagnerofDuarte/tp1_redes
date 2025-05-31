#pragma once

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