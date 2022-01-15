# 使用方法
## 创建om_config.h配置文件
参照`config/om_config_template.h`，编写`om_config.h`。
## 引用
`include "om.h"`
## 添加同步函数
将`om_sync()`在中断或者线程按照OM_CALL_FREQ频率调用。
## 初始化
    om_status_t om_init();
## 配置
    om_topic_t* om_config_topic(om_topic_t* topic, const char* format, ...)

    om_suber_t* om_config_suber(om_suber_t* suber, const char* format, ...)

    om_puber_t* om_config_puber(om_puber_t* puber, const char* format, ...)

topic/suber/puber==NULL时，会新创建对应类型的变量并返回，且om_config_topic的第一个可选参数为话题名。

format参数支持大小写。
### 发布者配置
| 选项 | 参数                              | 功能              |
| ---- | --------------------------------- | ----------------- |
| n    | om_status_t (*fun)(om_msg_t *msg) | 注册new函数       |
| g    | om_status_t (*fun)(om_msg_t *msg) | 注册get函数       |
| t    | om_topic_t *                      | 将发布者指向话题  |
| q    | float                             | 设置new的调用频率 |

### 订阅者配置
| 选项 | 参数                              | 功能               |
| ---- | --------------------------------- | ------------------ |
| f    | om_status_t (*fun)(om_msg_t *msg) | 注册filter函数     |
| d    | om_status_t (*fun)(om_msg_t *msg) | 注册deploy函数     |
| t    | om_topic_t *                      | 将订阅者者指向话题 |

### 话题配置
| 选项 | 参数                                                                   | 功能                                                                     |
| ---- | ---------------------------------------------------------------------- | ------------------------------------------------------------------------ |
| f    | om_status_t (*fun)(om_msg_t *msg)                                      | 注册filter函数                                                           |
| l    | om_topic_t *                                                           | 指向参数中的话题                                                         |
| s    | om_suber_t *                                                           | 将订阅者指向话题                                                         |
| p    | om_puber_t *                                                           | 将发布者指向话题                                                         |
| d    | om_status_t (*fun)(om_msg_t *msg)                                      | 先将fun注册为新创建的订阅者的deploy函数，再将订阅者指向话题              |
| n或g | om_status_t (*fun1)(om_msg_t *msg)，om_status_t (*fun2)(om_msg_t *msg) | 先将fun1，fun2分别注册为新创建的发布者的new和get函数，再将发布者指向话题 |
| t    | om_topic_t *                                                           | 将参数中的话题指向自身                                                   |
| v    | 无                                                                     | 设置为虚话题(不拷贝消息内容，只传递指针)                                 |
| a    | 无                                                                     | 将话题添加到队列                                                         |

例：
* om_config_suber(NULL,"fdt",fun1,fun2,your_topic)会返回一个新创建的指向your_topic的订阅者，且其filter函数为fun1,deploy函数为fun2。
* om_config_topic(your_topic,"sp",suber,puber)会将订阅者suber和发布者puber指向your_topic。
* om_config_topic(NULL,"v","topic_name")会新创建一个名为topic_name虚话题并返回
## 返回值
  * OM_OK = 0,
  * OM_ERROR = 1
  * OM_ERROR_NULL
  * OM_ERROR_BUSY
  * OM_ERROR_TIMEOUT
  * OM_ERROR_NOT_INIT

## 用户函数
| 函数名 | 功能                                                                                          |
| ------ | --------------------------------------------------------------------------------------------- |
| filter | 如果注册了filter函数，话题和订阅者收到的消息只有返回值为OM_OK时才会保存                       |
| new    | 发布者必须注册，om_sync会定期调用new函数            ，如果返回值为OM_OK,则会进一步调用get函数 |
| get    | 发布者必须注册，将接收到的消息保存在msg中，如果返回OM_OK,则会将msg发布到绑定的话题            |
| deploy | 如果注册了deploy函数，订阅者收到订阅的时候会将调用此函数，并将收到的消息传入                  |

## 将话题加入队列

    om_status_t om_add_topic(om_topic_t *topic)

加入队列后，会立刻开始收发消息，所以需要在调用前完成话题的所有配置。
## 主动发布
    om_status_t om_publish(om_topic_t* topic, void* buff, size_t size, bool block)
block参数决定当其他线程发布时是否阻塞
## 订阅话题
    om_status_t om_subscript(om_topic_t *topic, void *buff, size_t max_size, om_user_fun_t filter)

filter==NULL时不添加过滤器函数
## log
    om_topic_t* om_get_log_handle()    //返回log所在话题
    om_status_t om_print_log(char* name, om_log_level_t level, const char* format,...)

| Level          | Color   |
| -------------- | ------- |
| OM_LOG_DEFAULT | DEFAULT |
| OM_LOG_WARNING | YELLOW  |
| OM_LOG_ERROR   | RED     |
| OM_LOG_PASS    | GREEN   |
| OM_LOG_NOTICE  | BLUE    |
## 查找话题
    om_topic_t *om_core_find_topic(const char *name)
如果能找到对应名字的话题，返回此话题，否则返回NULL

## 注销
    om_status_t om_deinit();
释放OneMessage申请的所有内存空间
## 其他API
| 函数名             | 功能       |
| ------------------ | ---------- |
| om_topic_add_puber | 添加发布者 |
| om_topic_add_suber | 添加订阅者 |
| om_topic_link      | 链接话题   |