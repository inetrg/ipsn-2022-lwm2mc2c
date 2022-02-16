# BR: m3-317
# Host: m3-323
# Requester: m3-302
# Host <-> Requester: ['m3-323', 'm3-313', 'm3-308', 'm3-302'] (3 hops)
# Host <-> BR: ['m3-323', 'm3-317'] (1 hops)
# Requester <-> BR: ['m3-302', 'm3-308', 'm3-313', 'm3-323', 'm3-317'] (4 hops)

declare -a ROUTES=(
   "323 0 317 up"
   "317 323 323 down"

   "302 0 308 up"
   "308 302 302 down"
   "308 0 313 up"
   "313 302 308 down"
   "313 0 323 up"
   "323 302 313 down"
   "323 0 317 up"
   "317 302 323 down"

   "323 302 313 down"
   "313 323 323 down"
   "313 302 308 down"
   "308 323 313 down"
   "308 302 302 down"
   "302 323 308 down"
)
declare -a PROFILES=(
   "317 border_router 0"
   "323 host 302"
   "302 requester 323"
   "313 forwarder 0"
   "308 forwarder 0"
)
