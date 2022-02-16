# Booking command: iotlab experiment submit -d 240 -l grenoble,m3,308+312+317,,m3_15
# BR: m3-317
# Host: m3-308
# Requester: m3-312
# Host <-> Requester: ['m3-308', 'm3-312'] (1 hops)
# Host <-> BR: ['m3-308', 'm3-312', 'm3-317'] (2 hops)
# Requester <-> BR: ['m3-312', 'm3-317'] (1 hops)

declare -a ROUTES=(
   "308 0 312 up"
   "312 308 308 down"
   "312 0 317 up"
   "317 308 312 down"

   "312 0 317 up"
   "317 312 312 down"

   "308 312 312 down"
   "312 308 308 down"
)
declare -a PROFILES=(
   "317 border_router 0"
   "308 host 312"
   "312 requester 308"
)
