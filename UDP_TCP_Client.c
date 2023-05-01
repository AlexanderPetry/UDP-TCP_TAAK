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
	void OSInit( void ) {}
	void OSCleanup( void ) {}
#endif

int initialization_UDP( struct sockaddr ** internet_address, socklen_t * internet_address_length );
void execution_UDP( int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length );
void cleanup_UDP( int internet_socket, struct sockaddr * internet_address );
char* SRBytes_UDP(char *message,int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length ,int action);
int check_timeout_UDP(int sockfd, int timeout_sec, char* buf, struct sockaddr* addr, socklen_t* addrlen);
void delay(int number_of_seconds);
//gcc UDP_TCP_Client.c -l ws2_32 -o cnt.exe
#include <time.h>
#define MAX_BUF_SIZE 1024
int sockfd;
int number_of_bytes_received = 0;
int initialization();
void execution( int internet_socket );
void cleanup( int internet_socket );
int y = 1;


int main( int argc, char * argv[] )
{
	// UDP client
	OSInit();
	struct sockaddr * internet_address = NULL;
	socklen_t internet_address_length = 0;
	int internet_socket = initialization_UDP( &internet_address, &internet_address_length );
	execution_UDP( internet_socket, internet_address, internet_address_length );
	cleanup_UDP( internet_socket, internet_address );
	OSCleanup();

	//TCP client
	for(int z = 0; z < y; z++)
	{
		y += 1;
		OSInit();
		int internet_socket = initialization();
		execution( internet_socket );
		cleanup( internet_socket );
		OSCleanup();
	}

	return 0;
}

