# 使用方法

## 创建om_config.h配置文件

参照`config/om_config_template.h`和[配置文件介绍](config.md),编写`om_config.h`。

## 添加源文件和头文件到工程

### CMake

直接将src文件夹作为子目录加入

    add_subdirectory(user_dir/one-message/src om.out)

将om_config.h配置文件所在目录加入到OneMessage目标

    target_include_directories(OneMessage PUBLIC config_file_dir)

### Makefile

将src下的所有目录和源文件，以及om_config.h配置文件所在目录加入到编译目标当中。

## 引用头文件

`include "om.h"`

## 初始化

    om_status_t om_init();

## 创建话题/订阅者/链接

动态分配内存

    om_topic_t* om_create_topic(const char* name,size_t buff_len)
    om_suber_t* om_create_suber(om_topic_t* link)
    om_link_t* om_link_create(om_suber_t* sub, om_topic_t* topic)

静态内存

    om_topic_t* om_create_topic_static(om_topic_t* topic, const char* name,size_t buff_len)
    om_suber_t* om_create_suber_static(om_suber_t* suber, om_topic_t* link)
    om_link_t* om_link_create(om_link_t* link, om_suber_t* sub, om_topic_t* topic)

## 配置

    om_topic_t* om_config_topic(om_topic_t* topic, const char* format, ...)

    om_suber_t* om_config_suber(om_suber_t* suber, const char* format, ...)

1. topic/suber==NULL时,会自动创建对应类型的变量（动态分配内存）并返回,且om_config_topic的第一个可选参数为话题名，第二个参数的话题数据的最大大小。

1. 如果允许动态分配的情况下，建议使用om_config_topic/om_config_suber直接创建话题/订阅者/链接。

1. 使用静态内存时，om_config_topic需要调用om_create_topic初始化话题，om_config_suber不需要。

1. om_config_topic支持对同一个话题配置多次，om_config_suber只允许配置一次

1. format参数支持大小写。

### 用户函数

调用用户函数时,会将注册时的fun_arg和消息一起传入。

| 函数名       | 功能                                                           |
| ------------ | -------------------------------------------------------------- |
| filter       | 如果注册了filter函数,话题只有在其返回值为OM_OK时才会收到此消息 |
| sub_callback | 如果注册了sub_callback函数,订阅者收到消息的时候会将调用此函数  |

### 订阅者配置

| 选项 | 参数                                                              | 功能                                       |
| ---- | ----------------------------------------------------------------- | ------------------------------------------ |
| d    | om_status_t (`*fun`)(`om_msg_t *msg, void *arg`), `void* fun_arg` | 注册sub_callback函数                       |
| t    | om_topic_t *                                                      | 将订阅者者指向话题(会申请一块内存作为链接) |

### 话题配置

| 选项 | 参数                                                              | 功能                                                                                    |
| ---- | ----------------------------------------------------------------- | --------------------------------------------------------------------------------------- |
| f    | om_status_t (`*fun`)(`om_msg_t *msg, void *arg`), `void* fun_arg` | 注册filter函数                                                                          |
| l    | om_topic_t *                                                      | 链接话题，以后收到的消息会转发到参数中的话题（会申请一块内存用来存放新的链接和订阅者）  |
| k    | om_suber_t*,om_link_t*,om_topic_t *                               | 链接话题，以后收到的消息会转发到参数中的话题，不会动态申请内存                          |
| t    | om_suber_t *                                                      | 链接话题，参数中的话题收到的消息会转发到本话题（会申请一块内存用来存放新的链接和订阅者) |
| e    | om_suber_t*,om_link_t*,om_topic_t *                               | 链接话题，参数中的话题收到的消息会转发到本话题，不会动态申请内存                        |
| d    | om_status_t (`*fun`)(`om_msg_t *msg, void *arg`), `void* fun_arg` | 先将fun注册为新创建（使用动态分配）的订阅者的sub_callback函数,再将订阅者指向话题        |
| s    | om_suber_t *                                                      | 将参数中的订阅者者添加到话题                                                            |
| c    | 无                                                                | 申请一块内存作为话题的缓存区，以后接收消息时只拷贝数据，不传递指针                      |
| x    | void *                                                            | 将参数指向的内存空间作为话题的缓存区，以后接收消息时只拷贝数据，不传递指针              |
| a    | 无                                                                | 将话题添加到网络，使其可被查找                                                          |

例：

1. 创建一个名为topic_name话题,为其添加一块大小为topic_buff_len的缓存空间，并将其添加到索引网络中。
2. 创建一个指向your_topic,且其sub_callback函数为fun1的订阅者
3. 创建一个名为topic_name1话题,链接到topic_name话题,并将其添加到索引网络中。

* 使用动态内存分配

      // 创建话题
      om_topic_t* your_topic = om_config_topic(NULL,"ca","topic_name",topic_buff_len);
      // 创建订阅者并添加到话题
      om_suber_t* you_suber = om_config_suber(NULL,"dt",fun1,fun1_arg,your_topic);
      // 创建另一个话题并链接
      om_topic_t* your_topic1 = om_config_topic(NULL,"la","topic_name1",topic_buff_len, your_topic);
    或者

      // 创建订阅者
      om_suber_t* you_suber = om_config_suber(NULL,"d",fun1,fun1_arg);
      // 创建话题并添加订阅者
      om_topic_t* your_topic = om_config_topic(NULL,"csa","topic_name",topic_buff_len,you_suber);
      // 创建另一个话题并链接
      om_topic_t* your_topic1 = om_config_topic(NULL,"la","topic_name1",topic_buff_len, your_topic);
    或者

      // 创建话题，创建一个订阅者并添加到话题
      om_topic_t* your_topic = om_config_topic(NULL,"cda","topic_name",topic_buff_len,fun1,fun1_arg);
      // 创建另一个话题并链接
      om_topic_t* your_topic1 = om_config_topic(NULL,"la","topic_name1",topic_buff_len, your_topic);

