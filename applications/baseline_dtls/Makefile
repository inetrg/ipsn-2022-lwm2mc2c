# name of your application
APPLICATION ?= wakaama

# If no BOARD is found in the environment, use this default:
BOARD ?= iotlab-m3

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules
USEMODULE += gnrc_ipv6_router_default
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_icmpv6_echo

USEMODULE += gnrc_netif_events

# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# NOTE: Add the package for wakaama
USEPKG += wakaama
# Extra LwM2M objects
USEMODULE += wakaama_objects_access_control
USEMODULE += wakaama_objects_light_control

# add DTLS support
USEMODULE += wakaama_client_dtls

# Specify sock DTLS implementation
USEPKG += tinydtls
# tinydtls needs crypto secure PRNG
USEMODULE += prng_sha1prng

# dump requests
USEMODULE += od
USEMODULE += od_string

include $(RIOTBASE)/Makefile.include
