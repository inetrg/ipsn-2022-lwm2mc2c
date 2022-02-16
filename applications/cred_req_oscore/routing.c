#include "net/ipv6/addr.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/ipv6/nib.h"
#include <errno.h>

#define ROUTE(myid, laddr, toaddr, nhaddr)
#include "routesdown.inc"
#include "routesup.inc"
#undef ROUTE

/* network interface to use */
gnrc_netif_t *mynetif;

/* node hardware address */
uint8_t hwaddr[GNRC_NETIF_L2ADDR_MAXLEN];
/* node hardware address as a string */
char hwaddr_str[GNRC_NETIF_L2ADDR_MAXLEN * 3];

/* node link local and global addresses */
ipv6_addr_t _my_link_local, _my_global;
/* node link local and global addresses as strings */
char _my_link_local_str[IPV6_ADDR_MAX_STR_LEN];
char _my_global_str[IPV6_ADDR_MAX_STR_LEN];

/* iotlab node ID */
uint16_t _my_id;

void install_routes(char *laddr, char *toaddr_str, char *nhaddr_str)
{
    ipv6_addr_t toaddr, nhaddr;
    if(strncmp(laddr, hwaddr_str, strlen(laddr))) {
        /* not for me => bail */
        return;
    }

    printf("Installing route to %s via %s...\n", toaddr_str, nhaddr_str);

    ipv6_addr_from_str(&toaddr, toaddr_str);
    ipv6_addr_from_str(&nhaddr, nhaddr_str);
    int res = gnrc_ipv6_nib_ft_add(&toaddr, IPV6_ADDR_BIT_LEN, &nhaddr, mynetif->pid, 0);

    if (-EINVAL == res) {
        puts("Could not install route, wrong parameters");
    }
    else if (-ENOMEM == res) {
        puts("Not enough memory for new route");
    }
    else if (-ENOTSUP == res) {
        puts("Not supported");
    }
    else if (0 == res) {
        puts("Done");
    }
    else {
        printf("Unknown return code %d\n", res);
    }
}

void get_addr(uint16_t id, char *address)
{
#define MYMAP(ID,ADDR)                                          \
    if (id == ID) {                                             \
        memcpy(address, ADDR, strlen(ADDR)+1);                  \
        return;                                                 \
    }
#include "idaddr.inc"
#undef MYMAP
}

uint16_t get_id(char *address)
{
#define MYMAP(ID,ADDR)                                           \
    if (!memcmp(ADDR, address, strlen(ADDR))) {                  \
        return ID;                                               \
    }
#include "idaddr.inc"
#undef MYMAP
    return 0;
}

void setup_routing(void)
{
    /* find first netif */
    mynetif = gnrc_netif_iter(NULL);

    /* get local HW address */
    uint16_t src_len = 8U;
    gnrc_netapi_set(mynetif->pid, NETOPT_SRC_LEN, 0, &src_len, sizeof(src_len));
#ifdef BOARD_NATIVE
    gnrc_netapi_get(mynetif->pid, NETOPT_ADDRESS, 0, hwaddr, sizeof(hwaddr));
#else
    gnrc_netapi_get(mynetif->pid, NETOPT_ADDRESS_LONG, 0, hwaddr, sizeof(hwaddr));
#endif
    gnrc_netif_addr_to_str(hwaddr, sizeof(hwaddr), hwaddr_str);

    /* get first ipv6 address from netif */
    gnrc_netif_ipv6_addrs_get(mynetif, &_my_link_local, sizeof(_my_link_local));
    ipv6_addr_to_str(_my_link_local_str, &_my_link_local, sizeof(_my_link_local_str));

    ipv6_addr_t _ipv6_prefix;
    ipv6_addr_from_str(&_ipv6_prefix, CONFIG_IPV6_PREFIX);

    /* set global ipv6 address */
    memcpy(&_my_global, &_my_link_local, sizeof(_my_global));
    ipv6_addr_init_prefix(&_my_global, &_ipv6_prefix, CONFIG_IPV6_PREFIX_LEN);
    ipv6_addr_to_str(_my_global_str, &_my_global, sizeof(_my_global_str));
    gnrc_netif_ipv6_addr_add(mynetif, &_my_global, 128, GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID);

    /* get iotlab node ID */
    _my_id = get_id(_my_global_str);

    printf("addr;%u;%s;%s;%s\n", _my_id, hwaddr_str, _my_link_local_str, _my_global_str);

    #define ROUTE(myid, laddr, toaddr, nhaddr) install_routes(laddr, toaddr, nhaddr);
    #include "routesdown.inc"
    #include "routesup.inc"
    #undef ROUTE
}
