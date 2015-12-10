#include "lwip/init.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/ip_addr.h"
#include "lwip/udp.h"
#include "lwip/netif.h"
#include "netif/etharp.h"

#if LWIP_DHCP==1
#include "lwip/dhcp.h"
#endif

#include "arch/lpc_arch.h"
#include "emac_netif.h"

#include "ethernet.h"
#include "s24_eeprom.h"
#include "lutos.h"
#include "task.h"
#include "tick.h"
#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ID_NUM	8
uint8_t ID_ports[] = {0,0,0,0,0,0,0,0};
uint8_t ID_pins[] = {22,21,20,19,18,17,15,16};

#define INIT_PORT			2
#define INIT_PIN			7
//#define INIT_STATUS()		(0==gpio_read(INIT_PORT, INIT_PIN))
#define INIT_STATUS()		1

#define EE_ADDR_IPCFG		0
#define EE_SIZE_IPCFG		16
//#define EE_ADDR_MACCFG		20
#define IPCONFIG_BUFLEN		256

#define IAP_LOCATION 0x1FFF1FF1
typedef void (*IAP)(unsigned int [],unsigned int[]);
IAP iap_entry = (IAP) IAP_LOCATION;
unsigned int iap_command[5];
volatile unsigned int iap_result[5];
TICK *SoftResetTick;

static struct netif my_netif;
struct udp_pcb *ipconfig_upcb;
static volatile uint8_t ipconfig[EE_SIZE_IPCFG+2];
static char ipconfig_buf[IPCONFIG_BUFLEN] __attribute__((aligned (8)));
static char cstrMAC[18];
static const char *cstrKeyword[] = {
	"VIEWMOVE LAN SEARCH",
	"VIEWMOVE LAN QUERY",
	"VIEWMOVE LAN INSPECT",
	"VIEWMOVE LAN CONFIG",
	"DEVICE IP",
	"DEVICE NETMASK",
	"DEVICE GATEWAY",
	"DEVICE MAC"
};
static const char *cstrMessage[] = {
	"SEARCH REPLY",
	"QUERY REPLY",
	"INSPECT REPLY",
	"CONFIG REPLY",
	"TB-1014-M",
	"OK",
	"NG"
};
extern const char *cstrVersion[];

uint16_t checksum_generate(uint8_t *data, uint16_t size)
{
	uint16_t i, sum=0;
	for(i=0; i<size; i++) sum += data[i];
	return sum;
}

uint8_t checksum_verify(uint8_t *data, uint16_t size, uint16_t checksum)
{
	uint8_t result=0;
	uint16_t sum = checksum_generate(data, size);
	if( sum == checksum ) result = 1;
	return result;
}

uint8_t is_numeric(const char *str)
{
	if(*str == '-') ++str;
	if(*str == 0) return 0;
	while(*str)
	{
		if( *str < '0' || *str > '9') return 0;
		else ++str;
	}
	return 1;
}

uint8_t check_netmask(volatile uint8_t *mask)
{
	uint8_t bit_foo = 1, bit_bar;
	uint8_t i, j, rcnt = 0, fcnt = 0;
	
	for(i=0; i<4; i++)
	{
		for(j=0; j<8; j++)
		{
			bit_bar = mask[i] & (1<<(7-j));
			if( bit_foo == 0 && bit_bar != 0 ) rcnt++;
			if( bit_foo != 0 && bit_bar == 0 ) fcnt++;
			bit_foo = bit_bar;
		}
	}
	return (rcnt==0 && fcnt==1);
}

