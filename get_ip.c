******************************************************************************************
*Get device ip address and mac addres ,then by function sendto() to client in struct;
*
*
/************************************************************************************************
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>
#include <time.h>
#include <netdb.h>
#include <sys/vfs.h>
#include <ifaddrs.h>
#include <signal.h>

#define BUFFER_LEN 32
#define IP_FOUND "IP_FOUND"
#define IP_FOUND_ACK "ACK"
#define IFNAME "eth0"
#define MATCH_INFO "Analog"
#define MCAST_PORT 8888

struct Device_Info
{
    char device_ip[20];
    char device_num[8];
    char device_mac[20];
    char info[10];
};

char* get_local_ip_1(void)
{
	struct ifreq ifr;
	size_t if_name_len=strlen(IFNAME);
	if (if_name_len<sizeof(ifr.ifr_name)) 
	{
		memcpy(ifr.ifr_name, IFNAME, if_name_len);
		ifr.ifr_name[if_name_len]=0;
	} else 
	{
        printf("interface name is too long\n");
	}

	int fd=socket(AF_INET,SOCK_DGRAM,0);
	if (fd==-1)
	{
		printf("%s\n",strerror(errno));
	}

	if (ioctl(fd,SIOCGIFADDR,&ifr)==-1) 
	{
		int temp_errno=errno;
		close(fd);
		printf("%s\n",strerror(temp_errno));
	}
	close(fd);

	struct sockaddr_in* ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
	printf("IP address: %s\n", inet_ntoa(ipaddr->sin_addr));
	
	return inet_ntoa(ipaddr->sin_addr);
}

char* get_local_mac(void)
{ 
	struct ifreq ifr;
    char local_mac[18];
	size_t if_name_len=strlen(IFNAME);
	if (if_name_len<sizeof(ifr.ifr_name))
	{
		memcpy(ifr.ifr_name, IFNAME, if_name_len);
		ifr.ifr_name[if_name_len]=0;
	} 
	else
	{
		printf("interface name is too long\n");
	}

	int fd=socket(AF_UNIX,SOCK_DGRAM,0);
	if (fd==-1)
	{
		printf("%s\n",strerror(errno));
	}

	if (ioctl(fd,SIOCGIFHWADDR,&ifr)==-1) 
	{
		int temp_errno=errno;
		close(fd);
		printf("%s\n",strerror(temp_errno));
	}
	close(fd);
	
	if (ifr.ifr_hwaddr.sa_family!=ARPHRD_ETHER)
	{
		printf("not an Ethernet interface\n");
	}
	
	const unsigned char* mac=(unsigned char*)ifr.ifr_hwaddr.sa_data;
	int n = sprintf(local_mac, "%02X:%02X:%02X:%02X:%02X:%02X",
   	mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	printf("local mac:%s count:%d", local_mac, n);

	return local_mac;
}

char* get_local_ip(void)
{
    struct ifaddrs *ifAddrStruct = NULL;
    void * tmpAddrPtr = NULL;
    getifaddrs(&ifAddrStruct);
    
     while(ifAddrStruct != NULL)
	{
		if(ifAddrStruct->ifa_addr->sa_family == AF_INET)
		{
			tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			if(strstr( ifAddrStruct->ifa_name,IFNAME))
			{
			//	printf("%s IPV4 Address is %s\n",ifAddrStruct->ifa_name,addressBuffer);
				return addressBuffer;
			}
		}
	/*	else if(ifAddrStruct->ifa_addr->sa_family == AF_INET6)
		{
			tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			char addressBuffer[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, tmpAddrPtr, ifAddrStruct->ifa_name, addressBuffer);
			printf("%s IPV6 Address is %s\n", ifAddrStruct->ifa_name, addressBuffer);			
		}
	*/	
  	   ifAddrStruct = ifAddrStruct->ifa_next;
	}
	return NULL;
}

int main(void *arg)
{
    int ret = -1;
    int sock = -1;
    struct Device_Info device;
    unsigned char *ptr = NULL;

    struct sockaddr_in local_addr;
    struct sockaddr_in from_addr;

    int from_len = sizeof(struct sockaddr_in);
    int count = -1;
    fd_set readfd;
    char buff[BUFFER_LEN];
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    
    memset(device.device_ip, 0, sizeof(device.device_ip));
    memset(device.device_mac, 0, sizeof(device.device_mac));
    memset(device.device_num, 0, sizeof(device.device_num));
    memset(device.info, 0, sizeof(device.info));    
 
    memcpy(device.device_ip, get_local_ip(), 20);
    memcpy(device.device_mac, get_local_mac(), 20);
    memcpy(device.device_num, "AG172", sizeof("AG172"));
    memcpy(device.info, MATCH_INFO, sizeof(MATCH_INFO));

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if( sock < 0 )
    {
        printf("HandleIPFound: socket init error!\n");
        return 0;
    }
    memset((void*)&local_addr, 0, sizeof(struct sockaddr_in));

    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(MCAST_PORT);

    printf("local IP is %s\n",inet_ntoa(local_addr.sin_addr));
    ret = bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr));
    if(ret != 0)
    {
        printf("HandleIPFound:bind error...\n");
        return 0;
    }
    while(1)
    {
        FD_ZERO(&readfd);
        FD_SET(sock, &readfd);
        ret = select(sock+1, &readfd, NULL, NULL, &timeout);
        switch(ret)
        {
            case -1:
                break;
            case  0:
                break;
            default:
                if( FD_ISSET( sock, &readfd ))
                {
					memset(buff, 0, BUFFER_LEN);
                    count = recvfrom(sock, buff, BUFFER_LEN, 0, (struct sockaddr*)&local_addr, &from_len);
                    printf("Recv msg is %s,count=%d\n", buff, count);
                    if( strstr(buff, IP_FOUND))
                    {
			
                       // memcpy(buff, (char *)&device, strlen(device));
						printf("Now buff is %s\n", buff);
						printf("local ip is %s\n", device.device_ip);
						printf("local mac is: %s\n", device.device_mac);
					//	ptr = (unsigned char *)&device;
						count = sendto(sock, (struct Device_Info *)&device, sizeof(device), 0, (struct sockaddr*)&local_addr, from_len);
					  //count = sendto(sock, ptr, sizeof(device), 0, (struct sockaddr*)&local_addr, from_len);
						printf("count = %d\n",count);
                    }
                }
        }
    }

    return 0;
}
