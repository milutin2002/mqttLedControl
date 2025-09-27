// lwipopts.h – minimal config for Pico W + FreeRTOS + sockets + MQTT

#ifndef LWIPOPTS_H
#define LWIPOPTS_H

// ----- OS integration -----
#define NO_SYS                 0
#define SYS_LIGHTWEIGHT_PROT   1

// ----- Memory pools (tweak if you see OOM) -----
#define MEM_ALIGNMENT          4
#define MEM_SIZE               (70 * 1024)

#define MEMP_NUM_TCP_PCB       8
#define MEMP_NUM_UDP_PCB       4
#define MEMP_NUM_TCP_SEG       32
#define PBUF_POOL_SIZE         16
#define PBUF_POOL_BUFSIZE      1536

// ----- TCP/IP -----
#define LWIP_TCP               1
#define TCP_MSS                1460
#define TCP_SND_BUF            (8 * TCP_MSS)
#define TCP_WND                (8 * TCP_MSS)

#define LWIP_UDP               1
#define LWIP_ICMP              1
#define LWIP_DNS               1
#define LWIP_DHCP              1

// ----- Sockets / Netconn (handy for tests) -----
#define LWIP_SOCKET            1
#define LWIP_NETCONN           1

// ----- Checksum (use hardware/driver where possible) -----
#define CHECKSUM_BY_HARDWARE   0

#define LWIP_TIMERS              1

#define MEMP_NUM_SYS_TIMEOUT     32 

// ----- MQTT app -----
#define LWIP_ALTCP             0
#define LWIP_ALTCP_TLS         0
#define LWIP_MDNS_RESPONDER    0

// ----- Thread settings (FreeRTOS) -----
#define TCPIP_THREAD_STACKSIZE     2048
#define DEFAULT_THREAD_STACKSIZE   1024
#define TCPIP_MBOX_SIZE            16
#define DEFAULT_RAW_RECVMBOX_SIZE  8
#define DEFAULT_UDP_RECVMBOX_SIZE  8
#define DEFAULT_TCP_RECVMBOX_SIZE  8
#define DEFAULT_ACCEPTMBOX_SIZE    8

#endif /* LWIPOPTS_H */
