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


#include "mavlink/common/mavlink.h"


mavlink_status_t status;
mavlink_message_t msg;

int hOut = 0;

#define BUFLEN 2048
#define PORT 2316
uint8_t outBuffer[BUFLEN];

int ParseMavlink(char *inBuf, int len)
{
	for(int i=0;i<len;i++)
	{
		//(uint8_t)inBuf[i]
		if( mavlink_parse_char(0, inBuf[i], &msg, &status ) )
		{
			//printf("Received message with ID %d, sequence: %d from component %d of system %d \n", msg.msgid, msg.seq, msg.compid, msg.sysid);
			if(msg.msgid == 0 ||  msg.msgid == 2 || msg.msgid == 24  || msg.msgid == 33)
			{
				//printf("msg id: %d \n",  msg.msgid);
				// Copy the message to the send buffer
				uint16_t len = mavlink_msg_to_send_buffer(&outBuffer[0], &msg);
				if(len > 0)
				{
					write(hOut,outBuffer,len);
					fsync(hOut);
					//printf("%d Bytes was send to UART\n",len);
				}
			}
		}
	}
	return -1;
}

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
			printf("recvfrom() error");
		}
		else
		{
			//buf[recv_len] = '\0';
			ParseMavlink(&buf[0], recv_len);
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
                cfsetospeed(&options, B38400);
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


int main(int argc, char *argv[])
{

	hOut = OpenAndConfigure("/dev/serial0");
	StartUDPServer();

    return 0;
}
