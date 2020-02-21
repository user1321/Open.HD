#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>


pthread_t hFileThread=0;
pthread_t hFileThread2=0;
pthread_t hUDPThread=0;

int ActiveThread = 2;
int hOut = 0;
int hResponse = 0;
//1 - Lora thread active
//2 - OpenHD thread active
#define BUFLEN 1
#define PORT 2312

int StartUDPServer()
{
	struct sockaddr_in si_me, si_other;
	
	int s, i, slen = sizeof(si_other) , recv_len;
	char buf[BUFLEN+1];
	
	
	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		printf("socket error");
	}
	
	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));
	
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//bind socket to port
	if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
	{
		printf("bind error");
	}
	
	//keep listening for data
	while(1)
	{
		
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
		{
			printf("recvfrom()");
		}
		else
		{
			//1 - Lora thread active
			//2 - OpenHD thread active
			
			buf[recv_len] = '\0';
			ActiveThread = strtol(&buf[0],NULL,10) ;
		}

	}

	close(s);
	return NULL;
}


int OpenAndConfigure(char *file)
{
	int serialport = open(file, O_RDWR | O_NOCTTY ); // | O_NDELAY);
	if(serialport != -1)
	{
		struct termios options;
                tcgetattr(serialport, &options);
                cfmakeraw(&options);
                cfsetospeed(&options, B9600);
                options.c_cflag &= ~CSIZE;
                options.c_cflag |= CS8; // Set 8 data bits
                options.c_cflag &= ~PARENB; // Set no parity
                options.c_cflag &= ~CSTOPB; // 1 stop bit
                options.c_lflag &= ~ECHO; // no echo
                options.c_cflag &= ~CRTSCTS; // no RTS/CTS Flow Control
                options.c_cflag |= CLOCAL; // Set local mode on
                tcsetattr(serialport, TCSANOW, &options); //write options
		return serialport;
	}
	printf(stderr,"cant configure port\n");
	return -1;

}

void ReadFile(char *file, int ThreadID)
{
	int fd = 0;
	if(ThreadID == 1)
	{
		fd = OpenAndConfigure(file);
	}
	else
	{
		fd = OpenAndConfigure(file);
	}


    int ret;
    char* buf = (char*)malloc(2048);

    do
    {
        ret = read(fd, buf, 1024);
	if( ret != -1 && ret != 0)
	{
		if ( ThreadID ==  ActiveThread)
		{
			write(hOut,buf,ret);
			fsync(hOut);
		}
	}
     }while(1);
}


//1 - Lora thread active
//2 - OpenHD thread active
void* MavlinkLoraReader(void *arg)
{
	ReadFile("/dev/ttyUSBLoraMavOut",1);
}

void* MavlinkOpenHDReader(void *arg)
{
	ReadFile("/dev/pts/1",2);
}

void *ResponseThread(void *arg)
{
	int ret;
	char* buf = (char*)malloc(2048);
	do
	{
		ret = read(hOut, buf, 1024);
		if( ret != -1 && ret != 0)
		{
			printf(stderr,"write(hResponse,buf,ret: %d \n", ret);
			write(hResponse,buf,ret);
			fsync(hResponse);
		}
	}while(1);
}

int InitThreads()
{
        int err = pthread_create(&hUDPThread, NULL, &MavlinkLoraReader, NULL);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        else
            printf("\n MavlinkLoraReader thread created successfully\n");

        err = pthread_create(&hUDPThread, NULL, &MavlinkOpenHDReader, NULL);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        else
            printf("\n MavlinkOpenHDReader thread created successfully\n");

	err = pthread_create(&hUDPThread, NULL, &ResponseThread, NULL);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        else
            printf("\n ResponseThread created successfully\n");

	return 0;
}



int main(int argc, char *argv[])
{

	hOut = OpenAndConfigure("/dev/pts/4");
	hResponse = OpenAndConfigure("/dev/pts/1");
	InitThreads();
	StartUDPServer();

    return 0;
}
