#include "ets_sys.h"
#include "osapi.h"
#include "ip_addr.h"
#include "espconn.h"
#include "driver/vga.h"
#include "draw.h"
#include "net.h"

struct espconn server;
esp_tcp esptcp;

int x, y;

static void ICACHE_FLASH_ATTR
server_sent_cb(void *arg)
{
	//data sent successfully
	os_printf("tcp sent cb \r\n");
}

static void ICACHE_FLASH_ATTR new_line()
{
	if (y + 16 >= 480)
		vga_scroll_down(16);
	else
		y += 16;
}
 
static void ICACHE_FLASH_ATTR
server_recv_cb(void *arg, char *data, unsigned short len)
{
	//received some data from tcp connection
	int i;
	struct espconn *pespconn = arg;

	for (i = 0; i < len; i++) {
		if (data[i] >= 0xa0 && i < len - 1) {
			if (x + 16 > 512) {
				x = 0;
				new_line();
			}
			draw_gb(x, y, data[i], data[i+1]);
			x += 16;
			i++;
		} else if (data[i] == '\n') {
			x = 0;
			new_line();
		} else {
			if (x + 8 > 512) {
				x = 0;
				new_line();
			}			
			draw_asc(x, y, data[i]);
			x += 8;
		}
		if (x >= 512) {
			x = 0;
			new_line();
		}
	}
}


static void ICACHE_FLASH_ATTR
server_discon_cb(void *arg)
{
	//tcp disconnect successfully
	os_printf("tcp disconnect succeed !!! \r\n");
}


static void ICACHE_FLASH_ATTR
server_recon_cb(void *arg, sint8 err)
{
	//error occured , tcp connection broke.
	os_printf("reconnect callback, error code %d !!! \r\n",err);
}

static void ICACHE_FLASH_ATTR
server_listen(void *arg)
{
	struct espconn *pesp = arg;
	os_printf("server_listen\n");

	espconn_regist_recvcb(pesp, server_recv_cb);
	espconn_regist_reconcb(pesp, server_recon_cb);
	espconn_regist_disconcb(pesp, server_discon_cb);
	espconn_regist_sentcb(pesp, server_sent_cb);
}

void  ICACHE_FLASH_ATTR
net_init()
{
	espconn_create(&server);
	server.type = ESPCONN_TCP;
	server.state = ESPCONN_NONE;
	server.proto.tcp = &esptcp;
	server.proto.tcp->local_port = 88;
	espconn_regist_connectcb(&server, server_listen);

	int ret = espconn_accept(&server);
	os_printf("espconn_accept: %d\n", ret);
}
