# 使用方法

## 创建om_config.h配置文件

参照`config/om_config_template.h`,编写`om_config.h`。

## 引用

`include "om.h"`

## 添加同步函数

将`om_sync(bool in_isr)`在中断或者线程按照OM_CALL_FREQ频率调用。

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

buff_size取决于话题的数量，请提供足够大的缓冲空间来保证正常打印

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

## 事件触发器

    om_event_group_t om_event_create_group(char* name);

创建一个事件组。内部使用uint32_t来存放事件，一位对应一个事件，建议使用枚举加左移(1 << 0)的形式。

示例：`om_event_group_t evt_group = om_event_create_group("test_group")`

    om_status_t om_event_register(om_event_group_t group, uint32_t event, om_user_fun_t fun, void* arg);

为某个事件注册回调函数，fun在事件发生时会被调用，并将msg和arg传入，msg包含此次触发的所有事件；

示例：`om_event_register(evt_group, EVENT_1, callback_fun, fun_arg)`

    om_status_t om_event_active(om_event_group_t group, uint32_t event, bool block， bool in_isr);

触发事件。示例：`om_event_active(evt_group, EVENT_1, true, false)`

注册和触发时可同时选择多个事件，如（EVENT_1 | EVENT_2），同时每个事件可注册多个回调函数。

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

## 活动上报

    typedef enum {
    OM_ACTIVITY_PUBLISH = 0,
    OM_ACTIVITY_SUBSCRIBE,
    OM_ACTIVITY_FILTER,
    OM_ACTIVITY_LINK,
    OM_ACTIVITY_EXPORT
    } om_activity_t;

    typedef struct {
    uint32_t id_activity;波
    om_status_t om_send_report_data()

| TOPIC_MAP | 前缀  | 话题1 ID | 话题1名称 | ... | ... | 话题N ID | 话题N名称 | 后缀  |
| --------- | ----- | -------- | --------- | --- | --- | -------- | --------- | ----- |
| 功能      | 识别  | ID       | 名称      | ... | ... | ID       | 名称      | 识别  |
| 长度      | 4字节 | 2字节    | 不定      | ... | ... | 2字节    | 不定      | 4字节 |
| 内容      | @msg  | 整型数   | 字符串    | ... | ... | 整形数   | 字符串 -  | @end  |

| DATA | 包1 ID | 包1 活动      | 包1 时间 | ... | ... | ... | 包N ID | 包N 活动      | 包N 时间 |
| ---- | ------ | ------------- | -------- | --- | --- | --- | ------ | ------------- | -------- |
| 功能 | ID     | 活动类型      | 时间     | ... | ... | ... | ID     | 活动类型      | 时间     |
| 长度 | 2字节  | 2字节         | 4字节    | ... | ... | ... | 2字节  | 2字节         | 4字节    |
| 内容 | 整型数 | om_activity_t | 整型数   | ... | ... | ... | 整型数 | om_activity_t | 整型数   |

`om_send_report_data`会先对所有队列中的话题生成一个表，前缀为`"@msg"`,后缀为`"@end"`,表元素为话题ID+话题名字符串，然后调用`om_report_transmit`发送。接着会把所有采集到的信息按照`om_report_t`格式打包并按时间排列，继续调用`om_report_transmit`发送。

由于每次都要发送所有话题的名称，所以可以减少调用频率节省带宽，同时增大缓冲区避免数据丢失。

`id_activity` 前16位为话题id,后十六位为活动类型。utils/prase_report.py只提供了`小端`数据的解析示例。
