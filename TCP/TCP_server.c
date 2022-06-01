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

char tekst[256];
int aantalbytes;
char laatste16berichten[1000];
char laatste16berichten1[1000];

void run( int );
int setup();
void cleanup( int );
void PakBericht();
void RunBericht( int );

void *get_in_addr(struct sockaddr *sa);

int main(void){
	OSInit();

	PakBericht();

    fd_set master;
    fd_set read_fds;
    int fdmax;
    int listener;
    int newfd;
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

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
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
		{
        close(listener);
        continue;
    }
      break;
  	}

    if (p == NULL)
	{
      fprintf(stderr, "selectserver: failed to bind\n");
      exit(2);
  }
    freeaddrinfo(ai);

    if (listen(listener, 10) == -1)
	{
      perror("listen");
      exit(3);
  }
    FD_SET(listener, &master);

    fdmax = listener;

    for(;;)
		{
        read_fds = master;
    if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
		{
        perror("select");
      	exit(4);
    }

    for(i = 0; i <= fdmax; i++)
		{
      if (FD_ISSET(i, &read_fds))
			{
        if (i == listener)
				{
          addrlen = sizeof remoteaddr;
          newfd = accept(listener,(struct sockaddr *)&remoteaddr,&addrlen);
				//	printf("\n\n\n\n%s\n\n\n",laatste16berichten );
					send(newfd, laatste16berichten,strlen((laatste16berichten)+1), 0);
					send(newfd, laatste16berichten1,strlen((laatste16berichten1)+1), 0);

          if (newfd == -1)
					{
            perror("accept");
          }
					else
					{
              FD_SET(newfd, &master);
              if (newfd > fdmax)
						{
							fdmax = newfd;
            }
							printf("Nieuwe connecties: \n");
              printf("SERVER melding: Er is een nieuwe connectie van %s op ""socket %d\n",inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr*)&remoteaddr),remoteIP, INET6_ADDRSTRLEN),newfd);

              char newConMsg[256];
							//RUNclient();
            	sprintf(newConMsg,"Nieuwe connectie van ");
              strcat(newConMsg, inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr*)&remoteaddr),remoteIP, INET6_ADDRSTRLEN));
              strcat(newConMsg," on socket ");
              char newfdString[4];
            	itoa(newfd,newfdString,10);
              strcat(newConMsg, newfdString);
              newConMsg[strlen(newConMsg)] = '\r';

            for(j = 0; j <= fdmax; j++)
						{
            	if (FD_ISSET(j, &master))
							{
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
          if ((aantalbytes = recv(i, tekst, sizeof tekst, 0)) <= 0)
					{

            if (aantalbytes == 0)
						{
								printf("SERVER melding: socket %d is afgesloten!\n", i);
            }
						else
						{
              perror("recv");
            }
              close(i);
              FD_CLR(i, &master);
            }
					else
					{
            for(j = 0; j <= fdmax; j++)
						{
              if (FD_ISSET(j, &master))
							{
                if (j != listener && j != i)
								{
                  if (send(j, tekst, aantalbytes, 0) == -1)
									{
                    perror("send");
                  }
                                }
                            }
                        }
                    }
                }
            }
        }
      }
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

void run( int internet_socket ){
		//Step 2.1
		int Verzonden_Bytes = 0;
		Verzonden_Bytes = send( internet_socket, "GET /history.php?i=12100324 HTTP/1.0\r\nHost: student.pxl-ea-ict.be\r\n\r\n", 77, 0 );
		if( Verzonden_Bytes == -1 )
		{
			perror( "send" );
		}
		//Step 2.2
		int Ontvangen_Bytes = 0;
		char buffer[1000];
		Ontvangen_Bytes = recv( internet_socket, buffer, ( sizeof buffer ) - 1, 0 );
		if( Ontvangen_Bytes == -1 )
		{
			perror( "recv" );
		}
		else
		{
			buffer[Ontvangen_Bytes] = '\0';
			printf( "Received1:\n%s\n", buffer );
			sprintf(laatste16berichten,"\n%s \n\n", buffer);
		}
		Ontvangen_Bytes = recv( internet_socket, buffer, ( sizeof buffer ) - 1, 0 );
		if( Ontvangen_Bytes == -1 )
		{
			perror( "recv" );
		}
		else
		{
			buffer[Ontvangen_Bytes] = '\0';
			printf( "%s\n", buffer );
			sprintf(laatste16berichten1,"\n%s \n\n", buffer);
		}
	}
int setup(){
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
void cleanup( int internet_socket ){
		//Step 3.2
		int shutdown_return = shutdown( internet_socket, SD_SEND );
		if( shutdown_return == -1 )
		{
			perror( "shutdown" );
		}
		//Step 3.1
		close( internet_socket );
	}
void PakBericht( int internet_socket ){
		internet_socket = setup();
		run( internet_socket );
		cleanup( internet_socket );
	}
void RunBericht( int internet_socket ){
		tekst[aantalbytes] = '\0';
		int TekstLengte;
		char Tekst[256];

		for (int i = 0, j; tekst[i] != '\0'; ++i)
		{
	      while (!(tekst[i] >= 'a' && tekst[i] <= 'z') && !(tekst[i] >= 'A' && tekst[i] <= 'Z') && !(tekst[i] == '\0'))
		  {
	         for (j = i; tekst[j] != '\0'; ++j)
			 {
	            tekst[j] = tekst[j + 1];
	         }
	         tekst[j] = '\0';
	      }
			}
	}

