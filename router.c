#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// create global parameters
int costs[4][4];
int dist[4], buff[3];
int sock, portNum, this_host, num_host, new_cost, d_node, i, j;

pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;
struct sockaddr_in serverAddr;
socklen_t addr_size;
struct sockaddr_storage serverStorage;

// machine structure
struct machine {
	char name[50];
	char ip[50];
	int port;
};

struct machine my_machine[4];

// prints the table
void print_table(int i, int j, int costs[i][j])
{
        for(i=0; i<4; i++) {
                for(j=0;j<4;j++) {
                        printf("%d ", costs[i][j]);
                }
                printf("\n");
        }
}

// receives the info, updates the table
void* receive_info(void* param)
{	
	while (1)
	{
		int l_buff[3];
		recvfrom(sock, l_buff, sizeof(l_buff), 0, (struct sockaddr *)&serverStorage, &addr_size);
		printf("Recieved src: %d\n", l_buff[0]);
		printf("Received dest: %d\n", l_buff[1]);
		printf("Received cost: %d\n", l_buff[2]);
			
		pthread_mutex_lock(&my_mutex);
		costs[l_buff[0]][l_buff[1]] = l_buff[2];
		costs[l_buff[1]][l_buff[0]] = l_buff[2];
		pthread_mutex_unlock(&my_mutex);
		print_table(4,4,costs);
	}
}

// produces the least cost distance 
void* link_state(void* param)
{
	while (1)
	{	
		sleep(10);
		int visited[4];
		for (i=0; i<4;i++)
		{
			dist[i] = costs[this_host][i];
			visited[i] = 0;
		}
		visited[this_host] = 1;
		
		pthread_mutex_lock(&my_mutex);
		for (i=0; i<3; i++)
		{	
			int min_index = -1;
			int min = 10000;
			for (j=0; j<4; j++)
			{
				
				if (visited[j]==0 && dist[j]<min)
				{	
					min = dist[j];
					min_index = j;
				}
			}
			visited[min_index] = 1;
			for (j=0; j<4; j++)
                        {	
				int x = dist[j];
				int y = dist[min_index] + costs[min_index][j];
                                if (visited[j]==0)
                                {       
                                        if (y<x)
						dist[j]=y;
                                }
                        }
		}
		for (i=0; i<4; i++)
			printf("%d ", dist[i]);
		printf("\n");
		pthread_mutex_unlock(&my_mutex);
	}
}

int main (int argc, char *argv[])
{
	if (argc < 4)
        {
                printf ("add more arguments \n");
                return 1;
        }

	// initialize a few objects
	this_host = atoi(argv[1]);
	num_host = atoi(argv[2]);
        FILE * c_src = fopen(argv[3],"rb");
        FILE * h_src = fopen(argv[4],"rb");

	// create costs table in 2d array
	int i;
        for (i=0; i<4; i++)
                fscanf(c_src, "%d %d %d %d", &costs[i][0], &costs[i][1], &costs[i][2], &costs[i][3]);

	// create machines struct array
        for (i=0; i<4; i++)
                fscanf(h_src, "%s %s %d", &my_machine[i].name, &my_machine[i].ip, &my_machine[i].port);

	// set up UDP connection
	serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(my_machine[this_host].port);
        memset (serverAddr.sin_zero, '\0', sizeof (serverAddr.sin_zero));
        serverAddr.sin_addr.s_addr = htonl (INADDR_ANY);
	addr_size = sizeof(serverStorage);
	
	// create socket
	if ((sock = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf ("socket error\n");
		return 1;
	}
	// bind
	if (bind (sock, (struct sockaddr *)&serverAddr, sizeof (serverAddr)) != 0)
	{
		printf ("bind error\n");
		return 1;
	}

	// create function threads
	pthread_t thr1, thr2;
	pthread_create(&thr1, NULL, receive_info, NULL);
	pthread_create(&thr2, NULL, link_state, NULL);
	sleep(3);

	// loop this bad boy
	while (1) 
	{	
		// get the destination node and new cost
		scanf("%d %d", &d_node, &new_cost);
		buff[0] = this_host;
		buff[1] = d_node;
		buff[2] = new_cost;
		
		for (i=0; i<4; i++) 
		{
			serverAddr.sin_port = htons(my_machine[i].port);
			sendto(sock, buff, sizeof(buff), 0, (struct sockaddr *)&serverAddr, addr_size);
		
		}
	}
	return 0;
}
