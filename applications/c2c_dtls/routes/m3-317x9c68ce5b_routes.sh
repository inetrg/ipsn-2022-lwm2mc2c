# BR: m3-317
# Host: m3-332
# Requester: m3-294
# Host <-> Requester: ['m3-332', 'm3-326', 'm3-321', 'm3-317', 'm3-312', 'm3-308', 'm3-302', 'm3-299', 'm3-294'] (8 hops)
# Host <-> BR: ['m3-332', 'm3-326', 'm3-321', 'm3-317'] (3 hops)
# Requester <-> BR: ['m3-294', 'm3-299', 'm3-302', 'm3-308', 'm3-312', 'm3-317'] (5 hops)

declare -a ROUTES=(
   "332 0 326 up"
   "326 332 332 down"
   "326 0 321 up"
   "321 332 326 down"
   "321 0 317 up"
   "317 332 321 down"

   "294 0 299 up"
   "299 294 294 down"
   "299 0 302 up"
   "302 294 299 down"
   "302 0 308 up"
   "308 294 302 down"
   "308 0 312 up"
   "312 294 308 down"
   "312 0 317 up"
   "317 294 312 down"

   "332 294 326 down"
   "326 332 332 down"
   "326 294 321 down"
   "321 332 326 down"
   "321 294 317 down"
   "317 332 321 down"
   "317 294 312 down"
   "312 332 317 down"
   "312 294 308 down"
   "308 332 312 down"
   "308 294 302 down"
   "302 332 308 down"
   "302 294 299 down"
   "299 332 302 down"
   "299 294 294 down"
   "294 332 299 down"
)
declare -a PROFILES=(
   "317 border_router 0"
   "332 host 294"
   "294 requester 332"
   "326 forwarder 0"
   "321 forwarder 0"
   "312 forwarder 0"
   "308 forwarder 0"
   "302 forwarder 0"
   "299 forwarder 0"
)
