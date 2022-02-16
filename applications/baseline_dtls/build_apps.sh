#!/bin/bash

# bring testbed nodes

# select which topology file to use (see routes/)
TOPOLOGY="multihop_0"

source ./nodes_testbed.sh
source ./routes/${TOPOLOGY}.sh

local_ipv6_prefix="fe80::"

# sanity configuration check
if [ ! -v CONFIG[site] ] || [ ! -v CONFIG[board] ] || [ ! -v CONFIG[global_prefix] ];
then
    echo "Incorrect configuration"
    exit 1
fi

# remove generated files
rm -f *.inc
rm -f *.elf
rm -f ../border_router/*.inc

# checks if a firmware profile has already been built
declare -A FW_FLAGS
declare -A GLOBAL_ADDRS

#  generates
#  ROUTE(<node_id>, <node_hw_addr>, <dest_global>, <next_hop>)
#
for i in "${ROUTES[@]}"; do
    set -- $i
    node=$1; dest=$2; next=$3; dir=$4;
    set -- ${NODES[$node]}
    hwaddr=$1;
    set -- ${NODES[$dest]}
    gaddr=${2/#fe80/2001:db8}
    global_ip=${2/#$local_ipv6_prefix/"${CONFIG[global_prefix]}":}
    set -- ${NODES[$next]}
    naddr=$2
    printf "ROUTE(${node#tap},\"${hwaddr}\",\"${global_ip}\",\"${naddr}\")\n" >> routes${dir}.inc
    printf "ROUTE(${node#tap},\"${hwaddr}\",\"${global_ip}\",\"${naddr}\")\n" >> ../border_router/routes${dir}.inc
done

#  generates
#  MYMAP(<node_id>, <node_global>)
#
for i in "${!NODES[@]}"; do
    set -- ${NODES[$i]}
    global_ip=${2/#$local_ipv6_prefix/"${CONFIG[global_prefix]}":}
    GLOBAL_ADDRS[$i]=$global_ip
    printf "MYMAP(${i#tap},\"${global_ip}\")\n" >> idaddr.inc
    printf "MYMAP(${i#tap},\"${global_ip}\")\n" >> ../border_router/idaddr.inc
done

# add the other nodes IP in configuration file
for p in "${PROFILES[@]}"; do
    set -- $p
    this_node=$1
    config=$2
    other_node=$3

    # border router is in a different directory, the config route is different
    if [ $config = "border_router" ];
    then
        config_file=../border_router/$config.config
    else
        config_file=./$config.config
    fi

    # configure the ipv6 prefix, depends on the experiment configuration
    sed -i '/CONFIG_IPV6_PREFIX/d' $config_file
    sed -i '/CONFIG_IPV6_PREFIX_LEN/d' $config_file
    echo "CONFIG_IPV6_PREFIX=\"${CONFIG[global_prefix]}\"" >> $config_file
    echo "CONFIG_IPV6_PREFIX_LEN=${CONFIG[global_prefix_length]}" >> $config_file

done

for p in "${PROFILES[@]}"; do
    set -- $p
    this_node=$1
    config=$2
    other_node=$3

    if [ ! -v FW_FLAGS[$config] ];
    then
        FW_FLAGS[$config]=1 # mark firmware as build so we only build once

        if [ $config = "border_router" ];
        then
            BOARD="${CONFIG[board]}" CCACHE_DISABLE=1 make clean  -C ../border_router
            BOARD="${CONFIG[board]}" CCACHE_DISABLE=1 KCONFIG_ADD_CONFIG=$config.config make all -j4 -C ../border_router
            if [ $? -ne 0 ];
            then
                echo "Error building application" >&2
                exit 1
            fi

            cp ../border_router/bin/"${CONFIG[board]}"/*.elf ./$config.elf
        else
            echo "Building profile '$config.elf'"
            BOARD="${CONFIG[board]}" CCACHE_DISABLE=1 QQ=@ make clean
            BOARD="${CONFIG[board]}" CCACHE_DISABLE=1 QQ=@ KCONFIG_ADD_CONFIG=$config.config make all -j4

            if [ $? -ne 0 ];
            then
                echo "Error building application" >&2
                exit 1
            fi

            cp ./bin/"${CONFIG[board]}"/*.elf ./$config.elf
        fi
    fi
done

rm -f ../border_router/*.inc
