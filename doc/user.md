# 使用方法
## 创建om_config.h配置文件
参照`config/om_config_template.h`，编写`om_config.h`。
## 引用
`include "om.h"`
## 添加同步函数
将`om_sync()`在中断或者线程按照OM_CALL_FREQ频率调用。
## 初始化
调用`om_init()`。
## 创建话题
使用空配置创建  
`om_topic_t* test = om_create_topic("test", &OM_EMPTY_CONFIG);`  
使用自定义配置创建  
`om_topic_t* test = om_create_topic("test", &your_config);`
## 创建发布者
`om_puber_t* pub = om_create_puber(pub_config);`
## 创建订阅者
使用空配置创建  
`om_suber_t* sub = om_create_suber(&OM_EMPTY_CONFIG);`  
使用自定义配置创建  
`om_suber_t* sub = om_create_suber(&your_config);`
## 创建话题配置
| 操作               | 参数           | 功能                       | 状态 |
| ------------------ | -------------- | -------------------------- | ---- |
| OM_USER_FUN_FILTER | 函数入口       | 设置话题过滤函数           | 可选 |
| OM_USER_FUN_GET    | 函数入口       | 发布者获取数据函数         | 必须 |
| OM_USER_FUN_DECODE | 函数入口       | 话题数据解码函数           | 可选 |
| OM_USER_FUN_NEW    | 函数入口       | 发布者检测新数据函数       | 必须 |
| OM_USER_FUN_APPLY  | 函数入口       | 订阅者应用数据函数         | 可选 |
| OM_LINK            | 链接的目标话题 | 将话题作为目标话题的发布者 | 可选 |
| OM_ADD_SUBER       | 订阅者         | 添加订阅者                 | 可选 |
| OM_ADD_PUBER       | 发布者         | 添加发布者                 | 可选 |
| OM_CONFIG_END      | NULL           | config结束标志             | 必须 |
----
发布者配置
设置NEW和GET函数

    om_config_t pub_config[] = {{OM_USER_FUN_NEW, new_fun},
                                {OM_USER_FUN_GET, get_fun},
                                {OM_CONFIG_END, NULL}};

订阅者配置
设置APPLY函数

    om_config_t sub_config[] = {{OM_USER_FUN_APPLY, apply_fun},
                                {OM_CONFIG_END, NULL}};

创建订阅者

    om_suber_t* sub = om_create_suber(sub_config);

创建发布者
    
    om_puber_t* pub = om_create_puber(pub_config);

话题一配置
设置FILTER和DECODE函数
添加发布者和订阅者

    om_config_t topic_config1[] = {{OM_USER_FUN_FILTER, filter_fun},
                                   {OM_USER_FUN_DECODE, decode_fun},
                                   {OM_ADD_PUBER, &pub},
                                   {OM_ADD_SUBER, &sub},
                                   {OM_CONFIG_END, NULL}};
创建话题一

    om_topic_t* topic = om_create_topic("topic", topic_config1);

话题二配置
链接到话题一

    om_config_t topic_config2[] = {{OM_LINK, &topic},
                                   {OM_CONFIG_END, NULL}};

创建话题二

    om_topic_t* topic2 = om_create_topic("topic2", topic_config2);

将两个话题加入队列

    om_add_topic(topic);
    om_add_topic(topic2);

## 主动加载配置

    om_status_t om_config_topic(om_topic_t* topic, om_config_t* config);

    om_status_t om_config_suber(om_suber_t* sub, om_config_t* config);

    om_status_t om_config_puber(om_puber_t* pub, om_config_t* config);

## 主动发布
    om_status_t om_publish_with_name(const char* name, void* buff, size_t size);

    om_status_t om_publish_with_handle(om_topic_t* topic, void* buff, size_t size);
## log
    om_get_log_handle();
    om_status_t om_print_log(om_log_t* log, const char* format, ...);
## 其他API
| 函数名             | 功能               |
| ------------------ | ------------------ |
| om_topic_add_puber | 添加发布者         |
| om_topic_add_suber | 添加订阅者         |
| om_add_topic       | 将话题加入队列     |
| om_topic_link      | 链接话题           |
| om_find_topic      | 根据话题名寻找话题 |
| om_deinit          | 注销OneMessage     |