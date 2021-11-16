#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

int clientCount = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

typedef struct Channel Channel;
typedef struct Arg_thread Arg_thread;
typedef struct Client Client;

struct Channel
{
	char name[30];
	int id;
};

struct Client
{
	int index;
	int sockID;
	struct sockaddr_in clientAddr;
	int len;
};

struct Arg_thread
{
	struct Channel *listChannels;
	struct Client *Client;
	int *nbChannels;
};

Channel listChannels[10];
int nbChannels = 1;

Client client[1024];
pthread_t thread[1024];

Arg_thread arg_thread = {&listChannels, NULL, &nbChannels};

void initiateChannel(Channel list[])
{
	list[0].id = 0;
	strcpy(list[0].name, "Main");
}

void createChannel(Channel list[], char *n, int i)
{
	strcpy(list[i].name, n);
}

void *doNetworking(void *arg_thread)
{
	Arg_thread *arg = (Arg_thread *)arg_thread;
	Client *clientDetail = (Client *)arg->Client;
	Channel *listC = arg->listChannels;
	int nbChannels = *(arg->nbChannels);
	int index = clientDetail->index;
	int clientSocket = clientDetail->sockID;

	printf("Client %d connected.\n", index + 1);

	while (1)
	{

		char data[1024];
		int read = recv(clientSocket, data, 1024, 0);
		data[read] = '\0';

		char output[1024];

		//renvoie la liste de channels
		if (strcmp(data, "CHANNELS") == 0)
		{
			
			int l = 0;
			//l += snprintf(output + l, 1024, "%d : Channel %d - %s.\n", nbChannels, listC[0].id, listC[0].name);

			for (int i = 0; i < nbChannels; i++)
			{

				l += snprintf(output + l, 1024, "Channel %d - %s.\n", listC[i].id, listC[i].name);
			}

			send(clientSocket, output, 1024, 0);
			continue;
		}

		// Create channel
		if (strcmp(data,"CREATE") == 0)
		{

		}

		// Join un channel
		if (strcmp(data, "JOIN") == 0)
		{
			
		}

		if (strcmp(data, "LIST") == 0)
		{

			int l = 0;

			for (int i = 0; i < clientCount; i++)
			{

				if (i != index)
					l += snprintf(output + l, 1024, "Client %d is at socket %d.\n", i + 1, client[i].sockID);
			}

			send(clientSocket, output, 1024, 0);
			continue;
		}
		if (strcmp(data, "SEND") == 0)
		{

			read = recv(clientSocket, data, 1024, 0);
			data[read] = '\0';

			int id = atoi(data) - 1;

			read = recv(clientSocket, data, 1024, 0);
			data[read] = '\0';

			send(client[id].sockID, data, 1024, 0);
		}
	}

	return NULL;
}

int main()
{
	initiateChannel(listChannels);

	int serverSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);
	serverAddr.sin_addr.s_addr = htons(INADDR_ANY);

	if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
		return 0;

	if (listen(serverSocket, 1024) == -1)
		return 0;

	printf("Server started listenting on port 8080 ...........\n");

	while (1)
	{
		
		client[clientCount].sockID = accept(serverSocket, (struct sockaddr *)&client[clientCount].clientAddr, &client[clientCount].len);
		client[clientCount].index = clientCount;

		arg_thread.Client = &client[clientCount];

		pthread_create(&thread[clientCount], NULL, doNetworking, (void *)&arg_thread);

		clientCount++;
	}

	for (int i = 0; i < clientCount; i++)
		pthread_join(thread[i], NULL);
}