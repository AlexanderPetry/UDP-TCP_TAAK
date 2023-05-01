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

//gcc UDP_Server.c -l ws2_32 -o net.exe
#include <time.h>

int initialization();
void execution( int internet_socket );
void cleanup( int internet_socket );
char* SRBytes(char *message,int internet_socket,int action);
void delay(int number_of_seconds);

int main( int argc, char * argv[] )
{
	//////////////////
	//Initialization//
	//////////////////
	srand(time(NULL));
	OSInit();

	int internet_socket = initialization();

	/////////////
	//Execution//
	/////////////

	execution( internet_socket );


	////////////
	//Clean up//
	////////////

	cleanup( internet_socket );

	OSCleanup();

	return 0;
}

int initialization()
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

void execution( int internet_socket )
{
	int highest = 0;

	if(strcmp(SRBytes("",internet_socket,1), "GO") == 0)
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
			SRBytes(temp_val,internet_socket,2);
		}
	}

	delay(3);

	char temp_val[1000];
	sprintf(temp_val,"%i",highest);
	printf("Expecting: %s\n" ,temp_val);

	if(strcmp(SRBytes("",internet_socket,1), temp_val) == 0)
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
			SRBytes(temp_val,internet_socket,2);		
		}
	}

	delay(3);

	sprintf(temp_val,"%i",highest);
	printf("Expecting: %s\n" ,temp_val);
	if(strcmp(SRBytes("",internet_socket,1), temp_val) == 0)
	{
		printf("Status: finishing\n");
		SRBytes("OK",internet_socket,2);

	}
}

char* SRBytes(char *message,int internet_socket,int action)
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


void cleanup( int internet_socket )
{
	//Step 3.1
	printf("cleanup");
	close( internet_socket );
}