# BR: m3-317
# Host: m3-297
# Requester: m3-327
# Host <-> Requester: ['m3-297', 'm3-302', 'm3-312', 'm3-307', 'm3-317', 'm3-327'] (5 hops)
# Host <-> BR: ['m3-297', 'm3-302', 'm3-312', 'm3-307', 'm3-317'] (4 hops)
# Requester <-> BR: ['m3-327', 'm3-317'] (1 hops)

declare -a ROUTES=(
   "297 0 302 up"
   "302 0 312 up"
   "312 0 307 up"
   "307 0 317 up"

   "327 0 317 up"

   "297 327 302 down"
   "302 297 297 down"
   "302 327 312 down"
   "312 297 302 down"
   "312 327 307 down"
   "307 297 312 down"
   "307 327 317 down"
   "317 297 307 down"
   "317 327 327 down"
   "327 297 317 down"
)
declare -a PROFILES=(
   "317 border_router 0"
   "297 host 327"
   "327 requester 297"
   "302 forwarder 0"
   "312 forwarder 0"
   "307 forwarder 0"
)
