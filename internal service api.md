# 内部接口api
## epd业务服务
* ### 心跳上报
serial-->epd
    

    发送：
    {
        "cmd":"heart",
        "device_id":"<终端id>",
        "data_id":"<数据id>",
        "battery":<电量>,
        “status”:<运行状态>,
        "msg":"<需要上报的消息>"
    }
    应答：
    {
        "status": "<ok|error>"
    }

* ### 注册上报
seiral-->epd
    
    
    发送：
    {
        "cmd":"register",
        "device_id":"<终端id>"
    }
    应答：
    {
        "status":"<ok|error>",
        "key":(鉴权码)
    }

* ### 任务通知
epd-->serial


    发送：
    {
        "cmd":"task",
        "method":"create",
        "task_id":<任务id>,
        "data_id":<数据id>,
        "start_time":"<开始时间>",
        "end_time":"<结束时间>"
    }
    接收：
    {
        "status":"ok"
    }

* ### 任务状态上报
serial-->epd


    发送：
    {
        "cmd":"task",
        "task_id":<任务id>
        "status":<0|1|2|3|4|5>
    }
    接收：
    {
        "status":"<ok|error>"
    }
    备注：
    1. 0-none 1-sleep 2-ready 3-run 4-finish 5-suspend
    
* ### 射频模块配置
epd-->serial


    发送：
    {
        "cmd":"update",
        "radio":<射频模块位置号>
    }
    接收:
    {
        "status":"<ok|error>"
    }

* ### 射频模块重启
epd-->serial


    发送：
    {
        "cmd":"restart",
        "radio":<射频模块位置号>
    }
    接收：
    {
        "status":"<ok|error>"
    }
    

    
    