void udp_ipconfig(void *arg, struct udp_pcb *upcb, struct pbuf *p, ip_addr_t *addr, u16_t port)
{
	static char *strbuf;
	const char delim[3] = "\r\n";
	uint32_t *tmp = (uint32_t *)ipconfig_buf;
	uint16_t copy_len, i, changed=0;
	
	if( p->tot_len > IPCONFIG_BUFLEN ) copy_len = IPCONFIG_BUFLEN;
	else copy_len = p->tot_len;
	pbuf_copy_partial(p, ipconfig_buf, copy_len, 0);
	ipconfig_buf[copy_len] = 0;
	strbuf = strtok(ipconfig_buf, delim);
	
	if( 0 == strncmp(ipconfig_buf, cstrKeyword[0], strlen(cstrKeyword[0])) )	// SCAN
	{
		sprintf(ipconfig_buf, "%s\r\n%s\r\n%s", cstrMessage[0], cstrMAC, cstrMessage[4]);
	}
	else if( 0 == strncmp(strbuf, cstrKeyword[1], strlen(cstrKeyword[1])) ||	// QUERY
			 0 == strncmp(strbuf, cstrKeyword[2], strlen(cstrKeyword[2])) )		// INSPECT
	{
		strbuf = strtok(NULL, delim);
		
		if( 0 == strncmp(strbuf, cstrMAC, strlen(cstrMAC)) )	// Check MAC address
		{
			strbuf = ipconfig_buf;
			strbuf += sprintf(strbuf, "%s\r\n%s", cstrMessage[1], cstrMAC);
			for(i=0; i<4; i++)
			{
				strbuf += sprintf(strbuf, "\r\n%s\r\n%d.%d.%d.%d", cstrKeyword[4+i],
								  ipconfig[0+4*i], ipconfig[1+4*i], ipconfig[2+4*i], ipconfig[3+4*i]);
			}
			strbuf += sprintf(strbuf, "\r\n%s\r\n%s", cstrVersion[0], cstrVersion[1]);
		}
		else
		{
			ipconfig_buf[0] = 0;
		}
	}
	else if( 0 == strncmp(strbuf, cstrKeyword[3], strlen(cstrKeyword[3])) )	// CONFIG
	{
		strbuf = strtok(NULL, delim);
		
		if( 0 == strncmp(strbuf, cstrMAC, strlen(cstrMAC)) )	// Check MAC address
		{
			strbuf = strtok(NULL, delim);
			
			while( strbuf != NULL )
			{
				for(i=0; i<4; i++)
				{
					if( 0 == strncmp(strbuf, cstrKeyword[4+i], strlen(cstrKeyword[4+i])) )	// Config Items
					{
						strbuf = strtok(NULL, delim);
						
						if( 4 == sscanf(strbuf, "%u.%u.%u.%u", &tmp[0], &tmp[1], &tmp[2], &tmp[3]) )
						{
							ipconfig[0+i*4] = tmp[0] & 0xff;
							ipconfig[1+i*4] = tmp[1] & 0xff;
							ipconfig[2+i*4] = tmp[2] & 0xff;
							ipconfig[3+i*4] = tmp[3] & 0xff;
							changed = 1;
						}
					}
				}
				strbuf = strtok(NULL, delim);
			}
			
			if( changed && INIT_STATUS() )
			{
				tmp[4] = checksum_generate((uint8_t *)ipconfig, EE_SIZE_IPCFG);
				ipconfig[EE_SIZE_IPCFG] = tmp[4] & 0xff;
				ipconfig[EE_SIZE_IPCFG+1] = (tmp[4]>>8) & 0xff;
				eeprom_busy_wait();
				eeprom_write_block(EE_ADDR_IPCFG, (uint8_t *)ipconfig, EE_SIZE_IPCFG+2);
				sprintf(ipconfig_buf, "%s\r\n%s\r\n%s", cstrMessage[3], cstrMAC, cstrMessage[5]);
				LutosTickEventResume(SoftResetTick);
			}
			else
			{
				sprintf(ipconfig_buf, "%s\r\n%s\r\n%s", cstrMessage[3], cstrMAC, cstrMessage[6]);
			}
		}
		else
		{
			ipconfig_buf[0] = 0;
		}
	}
	else
	{
		ipconfig_buf[0] = 0;
	}
	
	if( strlen(ipconfig_buf) > 0 )
	{
		struct pbuf *q;
		q = pbuf_alloc(PBUF_RAW, strlen(ipconfig_buf), PBUF_RAM);
		pbuf_take(q, ipconfig_buf, q->tot_len);
		udp_sendto(upcb, q, IP_ADDR_BROADCAST, port);
		pbuf_free(q);
	}
	pbuf_free(p);
}

void udp_ipconfig_init(void)
{
	ipconfig_upcb = udp_new();
	
	if( ipconfig_upcb != NULL )
	{
		udp_bind(ipconfig_upcb, IP_ADDR_ANY, 4040);
		udp_recv(ipconfig_upcb, udp_ipconfig, 0);
		ip_set_option(ipconfig_upcb, SOF_BROADCAST);
	}
}

void force_watchdog_reset(void *p)
{
	uint32_t force_time = LutosGetCurrentTicks() + LUTOS_WATCHDOG_TIMEOUT_MS + 500;
	
	while( LutosGetCurrentTicks() < force_time );
}

