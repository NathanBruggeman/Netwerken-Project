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
#define PORT "9025"

char tekst[256];    // tekst for client data
int aantalbytes;		//
char laatste16berichten[1000];

void RunBericht( int );
void cleanupBericht( int );
void StuurBerichtNaarHTTP();
void runHTTP( int );
void cleanupHTTP( int );
void PakHTTPBerichten();
void runHTTPGeenTekst();
void cleanupHTTPGeenTekst( int );
void PakHTTPBerichtenGeenTekst();
void *get_in_addr(struct sockaddr *sa);

int SetupBericht();
int setupHTTP();
int setupHTTPGeenTekst();



int main(void){
	OSInit();

	//Gets latest 16 messages.
	PakHTTPBerichten();

    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
	{
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next)
	{
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
		{
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
		{
            close(listener);
            continue;
        }
        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL)
	{
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1)
	{
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;)
	{
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
		{
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++)
		{
            if (FD_ISSET(i, &read_fds))
			{ // we got one!!
                if (i == listener)
				{
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,(struct sockaddr *)&remoteaddr,&addrlen);

					//This gets the latest 16 messages from the HTTP server and forwards it to the new user.
					PakHTTPBerichtenGeenTekst();
					send(newfd, laatste16berichten,strlen((laatste16berichten)+1), 0);

                    if (newfd == -1)
					{
                        perror("accept");
                    }
					else
					{
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax)
						{   // keep track of the max
							fdmax = newfd;
                        }
                        printf("SERVER melding: Er is een nieuwe connectie van %s op ""socket %d\n",inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr*)&remoteaddr),remoteIP, INET6_ADDRSTRLEN),newfd);

						//Sends that a new client got connected to everyone.
                        char newConMsg[256];
                        sprintf(newConMsg,"Nieuwe connectie van ");
                        strcat(newConMsg, inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr*)&remoteaddr),remoteIP, INET6_ADDRSTRLEN));
                        strcat(newConMsg," on socket ");
                        char newfdString[4];
                        itoa(newfd,newfdString,10);
                        strcat(newConMsg, newfdString);
                        newConMsg[strlen(newConMsg)] = '\r';

                        for(j = 0; j <= fdmax; j++)
						{
                            // send to everyone!
                            if (FD_ISSET(j, &master))
							{
                                // except the listener and ourselves
                                if (j != listener && j != i)
								{
                                    if (send(j, newConMsg,strlen(newConMsg), 0) == -1)
									{
                                        perror("send");
                                    }
                                }

                            }
                        }
                    }
                }
				else
				{
                    // handle data from a client
                    if ((aantalbytes = recv(i, tekst, sizeof tekst, 0)) <= 0)
					{
                        // got error or connection closed by client
                        if (aantalbytes == 0)
						{
                            // connection closed
                            printf("SERVER melding: socket %d is afgesloten!\n", i);
                        } else
						{
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    }
					else
					{
                        // we got some data from a client
                        for(j = 0; j <= fdmax; j++)
						{
                            // send to everyone!
                            if (FD_ISSET(j, &master))
							{
                                // except the listener and ourselves
                                if (j != listener && j != i)
								{
                                    if (send(j, tekst, aantalbytes, 0) == -1)
									{
                                        perror("send");
                                    }
									StuurBerichtNaarHTTP();
                                }

                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    OSCleanup();
    return 0;
}



void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET)
	{
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void runHTTP( int internet_socket ){

	int Verzonden_Bytes = 0;
	Verzonden_Bytes = send( internet_socket, "GET /history.php?i=12100324 HTTP/1.0\r\nHost: student.pxl-ea-ict.be\r\n\r\n", 77, 0 );
	if( Verzonden_Bytes == -1 )
	{
		perror( "send" );
	}

	//Step 2.2
	int Aangekomen_Bytes = 0;
	char buffer[1000];
	Aangekomen_Bytes = recv( internet_socket, buffer, ( sizeof buffer ) - 1, 0 );
	if( Aangekomen_Bytes == -1 )
	{
		perror( "recv" );
	}
	else
	{
		buffer[Aangekomen_Bytes] = '\0';
		printf( "%s\n\n", buffer );
	}
	Aangekomen_Bytes = recv( internet_socket, buffer, ( sizeof buffer ) - 1, 0 );
	if( Aangekomen_Bytes == -1 )
	{
		perror( "recv" );
	}
	else
	{
		buffer[Aangekomen_Bytes] = '\0';
		printf( "%s\n\n", buffer );
	}
}

void PakHTTPBerichten( int internet_socket ){
	internet_socket = setupHTTP();
	runHTTP( internet_socket );
	cleanupHTTP( internet_socket );
}

void cleanupHTTP( int internet_socket ){
	//Step 3.2
	int shutdown_return = shutdown( internet_socket, SD_SEND );
	if( shutdown_return == -1 )
	{
		perror( "shutdown" );
	}

	//Step 3.1
	close( internet_socket );
}

void StuurBerichtNaarHTTP( int internet_socket ){
	internet_socket = SetupBericht();
	RunBericht( internet_socket );
	cleanupBericht( internet_socket );}

void cleanupBericht( int internet_socket ){
	//Step 3.2
	int shutdown_return = shutdown( internet_socket, SD_SEND );
	if( shutdown_return == -1 )
	{
		perror( "shutdown" );
	}
	//Step 3.1
	close( internet_socket );
}

void RunBericht( int internet_socket ){
	tekst[aantalbytes] = '\0';
	int lenghtOfContentPacketToSend;
	char contentPacketToSend[256];

	for (int i = 0, j; tekst[i] != '\0'; ++i)
	{
      // enter the loop if the character is not an alphabet
      // and not the null character
      while (!(tekst[i] >= 'a' && tekst[i] <= 'z') && !(tekst[i] >= 'A' && tekst[i] <= 'Z') && !(tekst[i] == '\0'))
	  {
         for (j = i; tekst[j] != '\0'; ++j)
		 {
            // if jth element of line is not an alphabet,
            // assign the value of (j+1)th element to the jth element
            tekst[j] = tekst[j + 1];
         }
         tekst[j] = '\0';
      }
		}
}

void runHTTPGeenTekst( int internet_socket ){
	//Step 2.1
	int Verzonden_Bytes = 0;
	Verzonden_Bytes = send( internet_socket, "GET /history.php?i=12100324 HTTP/1.0\r\nHost: student.pxl-ea-ict.be\r\n\r\n", 77, 0 );
	if( Verzonden_Bytes == -1 )
	{
		perror( "send" );
	}

	//Step 2.2
	int Aangekomen_Bytes = 0;
	char buffer[1000];
	Aangekomen_Bytes = recv( internet_socket, buffer, ( sizeof buffer ) - 1, 0 );
	if( Aangekomen_Bytes == -1 )
	{
		perror( "recv" );
	}
	else
	{
		buffer[Aangekomen_Bytes] = '\0';
		sprintf(laatste16berichten,"\n Dit zijn de laatste 16 berichten van de HTTP server:\n %s \n\n", buffer);
	}
	Aangekomen_Bytes = recv( internet_socket, buffer, ( sizeof buffer ) - 1, 0 );
	if( Aangekomen_Bytes == -1 )
	{
		perror( "recv" );
	}
	else
	{
		buffer[Aangekomen_Bytes] = '\0';
	}
}

void cleanupHTTPGeenTekst( int internet_socket ){
	//Step 3.2
	int shutdown_return = shutdown( internet_socket, SD_SEND );
	if( shutdown_return == -1 )
	{
		perror( "shutdown" );
	}

	//Step 3.1
	close( internet_socket );
}

void PakHTTPBerichtenGeenTekst( int internet_socket ){
	internet_socket = setupHTTPGeenTekst();
	runHTTPGeenTekst( internet_socket );
	cleanupHTTPGeenTekst( internet_socket );
}

int SetupBericht(){

	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC;
	internet_address_setup.ai_socktype = SOCK_STREAM;
	int getaddrinfo_return = getaddrinfo( "student.pxl-ea-ict.be", "80", &internet_address_setup, &internet_address_result );
	if( getaddrinfo_return != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
		exit( 1 );
	}

	int internet_socket = -1;
	struct addrinfo * internet_address_result_iterator = internet_address_result;
	while( internet_address_result_iterator != NULL )
	{

		internet_socket = socket( internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol );
		if( internet_socket == -1 )
		{
			perror( "socket" );
		}
		else
		{

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

int setupHTTP()
{
	//Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_INET;
	internet_address_setup.ai_socktype = SOCK_STREAM;
	int getaddrinfo_return = getaddrinfo( "student.pxl-ea-ict.be", "80", &internet_address_setup, &internet_address_result );
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

int setupHTTPGeenTekst(){
	//Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_INET;
	internet_address_setup.ai_socktype = SOCK_STREAM;
	int getaddrinfo_return = getaddrinfo( "student.pxl-ea-ict.be", "80", &internet_address_setup, &internet_address_result );
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
