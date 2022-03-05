# 配置文件

------

## OM_DEBUG

>使能这个宏将会开启OM_ASSENT和OM_CHECK，对空指针等异常情况进行检测，定位异常发生位置。

## OM_CALL_FREQ

>定义为om_sync()被调用的频率

## OM_USE_USER_MALLOC

>使用用户的内存分配函数，使能时需要自定义om_malloc和om_free。
>>om_malloc 内存分配
>>
>>om_free 内存释放

## OM_DELAY

>非阻塞延时函数
>>om_delay_ms 等待一毫秒

## OM_MUTEX_*

### OS层提供的互斥锁API

>Linux示例
>>#include <pthread.h>
>>
>>#define om_mutex_t pthread_mutex_t
>>
>>#define om_mutex_init(arg) pthread_mutex_init(arg, NULL)
>>
>>#define om_mutex_lock(arg) pthread_mutex_lock(arg)
>>
>>#define om_mutex_trylock(arg) pthread_mutex_trylock(arg) == 0 ? OM_OK:OM_ERROR
>>
>>#define om_mutex_unlock(arg) pthread_mutex_unlock(arg)
>
>FreeRTOS示例
>>#include "semphr.h"
>>
>>#define om_mutex_t SemaphoreHandle_t
>>
>>#define om_mutex_init(arg) *arg = xSemaphoreCreateMutex(); \\ \
>>xSemaphoreGive(*arg)
>>
>>#define om_mutex_lock(arg) xSemaphoreTake(*arg,portMAX_DELAY)
>>
>>#define om_mutex_trylock(arg) xSemaphoreTake(*arg,0) == pdTRUE ? OM_OK:OM_ERROR
>>
>>#define om_mutex_unlock(arg) xSemaphoreGive(*arg)

## OM_VIRTUAL_TIME

>在无RTC的设备上使能，将运行时间作为消息发出时间。未使能时需要自定义om_time_t，om_time_get。
>>om_time_t 存放时间数据的类型
>>
>>om_time_t om_time_get() 返回时间

## OM_LOG_OUTPUT

>开启日志，输出到`om_log`话题。

## OM_LOG_COLORFUL

>按照日志等级以不同颜色输出。

## OM_TOPIC_MAX_NAME_LEN

>话题名称最大长度

## OM_LOG_MAX_LEN

>日志最大长度
