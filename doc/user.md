# 使用方法

## 创建om_config.h配置文件

参照`config/om_config_template.h`,编写`om_config.h`。

## 引用

`include "om.h"`

## 添加同步函数

将`om_sync(in_isr)`在中断或者线程按照OM_CALL_FREQ频率调用。

## 初始化

    om_status_t om_init();

## 配置

    om_topic_t* om_config_topic(om_topic_t* topic, const char* format, ...)

    om_suber_t* om_config_suber(om_suber_t* suber, const char* format, ...)

    om_puber_t* om_config_puber(om_puber_t* puber, const char* format, ...)

topic/suber/puber==NULL时,会新创建对应类型的变量并返回,且om_config_topic的第一个可选参数为话题名。

format参数支持大小写。

### 用户函数

调用用户函数时,会将注册时的fun_arg和消息一起传入。

| 函数名       | 功能                                                                                                          |
| ------------ | ------------------------------------------------------------------------------------------------------------- |
| filter       | 如果注册了filter函数,话题只有在其返回值为OM_OK时才会收到此消息                                                |
| refresh      | 发布者必须注册,由om_sync定期调用           ,需要将新消息写入参数msg中,如果返回OM_OK,则会将msg发布到绑定的话题 |
| sub_callback | 如果注册了sub_callback函数,订阅者收到消息的时候会将调用此函数                                                 |

### 发布者配置

| 选项 | 参数                                                              | 功能                      |
| ---- | ----------------------------------------------------------------- | ------------------------- |
| r    | om_status_t (`*fun`)(`om_msg_t *msg, void *arg`), `void* fun_arg` | 注册refresh函数           |
| t    | om_topic_t *                                                      | 将发布者指向话题          |
| q    | float                                                             | 设置refresh函数的调用频率 |

### 订阅者配置

| 选项 | 参数                                                              | 功能                 |
| ---- | ----------------------------------------------------------------- | -------------------- |
| f    | om_status_t (`*fun`)(`om_msg_t *msg, void *arg`), `void* fun_arg` | 注册filter函数       |
| d    | om_status_t (`*fun`)(`om_msg_t *msg, void *arg`), `void* fun_arg` | 注册sub_callback函数 |
| t    | om_topic_t *                                                      | 将订阅者者指向话题   |

### 话题配置

| 选项 | 参数                                                              | 功能                                                             |
| ---- | ----------------------------------------------------------------- | ---------------------------------------------------------------- |
| f    | om_status_t (`*fun`)(`om_msg_t *msg, void *arg`), `void* fun_arg` | 注册filter函数                                                   |
| l    | om_topic_t *                                                      | 指向参数中的话题                                                 |
| s    | om_suber_t *                                                      | 将订阅者指向话题                                                 |
| p    | om_puber_t *                                                      | 将发布者指向话题                                                 |
| d    | om_status_t (`*fun`)(`om_msg_t *msg, void *arg`), `void* fun_arg` | 先将fun注册为新创建的订阅者的sub_callback函数,再将订阅者指向话题 |
| r    | om_status_t (`*fun`)(`om_msg_t *msg`)                             | 先将fun注册为新创建的发布者的refresh函数,再将发布者指向话题      |
| t    | om_topic_t *                                                      | 将参数中的话题指向自身                                           |
| v    | 无                                                                | 设置为虚话题(`不拷贝消息内容,只传递指针`)                        |
| a    | 无                                                                | 将话题添加到队列                                                 |

例：

* om_config_suber(`NULL,"fdt",fun1,fun1_arg,fun2,fun2_arg,your_topic`)会返回一个新创建的指向your_topic的订阅者,且其filter函数为fun1,sub_callback函数为fun2。
* om_config_topic(`your_topic,"sp",suber,puber`)会将订阅者suber和发布者puber指向your_topic。
* om_config_topic(`NULL,"v","topic_name"`)会新创建一个名为topic_name虚话题并返回

## 返回值

* OM_OK = 0,
* OM_ERROR = 1
* OM_ERROR_NULL
* OM_ERROR_BUSY
* OM_ERROR_TIMEOUT
* OM_ERROR_NOT_INIT                                  
  

## 将话题加入队列

    om_status_t om_add_topic(om_topic_t *topic)

只有加入队列中的话题，才会被调用refresh函数或者统计信息。如果不需要这些功能，你甚至无需调用om_sync()

## 主动发布

    om_status_t om_publish(om_topic_t* topic, void* buff, uint32_t size, bool block, bool in_isr)
