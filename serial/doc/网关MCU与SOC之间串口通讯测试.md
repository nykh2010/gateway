# 网关MCU与SOC之间串口通讯测试

## 1. 测试环境

米尔imx6ull 开发板， 

DDR：512M，

EMMC： 4G

UART： ttyUSB0

波特率：115200

MCU：stm32开发板

radio：sx1278



## 2. 测试实例

`./example/entry_test.c`

`config.h` 配置

```c
#define EXAMPLE ENTRY_EXAMPLE
// value of section [serial]
#define CONFIG_DEFAULT_VAL_DEVICE "/dev/ttyUSB0"
#define CONFIG_DEFAULT_VAL_BAUDRATE 115200
#define CONFIG_DEFAULT_VAL_PARITY 'N'
#define CONFIG_DEFAULT_VAL_DATABITS 8
#define CONFIG_DEFAULT_VAL_STOPBITS 1
#define CONFIG_DEFAULT_VAL_READ_LIST_MAX_LEN 30
#define CONFIG_DEFAULT_VAL_WRITE_LIST_MAX_LEN 30
#define RTS_BUSY_TIME_MS 20
#define RTS_ACK_TIME_MS 20
```

## 3. 测试结果

× 实验过程：

由imx6ull（SoC）通过串口发送数据到stm32，stm32接受串口数据，通过SPI发送到sx1278.

测试方式采用压力测试，SoC端采取连续发送，MCU连续接收。

发送数据时间间隔从1s下降到6ms：分别为1s、800ms、80ms、8ms、6ms。

串口发送ACK等待时间超时20ms

串口发送BUSY延迟时间20ms

发送数据每包61字节，串口发送每包数据等待ACK应答，当等待时间超时，发送失败;

若收到BUSY，延时20ms重发;

每包最多重发3次;



× 结果

1. 发送数据时间间隔由1s下降到6ms过程中，当间隔位6ms时，出现3次重发后仍然无法发送成功的情况。连续发送150包后开始出现大量失败。分析：MCU接收数据后存入接受队列，当uart接收数据的频率超过radio发送数据的频率时，导致接收队列占满。

2. 当发送数据时间间隔设置为8ms时，连续发送10000包，未出现BUSY重发和发送失败; 分析：连续发送时间间隔设置为8ms时，串口发送基本稳定。此时记录：

   + 单包数据发送时间：8.12ms （10000包连续发送的平均值，从调用串口发送函数开始，到接收ACK，函数return OK，每包数据发送时间间隔8ms）

   + 一次稳定的有效通讯时间间隔： 8.12ms+8ms=16.12ms

   + 串口每包数据长度，有效数据61字节，帧头尾数据5字节; ACK长度5字节; BUSY长度5字节。


















































