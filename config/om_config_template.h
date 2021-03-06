/* Debug */
#define OM_DEBUG (1)

/* OneMessage主线程调用的频率 */
#define OM_CALL_FREQ (1000)

/* 使能这个宏可能会提高刷新频率的精度，但会消耗更多性能 */
#define OM_FREQ_USE_FLOAT (1)

/* 严格限制导出数据时的长度 */
#define OM_STRICT_LIMIT (0)

/* 使用用户自定义的内存分配 */
#define OM_USE_USER_MALLOC (0)

/* 用户内存分配函数 */
#if OM_USE_USER_MALLOC
#define om_malloc user_malloc
#define om_free user_free
#endif

/* 非阻塞延时函数 */
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/select.h>
#define om_delay_ms(arg)                    \
  struct timeval tv;                        \
  tv.tv_sec = arg / 1000;                   \
  tv.tv_usec = (arg % 1000) * 1000;         \
  int err;                                  \
  do {                                      \
    err = select(0, NULL, NULL, NULL, &tv); \
  } while (err < 0 && errno == EINTR);

/* OS层互斥锁api */
#include <pthread.h>
#define om_mutex_t pthread_mutex_t
#define om_mutex_init(arg) pthread_mutex_init(arg, NULL)
#define om_mutex_lock(arg) pthread_mutex_lock(arg)
#define om_mutex_trylock(arg) pthread_mutex_trylock(arg) == 0 ? OM_OK : OM_ERROR
#define om_mutex_unlock(arg) pthread_mutex_unlock(arg)

#define om_mutex_lock_isr(arg) pthread_mutex_lock(arg)
#define om_mutex_unlock_isr(arg) pthread_mutex_unlock(arg)

/* 将运行时间作为消息发出的时间 */
#define OM_VIRTUAL_TIME (0)

#if !OM_VIRTUAL_TIME
#include <time.h>
#define om_time_t time_t
#define om_time_get(_time) time(_time)
#endif

/* 开启"om_log"话题作为OneMessage的日志输出 */
#define OM_LOG_OUTPUT (1)

#if OM_LOG_OUTPUT
/* 按照日志等级以不同颜色输出 */
#define OM_LOG_COLORFUL (1)
/* 日志最大长度 */
#define OM_LOG_MAX_LEN (60)
#endif

/* 话题名称最大长度 */
#define OM_TOPIC_MAX_NAME_LEN (10)

#define OM_REPORT_ACTIVITY (1)

#if OM_REPORT_ACTIVITY

#define OM_REPORT_DATA_BUFF_NUM (128)
#define OM_REPORT_MAP_BUFF_SIZE (1024)
// #define om_get_realtime user_get_realtime_fun
// #define om_report_transmit user_report_transmit_fun

#endif
