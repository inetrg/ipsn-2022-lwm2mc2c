# tree-like topology, with 4 hops to BR
#
#         ┌───┐
#         │317│ Border
#         └─┬─┘ Router
#           │
#         ┌─┴─┐
#         │313│
#         └─┬─┘
#           │
#         ┌─┴─┐
#         │314│
#         └─┬─┘
#           │
#         ┌─┴─┐
#     ┌───┤316├───┐
#     │   └───┘   │
#   ┌─┴─┐       ┌─┴─┐
#   │318│       │319│
#   └───┘       └───┘
#     O           H
#

declare -a PROFILES=(
    "317 border_router 0"
    "313 forwarder 0"
    "314 forwarder 0"
    "316 forwarder 0"
    "318 requester 319"
    "319 host 318"
)

#
# columns:
#   1. Node ID where to install route
#   2. Node ID of the destination
#   3. Node ID of the next hop
#
# NOTE: 0 means the default route
#
declare -a ROUTES=(
    # 0
    "317 318 313 down"
    "317 319 313 down"

    # 1
    "313 318 314 down"
    "313 319 314 down"
    "313 0 317 up"

    # 2
    "314 318 316 down"
    "314 319 316 down"
    "314 0 313 up"

    # 3
    "316 318 318 down"
    "316 319 319 down"
    "316 0 314 up"

    # 4
    "318 0 316 up"

    "319 0 316 up"
)
