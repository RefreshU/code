/********************************************************************************
*	server : Obtain the information of device, and input it to the 
*	file "/tmp/devcie_info".
*********************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <errno.h>
#include <sys/ioctl.h>

#define BUFFER_LEN 64
#define IP_FOUND "IP_FOUND"
#define MATCH_INFO "Analog"
#define IFNAME "eth0"
#define MCAST_PORT 8888

struct  Device_Info
{
    char device_ip[20];
    char device_num[8];
    char device_mac[20];
    char info[10];
};

int main()
{
    int ret = -1;
    int sock = -1;
    int count = -1;
    int from_len;
    char all_msg[10][64];
    char *temp_str = NULL;
    char filename[]= "/tmp/device_info.txt";
    int so_broadcast = 1;
    struct Device_Info *device;    

    struct ifreq ifr;
    struct sockaddr_in broadcast_addr;		/* local address*/
    struct sockaddr_in from_addr;			/* client address*/
    struct timeval timeout;

    fd_set readfd;
    char buff[BUFFER_LEN];
    char device_info[64];

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
	
	/*
	**	Creating a new text file with 
	**  the saved information of gateway device 
	*/
    int fd = open(filename, O_RDWR);
    system("cat /dev/null > /tmp/device_info.txt");

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        printf("socket init error ...\n");
        return 0;
    }
    strcpy(ifr.ifr_name, IFNAME);

    if(ioctl(sock, SIOCGIFBRDADDR, &ifr) == -1)
    {
        perror("ioctl error ...");
        exit(1);
    }

    /*
	**initializing the socket
	*/

    memcpy(&broadcast_addr, &ifr.ifr_broadaddr, sizeof(struct sockaddr_in));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcast_addr.sin_port = htons(MCAST_PORT);

	/*
	**	setting the type of socket is broadcast
	*/
    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &so_broadcast, sizeof(so_broadcast));
    if(ret == -1)
    {
        printf("Fail to set socket..\n ");
        return 0;
    }
    int times = 10;
    int i, j;
    for(i = 0; i < times; i++)
    {   
        /*
		** server send the broadcast to client of AnalogGateway
		*/
        ret = sendto(sock, IP_FOUND, strlen(IP_FOUND), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
        if( ret == -1)
            continue;
        FD_ZERO(&readfd);
        FD_SET(sock, &readfd);
        ret = select(sock+1, &readfd, NULL, NULL, &timeout);
        switch(ret)
        {
            case -1:
                printf("error!!\n");
                break;
            case 0:
                printf("timeout ....\n");
                break;
            default:
                if(FD_ISSET(sock, &readfd))
                {   
                    /*
					**Recieving information of device from AnalogGateway of client 
					*/
                    memset(buff, 0, BUFFER_LEN);
                    count = recvfrom(sock, buff, BUFFER_LEN, 0, (struct sockaddr*)&from_addr, &from_len);
                    device = (struct Device_Info *)&buff;
					
					/*
					**	judging whether the data packet from the gateway device 
					*/
                    if(strstr(device->info, MATCH_INFO))
                    {
                        sprintf(device_info, "Device IP:%s Device MAC:%s\n", device->device_ip, device->device_mac);
                        memcpy(all_msg[i], device_info, 64);
                    }
                break;
                }
             memset(device, 0, sizeof(struct Device_Info));
        }
        sleep(5);
    }

/*
** Deleting the repeated information of device,  
** and it is written to the file of "/tmp/devcie_info.txt"
*/
    if(fd < 0)
    {
        printf("fail to open the file %s\n", filename);
    }
    else{
            for(i = 0; i < times; i++)
            {
                temp_str = all_msg[i];
                for(j = i+1; j < times; j++)
                {
                    if(strstr(temp_str, all_msg[j]))
                    {
                        all_msg[j][0] = '\0';
                    }
                }
                write(fd, all_msg[i], strlen(all_msg[i]));
            }
        }

    return 0;
}

