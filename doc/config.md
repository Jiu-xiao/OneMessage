# 配置文件

------

## OM_DEBUG

>使能这个宏将会开启OM_ASSENT和OM_CHECK，对空指针等异常情况进行检测，定位异常发生位置。

## OM_USE_USER_MALLOC

>使用用户的内存分配函数，使能时需要自定义om_malloc和om_free。
>
>>
>>FreeRTOS示例
>>
>>>#include "FreeRTOS.h"
>>>
>>>#define om_malloc pvPortMalloc
>>>
>>>#define om_free vPortFree

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
>>
>>#define om_mutex_lock_isr(arg) pthread_mutex_lock(arg)
>>
>>#define om_mutex_unlock_isr(arg) pthread_mutex_unlock(arg)
>>
>>#define om_mutex_delete(arg) pthread_mutex_destroy(arg)
>>
>
>FreeRTOS示例
>>#include "semphr.h"
>>
>>#define om_mutex_t SemaphoreHandle_t
>>
>>#define om_mutex_init(arg) *arg = xSemaphoreCreateBinary();
>>xSemaphoreGive(*arg)
>>
>>#define om_mutex_lock(arg) xSemaphoreTake(*arg,portMAX_DELAY)
>>
>>#define om_mutex_trylock(arg) xSemaphoreTake(*arg,0) == pdTRUE ? OM_OK:OM_ERROR
>>
>>#define om_mutex_unlock(arg) xSemaphoreGive(*arg)
>>
>>#define om_mutex_lock_isr(arg) xSemaphoreTakeFromISR(*arg, NULL) == pdTRUE ? OM_OK:OM_ERROR
>>
>>#define om_mutex_unlock_isr(arg) xSemaphoreGiveFromISR(*arg, NULL)
>>
>>#define om_mutex_delete(arg) vSemaphoreDelete(*arg)


## OM_TIME

>发布消息和log时记录时间
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

## OM_LOG_LEVEL

>日志最小打印等级 1:default 2:notice 3:pass 4:warning 5:error
