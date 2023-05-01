#ifdef _WIN32
	#define _WIN32_WINNT _WIN32_WINNT_WIN7
	#include <winsock2.h> //for all socket programming
	#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	void OSInit( void )
	{
		WSADATA wsaData;
		int WSAError = WSAStartup( MAKEWORD( 2, 0 ), &wsaData ); 
		if( WSAError != 0 )
		{
			fprintf( stderr, "WSAStartup errno = %d\n", WSAError );
			exit( -1 );
		}
	}
	void OSCleanup( void )
	{
		WSACleanup();
	}
	#define perror(string) fprintf( stderr, string ": WSA errno = %d\n", WSAGetLastError() )
#else
	#include <sys/socket.h> //for sockaddr, socket, socket
	#include <sys/types.h> //for size_t
	#include <netdb.h> //for getaddrinfo
	#include <netinet/in.h> //for sockaddr_in
	#include <arpa/inet.h> //for htons, htonl, inet_pton, inet_ntop
	#include <errno.h> //for errno
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	int OSInit( void ) {}
	int OSCleanup( void ) {}
#endif

//gcc file.c -l ws2_32 -o net.exe
#include <time.h>
#include <string.h>
#include <unistd.h>

int initialization();
int connection( int internet_socket );
int parse_formula(char* formula);
void execution( int internet_socket );
void cleanup( int internet_socket, int client_internet_socket );

int steps = 0;
int number_of_bytes_received = -1;
int number_of_bytes_send = 0;
char buffer[1000];
char delim[] = " ";
int num1 = 0;
int num2 = 0;
int result = 0;
int y = 1;
char *operator;
char resultChar[10];

char *output;
int stopped = 0;

int initialization_UDP();
void execution_UDP( int internet_socket );
void cleanup_UDP( int internet_socket );
char* SRBytes_UDP(char *message,int internet_socket,int action);
void delay(int number_of_seconds);

void cleanup( int internet_socket, int client_internet_socket);
int parse_formula(char* formula);
void execution( int internet_socket);
int connection( int internet_socket );
int initialization();



int main( int argc, char * argv[] )
{

	// UDP
	srand(time(NULL));
	OSInit();
	int internet_socket = initialization_UDP();
	execution_UDP( internet_socket );
	cleanup_UDP( internet_socket );
	OSCleanup();

	// TCP
	for(int z = 0; z < y; z++)
	{
		y += 1;
		OSInit();
		int internet_socket = initialization();
		int client_internet_socket = connection( internet_socket );
		execution( client_internet_socket );
		cleanup( internet_socket, client_internet_socket );
		OSCleanup();
	}

	return 0;
}

