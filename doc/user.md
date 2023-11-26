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
    om_status_t om_topic_link(om_topic_t* source, om_topic_t* target);

静态内存

    om_topic_t* om_create_topic_static(om_topic_t* topic, const char* name,size_t buff_len)
    om_suber_t* om_create_suber_static(om_suber_t* suber, om_topic_t* link)
    om_status_t om_topic_link_static(om_suber_t* suber, om_link_t* link,
                                    om_topic_t* source, om_topic_t* target);

## 发布消息

    om_status_t om_publish(om_topic_t* topic, void* buff, uint32_t size, bool block, bool in_isr)

block参数决定同时有其他线程发布这个话题时是否等待，in_isr取决于是否在中断中调用

## 异步订阅话题

    om_suber_t* om_subscribe(om_topic_t* topic);

    om_suber_t* om_subscribe_static(om_topic_t* topic, om_suber_t* sub);

    om_status_t om_suber_export(om_suber_t* suber, void* buff, bool in_isr);

    bool om_suber_available(om_suber_t* suber);

* om_subscribe会返回一个可导出话题数据的订阅者
* 当订阅者接收到新数据时,调用om_suber_export会将数据写入buff,并返回OM_OK.
* om_suber_available判断是否有新数据

## 为话题添加订阅队列

与普通订阅者不同，使用队列可以存储一个topic上的多个消息，但是无法使用回调函数。

动态分配内存

    om_fifo_t* om_queue_add(om_topic_t* topic, uint32_t len);

example:

    /* 创建topic */
    om_topic_t* topic = om_create_topic("topic", 1u);
    /* 添加队列 */
    om_fifo_t* queue = om_queue_add(topic, 10);
    /* 发布消息 */
    //om_publish(...)

    /* 加锁 */
    OM_TOPIC_LOCK(topic);

    /* 获取消息数量 */
    uint32_t count = om_fifo_readable_item_count(queue);

    /* 弹出数据 */
    for(int i=0;i<count;i++){
        om_fifo_pop(queue);
    }

    /* 解锁 */
    OM_TOPIC_UNLOCK(topic);

静态内存

    om_status_t om_queue_init_fifo_static(om_topic_t* topic, om_fifo_t* fifo,
                                        void* buff, uint32_t len);

    om_fifo_t* om_queue_add_static(om_topic_t* topic, om_suber_t* sub,
                                        om_fifo_t* fifo);

example:

    om_topic_t topic;
    om_fifo_t queue;
    om_suber_t suber;
    uint8_t buff[10];

    /* 创建话题 */
    om_create_topic_static(&topic, "topic", 1u);
    /* 初始化队列 */
    om_queue_init_fifo_static(&topic, &queue, buff, 10);
    /* 添加队列 */
    om_queue_add_static(&topic, &suber, &queue);

    /* 发布消息 */
    //om_publish(...)

    /* 加锁 */
    OM_TOPIC_LOCK(&topic);

    /* 获取消息数量 */
    uint32_t count = om_fifo_readable_item_count(queue);

    /* 弹出数据 */
    for(int i=0;i<count;i++){
        om_fifo_pop(queue);
    }

    /* 解锁 */
    OM_TOPIC_UNLOCK(&topic);

### 队列API

    void om_fifo_create(om_fifo_t* fifo, void* fifo_ptr, uint32_t item_sum,
                        uint32_t item_size);

    bool om_fifo_writeable(om_fifo_t* fifo);

    om_status_t om_fifo_write(om_fifo_t* fifo, const void* data);

    om_status_t om_fifo_writes(om_fifo_t* fifo, const void* data,
                            uint32_t item_num);

    bool om_fifo_readable(om_fifo_t* fifo);

    om_status_t om_fifo_read(om_fifo_t* fifo, void* data);

    om_status_t om_fifo_pop(om_fifo_t* fifo);

    om_status_t om_fifo_pop_batch(om_fifo_t* fifo, uint32_t item_num);

    om_status_t om_fifo_push(om_fifo_t* fifo, const void* data);

    om_status_t om_fifo_jump_peek(om_fifo_t* fifo, uint32_t num, void* data);

    om_status_t om_fifo_peek(om_fifo_t* fifo, void* data);

    om_status_t om_fifo_peek_batch(om_fifo_t* fifo, void* data, uint32_t item_num);

    om_status_t om_fifo_reads(om_fifo_t* fifo, void* data, uint32_t item_num);

    uint32_t om_fifo_readable_item_count(om_fifo_t* fifo);

    uint32_t om_fifo_writeable_item_count(om_fifo_t* fifo);

    om_status_t om_fifo_reset(om_fifo_t* fifo);

    om_status_t om_fifo_overwrite(om_fifo_t* fifo, const void* data);

    void om_fifo_foreach(om_fifo_t* fifo, bool (*fun)(void* data, void* arg),
                        void* arg);

    void* om_fifo_foreach_dist(om_fifo_t* fifo, void* data);

## 格式化配置

    om_topic_t* om_config_topic(om_topic_t* topic, const char* format, ...)

    om_suber_t* om_config_suber(om_suber_t* suber, const char* format, ...)