* 静态内存

      // 定义变量（注意变量生命周期）
      om_topic_t your_topic,your_topic1;
      om_suber_t your_suber,your_suber1;
      om_link_t your_link;

      uint8_t buff[topic_buff_len];

      // 初始化话题
      om_create_topic_static(&your_topic,"topic_name",topic_buff_len);
      om_create_topic_static(&your_topic1,"topic_name1",topic_buff_len);

      // 为订阅者注册函数
      om_config_suber(&your_suber,"d",fun1,fun1_arg);
      // 向话题添加缓冲区和订阅者
      om_config_topic(&your_topic,"xsa",buff,&your_suber)
      // 链接话题
      om_config_topic(&your_topic1,"ea",&your_suber1,&your_link,&your_topic)

## 返回值

* OM_OK = 0,
* OM_ERROR = 1
* OM_ERROR_NULL
* OM_ERROR_BUSY
* OM_ERROR_TIMEOUT
* OM_ERROR_NOT_INIT

## 发布消息

    om_status_t om_publish(om_topic_t* topic, void* buff, uint32_t size, bool block, bool in_isr)
block参数决定同时有其他线程发布这个话题时是否等待，in_isr取决于是否在中断中调用

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

## 查找话题

    om_topic_t *om_core_find_topic(const char *name, uint32_t timeout)
如果能此网络找到对应名字的话题,返回此话题,否则返回NULL

## 获取话题等对象的数量

    uint16_t om_msg_get_topic_num()

    uint16_t om_msg_get_suber_num(om_topic_t* topic)

    uint16_t om_msg_get_puber_num(om_topic_t* topic)

## 用户函数遍历网络

    om_status_t om_msg_for_each_topic(om_status_t (*fun)(om_topic_t*, void* arg),
                                  void* arg)

## 注销和删除

设计时不考虑框架的注销，用户可以将某个动态申请的话题/订阅者/链接删除，但是无法删除任何静态的对象。

## 其他API

| 函数名               | 功能                     |
| -------------------- | ------------------------ |
| om_topic_add_suber   | 添加订阅者               |
| om_topic_link        | 链接话题                 |
| om_suber_available   | 判断订阅者是否有新消息   |
| om_msg_del_topic     | 销毁话题                 |
| om_msg_del_suber     | 销毁订阅者               |
| om_msg_get_last_time | 获得话题最后一次消息时间 |

## 事件触发器

    om_event_group_t* om_event_create_group(char* name);

    om_event_group_t* om_event_create_group_static(om_event_group_t* group,
                                               const char* name);

创建一个事件组。内部使用uint32_t来存放事件，一个值对应一个事件

示例：`om_event_group_t * evt_group = om_event_create_group("test_group")`

    om_status_t om_event_register(om_event_group_t * group, uint32_t event, om_event_status_t status, om_user_fun_t fun, void* arg);

    om_status_t om_event_register_static(om_event_t* handle, om_event_group_t* group, uint32_t event, om_event_status_t status, void (*callback)(uint32_t event, void* arg), void* arg);

为某个事件注册回调函数，fun在事件发生时会被调用，并将event和arg传入

status决定了调用回调函数的条件

    typedef enum {
    OM_EVENT_START,      //上一次触发条件不成立且本次成立
    OM_EVENT_PROGRESS,   //只检查本次触发条件成立
    OM_EVENT_END,        //上一次触发条件成立且本次不成立
    } om_event_status_t;

示例：`om_event_register(evt_group, EVENT_1, OM_EVENT_START, callback_fun, fun_arg)`

    om_status_t om_event_active(om_event_group_t * group, uint32_t event, bool block， bool in_isr);

触发事件。示例：`om_event_active(evt_group, EVENT_1, true, false)`

## 高级过滤器

    om_status_t om_config_filter(om_topic_t* topic, const char* format, ...)

过滤器配置
| 选项 | 参数                                                                                              | 功能               |
| ---- | ------------------------------------------------------------------------------------------------- | ------------------ |
| l    | `om_topic_t* target,uint32_t length,uint32_t offset,uint32_t scope,void* fl_template`             | 设置过滤器列表模式 |
| r    | `om_topic_t* target,uint32_t length,uint32_t offset,uint32_t scope,uint32_t start,uint32_t range` | 设置过滤器范围模式 |
| d    | `om_topic_t* target,uint32_t length,uint32_t offset,uint32_t scope`                               | 设置过滤器分解模式 |

    om_status_t om_config_filter_static(om_topic_t* topic, const char* format, ...)

过滤器配置
| 选项 | 参数                                                                                                                      | 功能               |
| ---- | ------------------------------------------------------------------------------------------------------------------------- | ------------------ |
| l    | `om_afl_filter_t* filter,om_topic_t* target,uint32_t length,uint32_t offset,uint32_t scope,void* fl_template`             | 设置过滤器列表模式 |
| r    | `om_afl_filter_t* filter,om_topic_t* target,uint32_t length,uint32_t offset,uint32_t scope,uint32_t start,uint32_t range` | 设置过滤器范围模式 |
| d    | `om_afl_filter_t* filter,om_topic_t* target,uint32_t length,uint32_t offset,uint32_t scope`                               | 设置过滤器分解模式 |

在列表模式下,会从offset开始检测收到的消息是否与template相同,直到offset+scope为止,然后把整包发布到target话题。

在范围模式下,会从offset到offset+scope视为一个整形数,然后判断其大于等于start且小于start+range,`暂时只支持uint32_t类型`,再将整包发布到target。

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
