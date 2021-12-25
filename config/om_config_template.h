/* OneMessage主线程调用的频率 */
#define OM_CALL_FREQ (1000)

/* 使用用户自定义的内存分配 */
#define OM_USE_USER_MALLOC (0)

/* 用户内存分配函数 */
#if OM_USE_USER_MALLOC
#define om_malloc user_malloc
#define om_free user_free
#endif

/* 将运行时间作为消息发出的时间 */
#define OM_VIRTUAL_TIME (1)
// TODO:支持真实时间的记录

/* 开启"om_log"话题作为OneMessage的日志输出 */
#define OM_LOG_OUTPUT (1)

/* 话题名称最大长度 */
#define OM_TOPIC_MAX_NAME_LEN (10)

/* 日志最大长度 */
#define OM_LOG_MAX_LEN (30)