1. topic/suber==NULL时,会自动创建对应类型的变量(动态分配内存)并返回,且om_config_topic的第一个可选参数为话题名，第二个参数的话题数据的最大大小。

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
| l    | om_topic_t *                                                      | 链接话题，以后收到的消息会转发到参数中的话题(会申请一块内存用来存放新的链接和订阅者)  |
| k    | om_suber_t*,om_link_t*,om_topic_t *                               | 链接话题，以后收到的消息会转发到参数中的话题，不会动态申请内存                          |
| t    | om_suber_t *                                                      | 链接话题，参数中的话题收到的消息会转发到本话题(会申请一块内存用来存放新的链接和订阅者) |
| e    | om_suber_t*,om_link_t*,om_topic_t *                               | 链接话题，参数中的话题收到的消息会转发到本话题，不会动态申请内存                        |
| d    | om_status_t (`*fun`)(`om_msg_t *msg, void *arg`), `void* fun_arg` | 先将fun注册为新创建(使用动态分配)的订阅者的sub_callback函数,再将订阅者指向话题        |
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

      // 定义变量(注意变量生命周期)
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
* OM_ERROR = 1,
* OM_ERROR_NULL,
* OM_ERROR_BUSY,
* OM_ERROR_TIMEOUT,
* OM_ERROR_FULL,
* OM_ERROR_EMPTY,
* OM_ERROR_NOT_INIT

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

## 给话题加锁

    bool OM_TOPIC_LOCK(om_topic_t*);

    void OM_TOPIC_UNLOCK(om_topic_t*);

如果使用om_suber_export方式获取话题数据，而且传递的是数据的原始指针，发布线程可以在写入数据时给话题加锁，防止订阅者读取到不完整的数据。

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

在列表模式下,会在内存地址offset到offset+scope内匹配是否与template相同,然后把整包发布到target话题。

在范围模式下,会把内存offset到offset+scope视为一个无符号整形数,然后判断其值大于等于start且小于start+range,`暂时只支持uint32_t类型`,再将整包发布到target。

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

## 通信收发器

为了满足跨进程和跨设备通信的需要，我们设计了这个通信收发器。它能够将任意一个topic中的数据连同topic的身份信息一同打包到一个数据包当中，同时能够处理其他设备通过任意方式传递来的数据包。

---

创建一个通信节点。每个通信节点包含一个接收fifo(fifo_buff，buffer_size)和解析用的缓冲区(prase_buff, prase_buff_len)。同时也包含一个有map_len长度的om_com_map_item_t数组(map，map_len)，用来存放topic的信息。

buffer_size和prase_buff_len应当大于接收topic的最大数据长度+10，加大buff能够更好的应对组包的情况。

    om_status_t om_com_create(om_com_t* com, uint32_t buffer_size, uint16_t map_len, uint32_t prase_buff_len);

    om_status_t om_com_create_static(om_com_t* com, void* fifo_buff, uint32_t buffer_size, om_com_map_item_t* map, uint16_t map_len, uint8_t* prase_buff, uint32_t prase_buff_len);

为通信节点添加topic。请确保在添加之前需要的topic已经创建完毕。

    om_status_t om_com_add_topic(om_com_t* com, om_topic_t* topic);

    om_status_t om_com_add_topic_with_name(om_com_t* com, const char* topic_name);

利用topic内的信息打包pack。因为加入了校验和topic信息，buff的长度应当比topic消息的长度多10个字节，我们提供了OM_COM_TYPE宏来快速创建缓冲区。

    om_status_t om_com_generate_pack(om_topic_t* topic, void* buff);

将接收到的数据包解析，如果存在完整的数据包，会直接发布到对应topic。支持拆包，组包，错位或者夹杂无效数据情况下的解析。

    om_com_recv_ans_t om_com_prase_recv(om_com_t* com, uint8_t* buff, uint32_t size, bool block, bool in_isr);

示例:

定义数据结构体

    typedef struct {
        uint32_t test;
        char str[10];
    } om_com_test_t;

发送端配置

    /* 初始化OneMessage */
    om_init();

    /* 创建话题 */
    om_topic_t* source = om_config_topic(NULL, "ca", "source", sizeof(om_com_test_t));

    /* 创建发送缓冲区 */
    OM_COM_TYPE(om_com_test_t) trans_buffer;

    /* 创建发送数据 */
    om_com_test_t test_data = {.str = "test data1", .test = 0x12345678};

    /* 发布消息 */
    om_publish(source, &test_data, sizeof(om_com_test_t), true, false);

    /* 打包消息，写入缓冲区 */
    om_com_generate_pack(source, &trans_buffer);

    /* 使用任何一种方式发送数据 */
    //user_trans_data(trans_buffer, sizeof(trans_buffer));

接收端配置

    /* 初始化OneMessage */
    om_init();

    /* 创建话题 */
    om_topic_t* source = om_config_topic(NULL, "ca", "source", sizeof(om_com_test_t));

    /* 接收节点变量 */
    om_com_t com;

    /* 创建接收节点 */
    om_com_create(&com, 128, 5, 128);

    /* 添加接收topic */
    om_com_add_topic(&com, source);

    /* 使用任何一种方式接收数据 */
    //user_recv_data(user_recv_buff, sizeof(user_recv_buff));

    /* 解析接收到的数据 */
    om_com_prase_recv(&com, (uint8_t*)&user_recv_buff, user_recv_len, true, false);

### 存在的问题

1. 包数据段较长（>10kb）时，解析性能可能较低。
2. 为了提高带宽利用率和打包速度，使用crc32作为topic的id，存在重复的可能性，出现此情况时请给topic更名。后续考虑使用分布更均匀的生成算法。