int initialization_UDP()
{
	//Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC;
	internet_address_setup.ai_socktype = SOCK_DGRAM;
	internet_address_setup.ai_flags = AI_PASSIVE;
	int getaddrinfo_return = getaddrinfo( NULL, "24042", &internet_address_setup, &internet_address_result );
	if( getaddrinfo_return != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
		exit( 1 );
	}

	int internet_socket = -1;
	struct addrinfo * internet_address_result_iterator = internet_address_result;
	while( internet_address_result_iterator != NULL )
	{
		//Step 1.2
		internet_socket = socket( internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol );
		if( internet_socket == -1 )
		{
			perror( "socket" );
		}
		else
		{
			//Step 1.3
			int bind_return = bind( internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			if( bind_return == -1 )
			{
				close( internet_socket );
				perror( "bind" );
			}
			else
			{
				break;
			}
		}
		internet_address_result_iterator = internet_address_result_iterator->ai_next;
	}

	freeaddrinfo( internet_address_result );

	if( internet_socket == -1 )
	{
		fprintf( stderr, "socket: no valid socket address found\n" );
		exit( 2 );
	}

	return internet_socket;
}

void execution_UDP( int internet_socket )
{
	int highest = 0;

	if(strcmp(SRBytes_UDP("",internet_socket,1), "GO") == 0)
	{
		printf("Status: starting\n");
		for(int i = 0; i < 5; i++)
		{
			int r = rand();
			if(r > highest)
			{
				highest = r;
			}
			char temp_val[1000];
			sprintf(temp_val,"%i",r);
			SRBytes_UDP(temp_val,internet_socket,2);
		}
	}

	delay(3);

	char temp_val[1000];
	sprintf(temp_val,"%i",highest);
	printf("Expecting: %s\n" ,temp_val);

	if(strcmp(SRBytes_UDP("",internet_socket,1), temp_val) == 0)
	{
		highest = 0;
		for(int i = 0; i < 5; i++)
		{
			int r = rand();
			if(r > highest)
			{
				highest = r;
			}
			char temp_val[1000];
			sprintf(temp_val,"%i",r);
			SRBytes_UDP(temp_val,internet_socket,2);		
		}
	}

	delay(3);

	sprintf(temp_val,"%i",highest);
	printf("Expecting: %s\n" ,temp_val);
	if(strcmp(SRBytes_UDP("",internet_socket,1), temp_val) == 0)
	{
		printf("Status: finishing\n");
		SRBytes_UDP("OK",internet_socket,2);

	}
}

char* SRBytes_UDP(char *message,int internet_socket,int action)
{
	char buffer[1000];
	struct sockaddr_storage client_internet_address;
	int number_of_bytes_received = 0;
	socklen_t client_internet_address_length = sizeof client_internet_address;
	if(action == 1)
	{
		number_of_bytes_received = recvfrom( internet_socket, buffer, ( sizeof buffer ) - 1, 0, (struct sockaddr *) &client_internet_address, &client_internet_address_length );
		if( number_of_bytes_received == -1 )
		{
			perror( "recvfrom" );
			return NULL;
		}
		else
		{
			buffer[number_of_bytes_received] = '\0';
			printf( "Received : %s\n", buffer );
		}
		return strdup(buffer);
	}
	
	if(action == 2)
	{
		int number_of_bytes_send = 0;
		number_of_bytes_send = sendto( internet_socket, message, strlen(message), 0, (struct sockaddr *) &client_internet_address, client_internet_address_length );
		printf( "Send : %s\n", message);
		if( number_of_bytes_send == -1 )
		{
			perror( "sendto" );
		}
		char* result = malloc(sizeof(char) * 10);
        snprintf(result, 10, "%d", number_of_bytes_send);
        return result;
	}

}

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
 
    // Storing start time
    clock_t start_time = clock();
 
    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}


void cleanup_UDP( int internet_socket )
{
	//Step 3.1
	printf("cleanup");
	close( internet_socket );
}