void ethernet_init(void)
{
	uint16_t *checksum = (uint16_t *)&(ipconfig[EE_SIZE_IPCFG]);
	ip_addr_t ipaddr, netmask, gw;
	
	//gpio_config(INIT_PORT, INIT_PIN, GPIO_IN | GPIO_PUR);
	gpio_array_config(ID_NUM, ID_ports, ID_pins, GPIO_IN | GPIO_PUR);
	iap_command[0] = 58;
	iap_entry(iap_command, (unsigned int *)iap_result);
	
	*checksum = 0xFFFF;
	s24_eeprom_init();
	eeprom_busy_wait();
	eeprom_read_block(EE_ADDR_IPCFG, (void *)ipconfig, EE_SIZE_IPCFG+2);
	if( iap_result[1] == 0xFFFFFFFF ) // 20150108 bugfix: work-around for chip without serial number
	{
		iap_result[1] = *((uint32_t *)(&ipconfig[12]));
	}
	
	SysTick_Enable(1);
	SoftResetTick = LutosTickEventCreate(0, 100,
		LutosTaskCreate(force_watchdog_reset, "", 0, PRIORITY_REALTIME) );
	
	/* Initialize LWIP */
	lwip_init();
	
	if( *checksum == 0 ) /* Dynamic IP assignment */
	{
		IP4_ADDR(&gw, 0, 0, 0, 0);
		IP4_ADDR(&ipaddr, 0, 0, 0, 0);
		IP4_ADDR(&netmask, 0, 0, 0, 0);
	}
	else /* Static IP assignment */
	{
		if( !checksum_verify((uint8_t *)ipconfig, EE_SIZE_IPCFG, *checksum) ||
			ipconfig[0] == 0 || ipconfig[0] == 255 ) /* Load default IP */
		{
			IP4_ADDR(&gw, 192, 168, 2, 254);
			IP4_ADDR(&ipaddr, 192, 168, 2, 40);
			IP4_ADDR(&netmask, 255, 255, 0, 0);
		}
		else
		{
			/* Set IP by DIP-switch */
			if( 0 == ipconfig[3] || 255 == ipconfig[3] )
			{
				ipconfig[3] = ~gpio_array_read(ID_NUM, ID_ports, ID_pins);
			}
			
			/* Check netmask */
			if( !check_netmask(&ipconfig[4]) )
			{
				ipconfig[4] = 255;
				ipconfig[5] = 255;
				ipconfig[6] = 0;
				ipconfig[7] = 0;
			}
			
			/* Check gateway */
			if( ((ipconfig[0] & ipconfig[4]) != (ipconfig[8] & ipconfig[4])) ||
				((ipconfig[1] & ipconfig[5]) != (ipconfig[9] & ipconfig[5])) ||
				((ipconfig[2] & ipconfig[6]) != (ipconfig[10] & ipconfig[6])) ||
				((ipconfig[3] & ipconfig[7]) != (ipconfig[11] & ipconfig[7])) )
			{
				ipconfig[8] = 255;
				ipconfig[9] = 255;
				ipconfig[10] = 255;
				ipconfig[11] = 255;
			}
			IP4_ADDR(&gw, ipconfig[8], ipconfig[9], ipconfig[10], ipconfig[11]);
			IP4_ADDR(&ipaddr, ipconfig[0], ipconfig[1], ipconfig[2], ipconfig[3]);
			IP4_ADDR(&netmask, ipconfig[4], ipconfig[5], ipconfig[6], ipconfig[7]);
		}
		ipconfig[0] = (ipaddr.addr>>0) & 0xFF;
		ipconfig[1] = (ipaddr.addr>>8) & 0xFF;
		ipconfig[2] = (ipaddr.addr>>16) & 0xFF;
		ipconfig[3] = (ipaddr.addr>>24) & 0xFF;
		ipconfig[4] = (netmask.addr>>0) & 0xFF;
		ipconfig[5] = (netmask.addr>>8) & 0xFF;
		ipconfig[6] = (netmask.addr>>16) & 0xFF;
		ipconfig[7] = (netmask.addr>>24) & 0xFF;
		ipconfig[8] = (gw.addr>>0) & 0xFF;
		ipconfig[9] = (gw.addr>>8) & 0xFF;
		ipconfig[10] = (gw.addr>>16) & 0xFF;
		ipconfig[11] = (gw.addr>>24) & 0xFF;
	}
	
	/* Set the MAC Address of netif interface */
	my_netif.hwaddr[0] = 0x00;
	my_netif.hwaddr[1] = 0x60;
	my_netif.hwaddr[2] = 0x37;
	my_netif.hwaddr[3] = (iap_result[1]>>16)&0xff;
	my_netif.hwaddr[4] = (iap_result[1]>>8)&0xff;
	my_netif.hwaddr[5] = (iap_result[1]>>0)&0xff;
	
	sprintf(cstrMAC, "%02X:%02X:%02X:%02X:%02X:%02X",
		my_netif.hwaddr[0],
		my_netif.hwaddr[1],
		my_netif.hwaddr[2],
		my_netif.hwaddr[3],
		my_netif.hwaddr[4],
		my_netif.hwaddr[5]
	);
	
	/* Add netif interface */
	netif_add(&my_netif, &ipaddr, &netmask, &gw, NULL, emac_netif_init,
			  ethernet_input);
	netif_set_default(&my_netif);
	netif_set_up(&my_netif);
	
	if( *checksum == 0 ) dhcp_start(&my_netif);
	udp_ipconfig_init();
}
