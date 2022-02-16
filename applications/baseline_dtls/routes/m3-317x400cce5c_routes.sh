# BR: m3-317
# Host: m3-299
# Requester: m3-337
# Host <-> Requester: ['m3-299', 'm3-308', 'm3-317', 'm3-311', 'm3-321', 'm3-330', 'm3-337'] (6 hops)
# Host <-> BR: ['m3-299', 'm3-308', 'm3-317'] (2 hops)
# Requester <-> BR: ['m3-337', 'm3-330', 'm3-321', 'm3-311', 'm3-317'] (4 hops)

declare -a ROUTES=(
   "299 0 308 up"
   "308 299 299 down"
   "308 0 317 up"
   "317 299 308 down"

   "337 0 330 up"
   "330 337 337 down"
   "330 0 321 up"
   "321 337 330 down"
   "321 0 311 up"
   "311 337 321 down"
   "311 0 317 up"
   "317 337 311 down"

   "299 337 308 down"
   "308 299 299 down"
   "308 337 317 down"
   "317 299 308 down"
   "317 337 311 down"
   "311 299 317 down"
   "311 337 321 down"
   "321 299 311 down"
   "321 337 330 down"
   "330 299 321 down"
   "330 337 337 down"
   "337 299 330 down"
)
declare -a PROFILES=(
   "317 border_router 0"
   "299 host 337"
   "337 requester 299"
   "308 forwarder 0"
   "311 forwarder 0"
   "321 forwarder 0"
   "330 forwarder 0"
)