block参数决定当其他线程发布时是否阻塞

## 订阅话题

    om_suber_t om_subscript(om_topic_t *topic, void *buff, uint32_t max_size)

    om_status_t om_suber_export(om_suber_t* suber, bool in_isr)

* om_subscript会返回一个可导出话题数据的订阅者
* 当订阅者接收到新数据时,调用om_suber_export会将数据写入buff,并返回OM_OK.

## log

    om_topic_t* om_get_log_handle()    //返回log所在话题
    om_status_t om_print_log(char* name, om_log_level_t level, bool block, bool in_isr, const char* format,...)

| Level                | Color   |
| -------------------- | ------- |
| OM_LOG_LEVEL_DEFAULT | DEFAULT |
| OM_LOG_LEVEL_WARNING | YELLOW  |
| OM_LOG_LEVEL_ERROR   | RED     |
| OM_LOG_LEVEL_PASS    | GREEN   |
| OM_LOG_LEVEL_NOTICE  | BLUE    |

并提供以下宏来增加代码可读性

    // in thread
    OMLOG_DEFAULT
    OMLOG_WARNING
    OMLOG_ERROR
    OMLOG_PASS
    OMLOG_NOTICE
    // in isr
    OMLOG_DEFAULT_ISR
    OMLOG_WARNING_ISR
    OMLOG_ERROR_ISR
    OMLOG_PASS_ISR
    OMLOG_NOTICE_ISR
## 打印话题信息到buff

    om_status_t om_print_topic_message(om_topic_t* topic, char* buff, uint32_t buff_size)


## 查找话题

    om_topic_t *om_core_find_topic(const char *name, uint32_t timeout)
如果能找到对应名字的话题,返回此话题,否则返回NULL

## 获取话题等对象的数量

    uint16_t om_msg_get_topic_num()

    uint16_t om_msg_get_suber_num(om_topic_t* topic)

    uint16_t om_msg_get_puber_num(om_topic_t* topic)

## 用户函数遍历所有话题

    om_status_t om_msg_for_each(om_status_t (*fun)(om_topic_t*))

## 注销

    om_status_t om_deinit();
释放OneMessage申请的所有内存空间

## 其他API

| 函数名               | 功能                     |
| -------------------- | ------------------------ |
| om_topic_add_puber   | 添加发布者               |
| om_topic_add_suber   | 添加订阅者               |
| om_topic_link        | 链接话题                 |
| om_msg_del_topic     | 销毁话题                 |
| om_msg_del_suber     | 销毁订阅者               |
| om_msg_del_puber     | 销毁发布者               |
| om_msg_get_last_time | 获得话题最后一次消息时间 |

## 高级过滤器

    om_status_t om_config_filter(om_topic_t* topic, const char* format, ...)

过滤器配置
| 选项 | 参数                                                                                              | 功能               |
| ---- | ------------------------------------------------------------------------------------------------- | ------------------ |
| l    | `om_topic_t* target,uint32_t length,uint32_t offset,uint32_t scope,void* fl_template`             | 设置过滤器列表模式 |
| r    | `om_topic_t* target,uint32_t length,uint32_t offset,uint32_t scope,uint32_t start,uint32_t range` | 设置过滤器范围模式 |
| d    | `om_topic_t* target,uint32_t length,uint32_t offset,uint32_t scope`                               | 设置过滤器分解模式 |

在列表模式下,会从offset开始检测收到的消息是否与template相同,直到offset+scope为止,然后把整包发布到target话题。

在范围模式下,会从offset到offset+scope视为一个整形数,然后判断其>=start且<=start+range,`暂时只支持uint32_t类型`,再将整包发布到target。

在分解模式下,会将offset到offset+scope这一段数据发布到target。

为了避免计算偏移量以及长度,OneMessage提供了OM_PRASE_STRUCT宏,可得到结构体对应成员的length,offset,scope三个参数。例：

数据结构体：

    typedef struct{
        char list[10];
        uint32_t range;
        float decomp;
    }om_example_t;

配置列表模式

    om_config_filter(source,"l",target_topic,OM_PRASE_STRUCT(om_example_t,list),your_template)

配置范围模式

    om_config_filter(source,"r",target_topic,OM_PRASE_STRUCT(om_example_t,range),start,range)

配置分解模式

    om_config_filter(source,"d",target_topic,OM_PRASE_STRUCT(om_example_t,decomp))
