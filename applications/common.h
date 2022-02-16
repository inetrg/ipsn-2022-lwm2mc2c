
#define EVENT_SEPARATOR                 ";"
#define EVENT_BEGIN_MARK                "b"
#define EVENT_END_MARK                  "e"
#define EVENT_ERR_MARK                  "er"

#define EVENT_BEGIN(type,id)            type EVENT_SEPARATOR id EVENT_SEPARATOR EVENT_BEGIN_MARK
#define EVENT_END(type,id)              type EVENT_SEPARATOR id EVENT_SEPARATOR EVENT_END_MARK
#define EVENT_ERR(type,id)              type EVENT_SEPARATOR id EVENT_SEPARATOR EVENT_ERR_MARK
