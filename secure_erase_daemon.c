/*
 * Copyright (c) 2020 Intel Corporation.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <linux/vm_sockets.h>
#include <sys/un.h>
#include <stdint.h>

#define SECURE_DATA_ERASE_PORT 2345
#define CMD_SEC_ERASE_START "SEC_ERASE:START"
#define CMD_SEC_ERASE_DONE "SEC_ERASE:DONE"
#define CMD_SEC_ERASE_FAIL "SEC_ERASE:FAILED"

extern int secure_wipe_partition(char* partition, uint64_t start, uint64_t end);

int main(int argc, char *argv[])
{
	int listen_fd;
	int client_fd;
	int ret;
	char msgbuf[64] = {0};
	char* partition;
	int read_size = 0;
	uint64_t start_sector = 0, end_sector = 0;
	struct sockaddr_vm sa_listen =
	{
		.svm_family = AF_VSOCK,
		.svm_cid = VMADDR_CID_ANY,
		.svm_port = SECURE_DATA_ERASE_PORT,
	};
	struct sockaddr_vm sa_client;
	socklen_t socklen_client = sizeof(sa_client);

	if (argc < 4) {
		fprintf(stderr, "ERROR: Parameters not specified\n");
        	return -1;
    	}
	partition = argv[1];
	start_sector = atol(argv[2]);
	end_sector = atol(argv[3]);
	if(start_sector > UINT64_MAX || end_sector > UINT64_MAX)
	{
		return -1;
	}
	listen_fd = socket(AF_VSOCK, SOCK_STREAM, 0);
	if(listen_fd < 0)
	{
		fprintf(stderr, "socket init failed\n");
		return -1;
	}

	if(bind(listen_fd, (struct sockaddr*)&sa_listen, sizeof(sa_listen)) != 0)
	{
		fprintf(stderr, "socket bind error\n");
		close(listen_fd);
		return -1;
	}

	if(listen(listen_fd, 32) != 0)
	{
		fprintf(stderr, "listen failed\n");
		close(listen_fd);
		return -1;
	}

	do
	{
		client_fd = accept(listen_fd, (struct sockaddr*)&sa_client, &socklen_client);
		if(client_fd < 0)
		{
			fprintf(stderr, "accept failed\n");
			ret = -1;
			break;
		}

		bzero(msgbuf, sizeof(msgbuf));
		//Receive a message from client
		if((read_size = recv(client_fd , msgbuf , sizeof(msgbuf) , 0)) > 0)
		{
			msgbuf[strlen(CMD_SEC_ERASE_START)+1]='\0';
			if(strncmp(msgbuf, CMD_SEC_ERASE_START, strlen(CMD_SEC_ERASE_START)) == 0)
			{
				
				if(secure_wipe_partition(partition, start_sector*512, end_sector*512) == true)
				{
					send(client_fd, CMD_SEC_ERASE_DONE, strlen(CMD_SEC_ERASE_DONE)+1, 0);
					fprintf(stdout, "SECURE ERASE SUCCESS\n");
				}
				else
				{
					send(client_fd, CMD_SEC_ERASE_FAIL, strlen(CMD_SEC_ERASE_FAIL)+1, 0);
				}
			}
			else
			{
				send(client_fd, CMD_SEC_ERASE_FAIL, strlen(CMD_SEC_ERASE_FAIL)+1, 0);
			}

		}
		else if(read_size == 0) {
			fprintf(stdout, "Client disconnected\n");
			fflush(stdout);
		}
		else if(read_size == -1) {
			fprintf(stderr, "recv failed\n");
		}
		if(client_fd >= 0)
			close(client_fd);
	} while(1);

	if(listen_fd >= 0)
		close(listen_fd);
	return ret;
}

