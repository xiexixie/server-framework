## 高性能服务器框架
### 开发环境
* ubuntu 20.04
* gcc9.4
* cmake 3.16.3
### 日志系统 `单例模式`
1) Log  

        Logger(定义日志类别)
        |
        |-------Formatter(日志格式)
        |
        Appender(日志输出地点)
2) 级别

      ```
      DEBUG,
      INFO,
      WARN,
      ERROR,
      FATAL
      ``` 
3) 日志格式
    ```
    %m--消息体
    %p--level
    %r--启动到输出日志的时间
    %c--日志名称
    %t--线程id
    %n--回车换行
    %d--时间戳
    %f--文件名
    %l--行号
    %T--tab
    %F--协程id
    ```     
### 协程库封装

### socket函数库
### http协议开发