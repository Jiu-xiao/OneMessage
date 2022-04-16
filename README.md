# OneMessage
一个基于`发布-订阅`模型的多线程消息框架，用于嵌入式平台，纯C实现，性能和灵活性极高。
---
---
## 硬件支持
已在linux,STM32F103,STM32F407上成功运行，欢迎适配到其他平台。
* 简单示例
    * [example for linux(use cmake)](https://gitee.com/jiu-xiao/msg-example.git)

    * [example for f103(use makefile)](https://gitee.com/jiu-xiao/om-example-mcu.git)
* 使用本框架的开源项目
    * [青岛大学Robomaster机器人嵌入式开源](https://gitee.com/qdu-rm-2022/qdu-rm-mcu)

## 特性
* 发布-订阅
    * 话题为操作的主体，支持多发布者和多订阅者
    * 话题可以作为话题的订阅者和发布者
    * 可实现消息的分流/汇集/桥接
* 消息过滤器
    * 支持为每个话题和订阅者注册过滤器，只接受符合条件的消息
* 订阅部署
    * 可为订阅者注册接收回调函数，自动解析收到的消息。
* 自动发布
    * 可为话题注册更新函数，由OneMessage自动检测并发布。
* 快速配置
    * 使用格式化输入的方式加载配置，使用较少的代码就能实现整个网络的搭建
* 批量筛选
    * 对于条件简单且分支较多的话题，可开启高级过滤器而不需要注册大量函数，现已支持`列表`、`范围`、`分解`模式。
* 日志框架
    * 使用示例程序打印的日志：![效果](img/log.png)
* 统计信息打印
    * 显示所有话题、订阅者和发布者的模式、缓冲区、注册函数等信息
    * 使用FreeRTOS命令行打印：![效果](img/cli.png)
* 活动追踪
    * 使用`utils/prase_report.py`将串口上传的数据转换成CSV文件：

        | time    | name      | activity  |
        | ------- | --------- | --------- |
        | 9.01536 | can_2_rx  | publish   |
        | 9.01536 | can_tof   | filter    |
        | 9.01539 | can_tof   | publish   |
        | 9.0154  | tof_fb    | link      |
        | 9.01541 | tof_fb    | publish   |
        | 9.01571 | tof_fb    | subscribe |
        | 9.01571 | motor_cmd | publish   |
        | 9.02386 | motor_cmd | export    |
        | 9.02386 | motor_out | publish   |
        | 9.02387 | can_2_out | link      |
        | 9.02388 | can_2_out | publish   |


-------
## 获取源码
克隆这个仓库
```
git clone https://gitee.com/jiu-xiao/one-message.git
```
或者使用`git submodule`将其包含在你的仓库中
```
git submodule add https://gitee.com/jiu-xiao/one-message.git <path>
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
****
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
****
## 文档
> [配置文件](doc/config.md)

> [使用说明](doc/user.md)

> [开发文档](doc/dev.md)
## 这个项目的意义
在复杂的工程中，为了实现线程的通信和数据的同步会耗费掉大量的开发时间，但往往bug都会出现在这些地方，所以需要找到一个稳定可靠的方案。OneMessage即是解决这个问题的一次尝试。
