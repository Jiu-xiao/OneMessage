#include <pthread.h>

/* OneMessage主线程调用的频率 */
#define OM_CALL_FREQ (1000)

/* 使用用户自定义的内存分配 */
#define OM_USE_USER_MALLOC (0)

/* 用户内存分配函数 */
#if OM_USE_USER_MALLOC
#define om_malloc user_malloc
#define om_free user_free
#endif

/* OS层互斥锁api */
#define om_mutex_t pthread_mutex_t
#define om_mutex_init(arg) pthread_mutex_init(arg, NULL)
#define om_mutex_lock(arg) pthread_mutex_lock(arg)
#define om_mutex_trylock(arg) pthread_mutex_trylock(arg) == 0 ? OM_OK : OM_ERROR
#define om_mutex_unlock(arg) pthread_mutex_unlock(arg)

/* 将运行时间作为消息发出的时间 */
#define OM_VIRTUAL_TIME (0)

#if !OM_VIRTUAL_TIME
#include <time.h>
#define om_time_t time_t
#define om_time_get(_time) time(NULL)
#endif

/* 开启"om_log"话题作为OneMessage的日志输出 */
#define OM_LOG_OUTPUT (1)

/* 话题名称最大长度 */
#define OM_TOPIC_MAX_NAME_LEN (10)

/* 日志最大长度 */
#define OM_LOG_MAX_LEN (30)