int initialization_UDP( struct sockaddr ** internet_address, socklen_t * internet_address_length )
{
	//Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC;
	internet_address_setup.ai_socktype = SOCK_DGRAM;
	int getaddrinfo_return = getaddrinfo( "::1", "24042", &internet_address_setup, &internet_address_result );
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
			*internet_address_length = internet_address_result_iterator->ai_addrlen;
			*internet_address = (struct sockaddr *) malloc( internet_address_result_iterator->ai_addrlen );
			memcpy( *internet_address, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			break;
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

void execution_UDP( int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length )
{
	int highest = 0;
	char input[1000];
	scanf("%s", &input); 
	time_t start_time = time(NULL);

	char temp_buffer[10000];

	SRBytes_UDP(input,internet_socket, internet_address, internet_address_length ,2);
	// SRBytes("",internet_socket, internet_address, internet_address_length ,1);

	while (1) {
    	// check if timeout has occurred
	    if (check_timeout_UDP(internet_socket, 3, temp_buffer, internet_address, &internet_address_length)) {
	        printf("Timeout occurred\n");
	        break;
	    }

    	// process received datab
    	int temp;
		temp = atoi(temp_buffer);
		printf( "Received : %s\n", temp_buffer);
		if(temp > highest)
		{
			highest = temp;
		}

    	// update start time
    	//printf("Time: %t\n", start_time);
    	start_time = time(NULL);
	}


	printf("Highest: %i\n",highest);

	while (1) {
    	// check if timeout has occurred
	    if (check_timeout_UDP(internet_socket, 2, temp_buffer, internet_address, &internet_address_length)) {
	        printf("Timeout occurred\n");
	        char temp_val[1000];
			sprintf(temp_val,"%i",highest);
			SRBytes_UDP(temp_val,internet_socket, internet_address, internet_address_length ,2);  
	    }
	    highest = 0;
    	break;

    	// update start time
    	//printf("Time: %t\n", start_time);
    	start_time = time(NULL);
	}

	while (1) {
    	// check if timeout has occurred
	    if (check_timeout_UDP(internet_socket, 3, temp_buffer, internet_address, &internet_address_length)) {
	        printf("Timeout occurred\n");
	        break;
	    }

    	// process received datab
    	int temp;
		temp = atoi(temp_buffer);
		printf( "Received : %s\n", temp_buffer);
		if(temp > highest)
		{
			highest = temp;
		}

    	// update start time
    	//printf("Time: %t\n", start_time);
    	start_time = time(NULL);
	}


	printf("Highest: %i\n",highest);

	while (1) {
    	// check if timeout has occurred
	    if (check_timeout_UDP(internet_socket, 2, temp_buffer, internet_address, &internet_address_length)) {
	        printf("Timeout occurred\n");
	        char temp_val[1000];
			sprintf(temp_val,"%i",highest);
			SRBytes_UDP(temp_val,internet_socket, internet_address, internet_address_length ,2);  
	    }

    	break;

    	// update start time
    	//printf("Time: %t\n", start_time);
    	start_time = time(NULL);
	}

	if(strcmp(SRBytes_UDP("",internet_socket, internet_address, internet_address_length ,1), "OK") == 0)
	{
		printf("Now TCP");
	}

}

char* SRBytes_UDP(char *message,int internet_socket, struct sockaddr * internet_address, socklen_t internet_address_length ,int action)
{
	char buffer[1000];
	struct sockaddr_storage client_internet_address;
	number_of_bytes_received = 0;
	socklen_t client_internet_address_length = sizeof client_internet_address;
	if(action == 1)
	{
		number_of_bytes_received = 0;
		number_of_bytes_received = recvfrom( internet_socket, buffer, ( sizeof buffer ) - 1, 0, internet_address, &internet_address_length );
		if( number_of_bytes_received == -1 )
		{
			perror( "recvfrom_0" );
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
		number_of_bytes_send = sendto( internet_socket, message, 16, 0, internet_address, internet_address_length );
		if( number_of_bytes_send == -1 )
		{
			perror( "sendto" );
		}
		char* result = malloc(sizeof(char) * 10);
        snprintf(result, 10, "%d", number_of_bytes_send);
        return result;
	}

}

int check_timeout_UDP(int sockfd, int timeout_sec, char* buf, struct sockaddr* addr, socklen_t* addrlen) 
{
    struct timeval tv;
    fd_set readfds;
    int n;
    // printf("Socket: %i\n",sockfd);
    // Set up the file descriptor set and the timeout value
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;

    // Wait for data to be received or timeout to occur
    n = select(sockfd + 1, &readfds, NULL, NULL, &tv);
    //printf("n: %i\n" , n);
    if (n == -1) {
        perror("select");
        exit(1);
    } else if (n == 0) {
        // Timeout occurred
        return 1;
    } else {
        // Data is available to be read
        memset(buf, 0, MAX_BUF_SIZE);
        int num_bytes = recvfrom(sockfd, buf, MAX_BUF_SIZE, 0, addr, addrlen);
        if (num_bytes == -1) {
            perror("recvfrom_1");
            exit(1);
        }
        return 0;
    }
}

void cleanup_UDP( int internet_socket, struct sockaddr * internet_address )
{
	//Step 3.2
	free( internet_address );

	//Step 3.1
	close( internet_socket );
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

int initialization()
{
	//Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC;
	internet_address_setup.ai_socktype = SOCK_STREAM;
	int getaddrinfo_return = getaddrinfo( "::1", "24042", &internet_address_setup, &internet_address_result );
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
			int connect_return = connect( internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			if( connect_return == -1 )
			{
				perror( "connect" );
				close( internet_socket );
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

void execution( int internet_socket )
{
	char input[1000];
	gets(input);
	// printf(input);

	int number_of_bytes_send = 0;
	number_of_bytes_send = send( internet_socket, input, 16, 0 );
	printf( "Send : %s\n", input );
	if( number_of_bytes_send == -1 )
	{
		perror( "send" );
	}

	if(strcmp(input, "STOP") == 0)
		{
			send( internet_socket, "KTHNXBYE", 16, 0 );
			printf("Stopping\n");
			y = 0;

		}

	//Step 2.2
	int number_of_bytes_received = 0;
	char buffer[1000];
	number_of_bytes_received = recv( internet_socket, buffer, ( sizeof buffer ) - 1, 0 );
	if( number_of_bytes_received == -1 )
	{
		perror( "recv" );
	}
	else
	{
		buffer[number_of_bytes_received] = '\0';
		printf( "Received : %s\n", buffer );
	}
}

void cleanup( int internet_socket )
{
	//Step 3.2
	int shutdown_return = shutdown( internet_socket, SD_SEND );
	if( shutdown_return == -1 )
	{
		perror( "shutdown" );
	}

	//Step 3.1
	close( internet_socket );
}
