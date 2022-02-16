# name of your application
APPLICATION = gnrc_border_router

# If no BOARD is found in the environment, use this default:
BOARD ?= iotlab-m3

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

# Default to using ethos for providing the uplink when not on native
UPLINK := ethos

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for 6LoWPAN border router
USEMODULE += gnrc_sixlowpan_border_router_default
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

USEMODULE += stdio_ethos

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Ethos/native TAP interface and UHCP prefix can be configured from make command
TAP ?= tap0
ifeq (1,$(USE_DHCPV6))
  # with DHCPv6 the 64-bit delegated prefixes are generated from a shorter
  # configured prefix.
  IPV6_PREFIX ?= 2001:db8::/32
else
  # UHCP advertises the prefix as is.
  IPV6_PREFIX ?= 2001:db8::/64
endif

# MODULE DEPENDENT CONFIGURATION IMPORT
# =====================================
# use ethos (ethernet over serial) for network communication and stdio over UART
# ethos baudrate can be configured from make command
ETHOS_BAUDRATE ?= 115200
include $(CURDIR)/Makefile.ethos.conf

SHOULD_RUN_KCONFIG ?=1
ETHOS_BAUDRATE=500000

CFLAGS += -DDEBUG_ASSERT_VERBOSE

include $(RIOTBASE)/Makefile.include

# Compile-time configuration for DHCPv6 client (needs to come after
# Makefile.include as this might come from Kconfig)
ifeq (1,$(USE_DHCPV6))
  ifndef CONFIG_GNRC_DHCPV6_CLIENT_6LBR_STATIC_ROUTE
    ifeq (1,$(STATIC_ROUTES))
      CFLAGS += -DCONFIG_GNRC_DHCPV6_CLIENT_6LBR_STATIC_ROUTE=1
      # CONFIG_GNRC_DHCPV6_CLIENT_6LBR_STATIC_ROUTE=1 requires one more address
      # for `fe80::2`.
      CFLAGS += -DCONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF=3
    endif
  endif
endif


.PHONY: host-tools

host-tools:
	$(Q)env -u CC -u CFLAGS $(MAKE) -C $(RIOTTOOLS)

# Set a custom channel if needed
include $(RIOTMAKE)/default-radio-settings.inc.mk
