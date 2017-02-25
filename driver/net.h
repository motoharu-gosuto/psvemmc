#pragma once

int init_net();
void deinit_net();

#define MSG_SIZE 256

int send_message_to_client(char* msg, int size);
