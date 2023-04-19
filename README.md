# OneMessage

一个基于 `发布-订阅`模型的跨平台消息框架，纯C语言编写，性能和灵活性极高

------------------------------------------------------------------------

## 什么是OneMessage

OneMessage核心为订阅发布的消息框架，包含了红黑树，链表，队列，crc校验和终端文本格式控制五个基本组件的完整实现。在此基础上完成了如下六个功能模块：

* MSG 消息/话题控制: 控制消息的订阅与发布，话题和订阅者的创建与管理
* FMT 格式化配置：为用户提供格式化字符串的方式来快速配置话题/订阅/过滤器
* EVT 事件机制：基于topic实现的事件触发机制
* LOG 日志系统：多等级的日志打印
* AFL 高级过滤器：为topic提供灵活的筛选方式
* COM 通信解析：用于跨进程/跨设备共享topic

本项目旨在为跨线程和跨设备通信提供一个通用解决方案，以较小的性能代价换取数倍的开发效率。

## 系统支持

已在Linux,FreeRTOS上成功运行，欢迎适配到其他平台。

* 简单示例
  * [example for linux(use cmake)](https://gitee.com/jiu-xiao/msg-example.git)
  * [example for stm32f103(use makefile)](https://gitee.com/jiu-xiao/om-example-mcu.git)
* 使用本框架的开源项目
  * [XRobot](https://gitee.com/qdu-rm-2022/qdu-rm-mcu)

## 特性

* 发布-订阅

  * 底层由红黑树+链表实现，查找与遍历都有良好的性能表现
  * 话题为操作的主体，匿名发布且不限制订阅者数量
  * 支持零拷贝发布

* 安全性
  * 保证线程安全
  * 允许在中断中使用
  * 提供静态API，可以实现无动态内存分配
  * 单元测试覆盖所有功能

* 消息过滤器

  * 支持为每个话题和订阅者注册过滤器，只接受符合条件的消息

* 订阅回调

  * 订阅者可在接收时调用用户回调函数，暂存数据或者转发到另一个话题。

* 快速配置

  * 使用格式化输入的方式配置话题，极少的代码量就能实现整个消息网络的搭建

* 事件触发

  * 以订阅为核心的事件触发器，在保证执行效率的同时，一个触发条件可以对应多个处理函数。

* 批量筛选

  * 对于条件简单且分支较多的话题，可开启高级过滤器而不需要注册大量函数，现已支持 `列表`、`范围`、`分解`模式。例如针对id筛选CAN总线数据，或者将结构体拆分后发布。

* 话题共享

  * 提供topic数据打包与解析api，可通过任意协议共享多个topic数据，实现跨进程和。通过fifo与crc校验保证数据完成性，在拆包/组包/错位和夹杂无效数据的情况下也能确保正确解析。

* 日志框架

  * 使用示例程序打印的日志：![效果](img/log.png)

## 获取源码

克隆这个仓库

[Gitee](https://gitee.com/jiu-xiao/one-message.git)

[Github](https://github.com/Jiu-xiao/OneMessage.git)

```
git clone https://github.com/Jiu-xiao/OneMessage.gitt
```

或者使用 `git submodule`将其包含在你的仓库中

```
git submodule add https://github.com/Jiu-xiao/OneMessage.git <path>
```

## 代码结构

```
OneMessage
├─config
├─src
│  ├─app
│  ├─comp
│  └─core
└─test
```

---

| 文件夹   | 功能         |
| -------- | ------------ |
| config   | 配置文件模板 |
| doc      | 文档         |
| src/app  | 应用         |
| src/comp | 组件         |
| src/core | 核心api      |
| src      | 用户接口     |
| test     | 单元测试     |
| example  | 例程         |
| utils    | 工具         |

## 文档

> [配置文件](doc/config.md)

> [使用说明](doc/user.md)

> [开发文档](doc/dev.md)
