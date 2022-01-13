# OneMessage
一个基于`发布-订阅`模型的消息框架，为嵌入式设备设计，使用C语言编写。
---
---
## 特性
* 发布-订阅
    * 话题为操作的主体，支持多发布者和多订阅者
    * 话题可以作为话题的订阅者和发布者
    * 可实现消息的分流/汇集/回环/桥接
* 消息过滤器
    * 支持为每个话题注册过滤器，只接受符合条件的消息
* 编解码器
    * 可在每个话题的出入口注册编解码器，根据协议打包解析消息
* 自动发布
    * 可为话题注册更新检查函数，由OneMessage自动发布
* 话题优先级
* 日志
    * 使用示例程序打印的日志：![效果](img/log.png) 
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
****
## 文档
> [配置文件](doc/config.md)  

> [使用说明](doc/user.md)

> [开发文档](doc/dev.md)
## 这个项目的意义
在复杂的工程中，为了实现线程的通信和数据的同步会耗费掉大量的开发时间，但往往bug都会出现在这些地方，所以需要找到一个稳定可靠的方案。OneMessage即是解决这个问题的一次尝试。
