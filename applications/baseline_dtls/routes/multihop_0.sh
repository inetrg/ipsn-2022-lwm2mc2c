# tree-like topology, with 1 hops to BR
#
#         ┌───┐
#         │317│ Border
#         └─┬─┘ Router
#           │
#           │
#     ┌────────────┐
#     │            │
#   ┌─┴─┐       ┌─┴─┐
#   │318│       │319│
#   └───┘       └───┘
#     O           H
#

declare -a PROFILES=(
    "317 border_router 0"
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
    "317 318 318 down"
    "317 319 319 down"

    # 1
    "318 0 317 up"

    "319 0 317 up"
)