int initialization()
{
	//Step 1.1
	//printf("step 1.1");
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC;
	internet_address_setup.ai_socktype = SOCK_STREAM;
	internet_address_setup.ai_flags = AI_PASSIVE;
	int getaddrinfo_return = getaddrinfo( NULL, "24042", &internet_address_setup, &internet_address_result );
	if( getaddrinfo_return != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
		exit( 1 );
	}

	int internet_socket = -1;
	struct addrinfo * internet_address_result_iterator = internet_address_result;
	while( internet_address_result_iterator != NULL )
	{
		//Step 1.2
		//printf("step 1.2");
		internet_socket = socket( internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol );
		if( internet_socket == -1 )
		{
			perror( "socket" );
		}
		else
		{
			//Step 1.3
			//printf("step 1.3");
			int bind_return = bind( internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			if( bind_return == -1 )
			{
				perror( "bind" );
				close( internet_socket );
			}
			else
			{
				//Step 1.4
				//printf("step 1.4");
				int listen_return = listen( internet_socket, 1 );
				if( listen_return == -1 )
				{
					close( internet_socket );
					perror( "listen" );
				}
				else
				{
					break;
				}
			}
		}
		internet_address_result_iterator = internet_address_result_iterator->ai_next;
	}

	freeaddrinfo( internet_address_result );

	if( internet_socket == -1 )
	{
		fprintf( stderr, "socket: no valid socket address found\n" );
		exit( 2 );
	}

	return internet_socket;
}

int connection( int internet_socket )
{
	//Step 2.1
	struct sockaddr_storage client_internet_address;
	socklen_t client_internet_address_length = sizeof client_internet_address;
	int client_socket = accept( internet_socket, (struct sockaddr *) &client_internet_address, &client_internet_address_length );
	if( client_socket == -1 )
	{
		perror( "accept" );
		close( internet_socket );
		exit( 3 );
	}
	return client_socket;
}

void execution( int internet_socket )
{
	int cntr = 0;
		//printf("\n|%i|\n",steps);
		switch (steps)
		{
			case 0:
				//printf("%i\n",steps);
    				number_of_bytes_received = recv( internet_socket, buffer, ( sizeof buffer ) - 1, 0 );

				buffer[number_of_bytes_received] = '\0';
				printf( "\nReceived :(%s)\n", buffer );
				



				// printf("stopping %i\n", strcmp(buffer, "STOP") == 0);

				
				if(strcmp(buffer, "STOP") == 0)
				{
					printf("Stopping\n");
					//printf("%i => ",steps);
					y = 0;
					steps = 3;
				}
				else
				{
					//printf("%i => ",steps);
					steps = 1;
				}

			case 1:
				//printf("%i\n",steps);
				//for(int i=0; i<1; i++)
					//{
						//printf("formula\n");
						result = parse_formula(buffer);
					//}
				//printf("%i => ",steps);
				steps = 2;
				
			case 2:
				printf("");
				int k = sprintf(resultChar,"%i", result);
				printf("result = %s\n",resultChar,strlen(resultChar));
				number_of_bytes_send = 0;
				number_of_bytes_send = send( internet_socket, resultChar, strlen(resultChar), 0 );	
				//printf("send = %i\n",number_of_bytes_send );
				number_of_bytes_received = -1;
				//printf("%i => ",steps);

				if( number_of_bytes_send == -1 )
				{
					perror( "send" );
					printf("error: number_of_bytes_send == -1\n");
					break;
				}
				steps = 0;
				break;

			case 7:
				printf("Stopped\n");
				stopped = 1;
				break;

			default:
				printf("error: steps out of bounds\n");
				stopped = 1;
				break;
		}
		
	}/*

	for(;;)
	{
		recv( internet_socket, buffer, ( sizeof buffer ) - 1, 0 );
		if(strcmp(buffer, "STOP") == 0)
			{
				printf("Stopping");
				break;
			}
		else
		{
			result = parse_formula(buffer);
			sprintf(resultChar,"%i", result);
			number_of_bytes_send = 0;
			number_of_bytes_send = send( internet_socket, resultChar, strlen(resultChar), 0 );	
			printf("send = %i\n",number_of_bytes_send );
		}
	}*/


int parse_formula(char* formula) 
{
    // Use strtok to split the string into two numbers and an operator
    //printf("parse formula: %s\n", formula);
    char* token = strtok(formula, " ");//"+-x/");
    if (token != NULL) {
        //printf("token: %s => ", token);
        num1 = atoi(token); //-->problem
    	//printf("num1: %i\n", num1);
        token = strtok(NULL, " ");
        if (token != NULL) {
        	//printf("token: %s => ", token);
            operator = token;
            //printf("operator: %s\n", operator);

            token = strtok(NULL, " ");
            //printf("token: %s => ", token);
            num2 = atoi(token);
            //printf("num2: %i\n", num2);
            // Compute the solution to the formula
			result = 0;
            switch (*operator) {
                case '+':
					//printf("result( %s ) = num1( %i ) + num2( %i )\n",result,num1, num2);
                    result = num1 + num2;
                    break;
                case '-':
                    result = num1 - num2;
                    break;
                case 'x':
                    result = num1 * num2;
                    break;
                case '/':
                    result = num1 / num2;
                    break;
                default:
                    //printf("Invalid operator\n");
                    return 0;
            }
            //printf("result 1: %s\n",result);
            return result;
        } else {
            //printf("Invalid formula\n");
            return 0;
        }
    } else {
        //printf("Invalid formula\n");
        return 0;
    }
}

void cleanup( int internet_socket, int client_internet_socket )
{
	printf("shutdown");
	//Step 4.2
	int shutdown_return = shutdown( client_internet_socket, SD_RECEIVE );
	if( shutdown_return == -1 )
	{
		perror( "shutdown" );
	}

	//Step 4.1
	close( client_internet_socket );
	close( internet_socket );
}