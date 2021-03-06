#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <getopt.h>

#include<arpa/inet.h>
#include<sys/socket.h>

#include "mavlink/common/mavlink.h"

#define SERVER "127.0.0.1"
#define BUFLEN 2  //Max length of buffer
#define PORT 1257   //The port on which to send data
#define PORTRTP 1256
#define PORTIRCUT 1255

int main(int argc, char *argv[])
{
	char fBrokenSocket = 0;
	uint8_t buf[402];  // data read from stdin
	mavlink_status_t status;
	mavlink_message_t msg;
	
	uint16_t chValue;
	uint16_t chValue2;
	uint16_t chValue3;
	int param_telemetry_protocol = 0;

	int ChannelToListen = atoi(argv[1]);
	int ChannelToListenRTPRecord = atoi(argv[2]);

	int ChannelToListenIRCut = atoi(argv[3]);

	//UDP init
	   struct sockaddr_in si_other;
	int s, i, slen = sizeof(si_other);
	char message[BUFLEN];
 
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		perror(s);
		exit(1);
	}
 
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
     
	if (inet_aton(SERVER, &si_other.sin_addr) == 0) 
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	//udp init end

	//udp init RTP to USB saver

	struct sockaddr_in si_other2;
        int s2, i2, slen2 = sizeof(si_other2);
        char message2[BUFLEN];

        if ((s2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        {
                perror(s2);
                exit(1);
        }

        memset((char *) &si_other2, 0, sizeof(si_other2));
        si_other2.sin_family = AF_INET;
        si_other2.sin_port = htons(PORTRTP);

        if (inet_aton(SERVER, &si_other2.sin_addr) == 0)
        {
                fprintf(stderr, "inet_aton() failed\n");
                exit(1);
        }
	//end init end

        //udp init IR cut

        struct sockaddr_in si_other3;
        int s3, i3, slen3 = sizeof(si_other3);
        char message3[BUFLEN];

        if ((s3 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        {
                perror(s3);
                exit(1);
        }

        memset((char *) &si_other3, 0, sizeof(si_other3));
        si_other3.sin_family = AF_INET;
        si_other3.sin_port = htons(PORTIRCUT);

        if (inet_aton(SERVER, &si_other3.sin_addr) == 0)
        {
                fprintf(stderr, "inet_aton() failed\n");
                exit(1);
        }
        //end init end

	
	while(!fBrokenSocket)
	{
		int inl = read(STDIN_FILENO, buf, 350);   // read the data		

		if(inl < 0) { return 1; }
		if (inl > 350) {  continue; }
		if (inl == 0) { usleep(1e5); continue;} // EOF
		if(inl > 0)
		{
			write(STDOUT_FILENO, &buf, inl);
		}
		if(param_telemetry_protocol == 0)
		{
			 // parse Mavlink
			int i = 0;
			for (i = 0; i < inl; i++)
			{
				uint8_t c = buf[i];
				if (mavlink_parse_char(0, c, &msg, &status))
				{
					switch (msg.msgid)
					{
						//printf( "mavlink id: %d \n", msg.msgid);
						case MAVLINK_MSG_ID_RC_CHANNELS:
							{
								//mavlink_rc_channels_t chinfo;	
								//mavlink_msg_rc_channels_decode(&msg, &chinfo);
								//printf("MAVLINK_MSG_ID_RC_CHANNELS\n");
								if(ChannelToListen >= 1 && ChannelToListen <= 18)
								{
									//printf(" in range 1 - 18\n");
									if (ChannelToListen == 1)  { chValue = mavlink_msg_rc_channels_get_chan1_raw(&msg); }
									if (ChannelToListen == 2) { chValue = mavlink_msg_rc_channels_get_chan2_raw(&msg); }
									if (ChannelToListen == 3) { chValue = mavlink_msg_rc_channels_get_chan3_raw(&msg); }
									if (ChannelToListen == 4) { chValue = mavlink_msg_rc_channels_get_chan4_raw(&msg); }
									if (ChannelToListen == 5) { chValue = mavlink_msg_rc_channels_get_chan5_raw(&msg); }
									if (ChannelToListen == 6) { chValue = mavlink_msg_rc_channels_get_chan6_raw(&msg); }
									if (ChannelToListen == 7) { chValue = mavlink_msg_rc_channels_get_chan7_raw(&msg); }
									if (ChannelToListen == 8) { chValue = mavlink_msg_rc_channels_get_chan8_raw(&msg); }
									if (ChannelToListen == 9) { chValue = mavlink_msg_rc_channels_get_chan9_raw(&msg); }
									if (ChannelToListen == 10) { chValue = mavlink_msg_rc_channels_get_chan10_raw(&msg); }
									if (ChannelToListen == 11) { chValue = mavlink_msg_rc_channels_get_chan11_raw(&msg); }
									if (ChannelToListen == 12) { chValue = mavlink_msg_rc_channels_get_chan12_raw(&msg); }
									if (ChannelToListen == 13) { chValue = mavlink_msg_rc_channels_get_chan13_raw(&msg); }
									if (ChannelToListen == 14) { chValue = mavlink_msg_rc_channels_get_chan14_raw(&msg); }
									if (ChannelToListen == 15) { chValue = mavlink_msg_rc_channels_get_chan15_raw(&msg); }
									if (ChannelToListen == 16) { chValue = mavlink_msg_rc_channels_get_chan16_raw(&msg); }
									if (ChannelToListen == 17) { chValue = mavlink_msg_rc_channels_get_chan17_raw(&msg); }
									if (ChannelToListen == 18) { chValue = mavlink_msg_rc_channels_get_chan18_raw(&msg); }
												
									//int sizeinbyte = sizeof(ChannelToListen);
									
									//unsigned int	under RPi2 = 2 byte
								
									message[0] = chValue & 0xFF;
									message[1] = chValue >> 8;
								
									sendto(s, message, 2, 0, (struct sockaddr *) &si_other, slen);
								}

								if( ChannelToListenRTPRecord >= 1 && ChannelToListenRTPRecord <= 18)
                                                                {
                                                                        //printf(" in range 1 - 18\n");
                                                                        if (ChannelToListenRTPRecord == 1)  { chValue2 = mavlink_msg_rc_channels_get_chan1_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 2) { chValue2 = mavlink_msg_rc_channels_get_chan2_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 3) { chValue2 = mavlink_msg_rc_channels_get_chan3_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 4) { chValue2 = mavlink_msg_rc_channels_get_chan4_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 5) { chValue2 = mavlink_msg_rc_channels_get_chan5_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 6) { chValue2 = mavlink_msg_rc_channels_get_chan6_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 7) { chValue2 = mavlink_msg_rc_channels_get_chan7_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 8) { chValue2 = mavlink_msg_rc_channels_get_chan8_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 9) { chValue2 = mavlink_msg_rc_channels_get_chan9_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 10) { chValue2 = mavlink_msg_rc_channels_get_chan10_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 11) { chValue2 = mavlink_msg_rc_channels_get_chan11_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 12) { chValue2 = mavlink_msg_rc_channels_get_chan12_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 13) { chValue2 = mavlink_msg_rc_channels_get_chan13_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 14) { chValue2 = mavlink_msg_rc_channels_get_chan14_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 15) { chValue2 = mavlink_msg_rc_channels_get_chan15_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 16) { chValue2 = mavlink_msg_rc_channels_get_chan16_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 17) { chValue2 = mavlink_msg_rc_channels_get_chan17_raw(&msg); }
                                                                        if (ChannelToListenRTPRecord == 18) { chValue2 = mavlink_msg_rc_channels_get_chan18_raw(&msg); }

                                                                        //int sizeinbyte = sizeof(ChannelToListen);

                                                                        //unsigned int  under RPi2 = 2 byte

                                                                        message2[0] = chValue2 & 0xFF;
                                                                        message2[1] = chValue2 >> 8;

                                                                        sendto(s2, message2, 2, 0, (struct sockaddr *) &si_other2, slen2);
                                                                }

                                                                if( ChannelToListenIRCut >= 1 && ChannelToListenIRCut <= 18)
                                                                {
                                                                        //printf(" in range 1 - 18\n");
                                                                        if (ChannelToListenIRCut == 1)  { chValue3 = mavlink_msg_rc_channels_get_chan1_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 2) { chValue3 = mavlink_msg_rc_channels_get_chan2_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 3) { chValue3 = mavlink_msg_rc_channels_get_chan3_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 4) { chValue3 = mavlink_msg_rc_channels_get_chan4_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 5) { chValue3 = mavlink_msg_rc_channels_get_chan5_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 6) { chValue3 = mavlink_msg_rc_channels_get_chan6_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 7) { chValue3 = mavlink_msg_rc_channels_get_chan7_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 8) { chValue3 = mavlink_msg_rc_channels_get_chan8_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 9) { chValue3 = mavlink_msg_rc_channels_get_chan9_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 10) { chValue3 = mavlink_msg_rc_channels_get_chan10_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 11) { chValue3 = mavlink_msg_rc_channels_get_chan11_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 12) { chValue3 = mavlink_msg_rc_channels_get_chan12_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 13) { chValue3 = mavlink_msg_rc_channels_get_chan13_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 14) { chValue3 = mavlink_msg_rc_channels_get_chan14_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 15) { chValue3 = mavlink_msg_rc_channels_get_chan15_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 16) { chValue3 = mavlink_msg_rc_channels_get_chan16_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 17) { chValue3 = mavlink_msg_rc_channels_get_chan17_raw(&msg); }
                                                                        if (ChannelToListenIRCut == 18) { chValue3 = mavlink_msg_rc_channels_get_chan18_raw(&msg); }

                                                                        //int sizeinbyte = sizeof(ChannelToListen);

                                                                        //unsigned int  under RPi2 = 2 byte

                                                                        message3[0] = chValue3 & 0xFF;
                                                                        message3[1] = chValue3 >> 8;

                                                                        sendto(s3, message3, 2, 0, (struct sockaddr *) &si_other3, slen3);
                                                                }


							}

							break;
						default:
							break;
					}
				}
			}
		}
	
	}
		
	close(s);
	close(s2);
	return 0;
}
