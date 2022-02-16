#include "net/ipv6/addr.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/ipv6/nib.h"
#include <errno.h>

#define ROUTE(myid, laddr, toaddr, nhaddr)
#include "routesdown.inc"
#include "routesup.inc"
#undef ROUTE

#define BORDER_ROUTER_WIRED_LINUX_ADDR "fe80::1"
#define BORDER_ROUTER_WIRED_LOCAL_ADDR "fe80::2"

/* network interface to use */
netif_t *wired, *wireless = NULL;
gnrc_netif_t *wireless_gnrc, *wired_gnrc;

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
    int res = gnrc_ipv6_nib_ft_add(&toaddr, IPV6_ADDR_BIT_LEN, &nhaddr, wireless_gnrc->pid, 0);

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
    printf("Getting ID for address %s\n", address);
#define MYMAP(ID,ADDR)                                           \
    printf("%s\n",ADDR); \
    if (!memcmp(ADDR, address, strlen(ADDR))) {                  \
        return ID;                                               \
    }
#include "idaddr.inc"
#undef MYMAP
    return 0;
}

void setup_routing(void)
{
    /* find wired and wireless interfaces */
    netif_t *netif = NULL;

    while ((netif = netif_iter(netif))) {
            int is_wired = netif_get_opt(netif, NETOPT_IS_WIRED, 0, NULL, 0);
            // NOTE: Assuming only 2 interfaces here
            if (is_wired < 0) {
                wireless = netif;
            }
            else {
                wired = netif;
            }
    };

    wireless_gnrc = container_of(wireless, gnrc_netif_t, netif);
    wired_gnrc = container_of(wired, gnrc_netif_t, netif);

    if (IS_ACTIVE(CONFIG_USE_CUSTOM_DATA_RATE)) {
        printf("app;dr;%d\n",CONFIG_DATA_RATE);

        uint8_t dr = CONFIG_DATA_RATE;
        if (netif_set_opt(wireless, NETOPT_OQPSK_RATE, 0, &dr, sizeof(uint8_t)) < 0) {
            puts("app;dr;err");
        }
    }


    /* get local HW address */
    uint16_t src_len = 8U;
    gnrc_netapi_set(wireless_gnrc->pid, NETOPT_SRC_LEN, 0, &src_len, sizeof(src_len));
    gnrc_netapi_get(wireless_gnrc->pid, NETOPT_ADDRESS_LONG, 0, hwaddr, sizeof(hwaddr));
    gnrc_netif_addr_to_str(hwaddr, sizeof(hwaddr), hwaddr_str);

    /* get first ipv6 address from netif */
    gnrc_netif_ipv6_addrs_get(wireless_gnrc, &_my_link_local, sizeof(_my_link_local));
    ipv6_addr_to_str(_my_link_local_str, &_my_link_local, sizeof(_my_link_local_str));

    ipv6_addr_t _ipv6_prefix;
    ipv6_addr_from_str(&_ipv6_prefix, CONFIG_IPV6_PREFIX);

    /* set global ipv6 address */
    memcpy(&_my_global, &_my_link_local, sizeof(_my_global));
    ipv6_addr_init_prefix(&_my_global, &_ipv6_prefix, CONFIG_IPV6_PREFIX_LEN);
    ipv6_addr_to_str(_my_global_str, &_my_global, sizeof(_my_global_str));
    gnrc_netif_ipv6_addr_add(wireless_gnrc, &_my_global, 128, GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID);

    /* for the wired interface set <prefix>::1 */
    ipv6_addr_t wired_global;
    ipv6_addr_from_str(&wired_global, "::1");
    ipv6_addr_init_prefix(&wired_global, &_ipv6_prefix, CONFIG_IPV6_PREFIX_LEN);
    gnrc_netif_ipv6_addr_add(wired_gnrc, &wired_global, 128, GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID);

    /* for the wired interface 'default' goes through linux tap address */
    ipv6_addr_t wired_addr;
    ipv6_addr_from_str(&wired_addr, BORDER_ROUTER_WIRED_LINUX_ADDR);
    gnrc_ipv6_nib_ft_add(NULL, 0, &wired_addr, wired_gnrc->pid, 0);

    ipv6_addr_from_str(&wired_addr, BORDER_ROUTER_WIRED_LOCAL_ADDR);
    gnrc_netif_ipv6_addr_add(wired_gnrc, &wired_addr, 128, GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID);

    /* get iotlab node ID */
    _my_id = get_id(_my_global_str);

    printf("addr;%u;%s;%s;%s\n", _my_id, hwaddr_str, _my_link_local_str, _my_global_str);

    #define ROUTE(myid, laddr, toaddr, nhaddr) install_routes(laddr, toaddr, nhaddr);
    #include "routesdown.inc"
    #include "routesup.inc"
    #undef ROUTE
}
