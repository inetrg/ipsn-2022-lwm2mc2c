# BR: m3-317
# Host: m3-198
# Requester: m3-334
# Host <-> Requester: ['m3-198', 'm3-216', 'm3-292', 'm3-297', 'm3-306', 'm3-312', 'm3-321', 'm3-326', 'm3-334'] (8 hops)
# Host <-> BR: ['m3-198', 'm3-216', 'm3-292', 'm3-297', 'm3-306', 'm3-312', 'm3-317'] (6 hops)
# Requester <-> BR: ['m3-334', 'm3-326', 'm3-321', 'm3-312', 'm3-317'] (4 hops)

declare -a ROUTES=(
   "198 0 216 up"
   "216 198 198 down"
   "216 0 292 up"
   "292 198 216 down"
   "292 0 297 up"
   "297 198 292 down"
   "297 0 306 up"
   "306 198 297 down"
   "306 0 312 up"
   "312 198 306 down"
   "312 0 317 up"
   "317 198 312 down"

   "334 0 326 up"
   "326 334 334 down"
   "326 0 321 up"
   "321 334 326 down"
   "321 0 312 up"
   "312 334 321 down"
   "312 0 317 up"
   "317 334 312 down"

   "198 334 216 down"
   "216 198 198 down"
   "216 334 292 down"
   "292 198 216 down"
   "292 334 297 down"
   "297 198 292 down"
   "297 334 306 down"
   "306 198 297 down"
   "306 334 312 down"
   "312 198 306 down"
   "312 334 321 down"
   "321 198 312 down"
   "321 334 326 down"
   "326 198 321 down"
   "326 334 334 down"
   "334 198 326 down"
)
declare -a PROFILES=(
   "317 border_router 0"
   "198 host 334"
   "334 requester 198"
   "216 forwarder 0"
   "292 forwarder 0"
   "297 forwarder 0"
   "306 forwarder 0"
   "312 forwarder 0"
   "321 forwarder 0"
   "326 forwarder 0"
)
