# 内部接口api
## epd业务服务下行接口协议
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
    
## epd业务服务上行接口协议
* ### 任务创建
iot-->epd

    url:localhost:5000/task/create
    
    
    {
        "id":"<命令id>",
        "from":"<命令来源>",
        "command":"task",
        "d":{
            "task_id":<任务id>,
            "image_data_id":<数据id>,
            "image_data_url":"数据地址",
            "image_data_md5":<数据md5>,
            "iot_dev_list_url":"更新终端列表地址",
            "iot_dev_list_md5":<更新终端列表地址md5>,
            "start_time":"<起始时间>",
            "end_time":"<终止时间>"
        }
    }
    备注：
    1. 时间格式 2019-09-10 10:23:30

* ### 任务创建应答
epd-->iot

    url:localhost:7788/mqtt/publish
    
    
    {
        "topic":"dma/cmd/resp",
        "payload":{
            "id":"<命令id>",
            "from":"<命令来源>",
            "status":"<ok|failed>"
            "command":"task",
            "d":{
                "code":"<ok|failed>",
                "msg":"[错误消息]"
            }
        }
    }

* ### 白名单下发
iot-->epd

    url:localhost:5000/gateway/white_list
    
    
    {
        "id":"<命令id>",
        "from":"<命令来源>",
        "command":"white_list",
        "d":{
            "url":"<白名单地址>",
            "md5":<白名单md5>
        }
    }
    
* ### 白名单下发应答
epd-->iot

    url:localhost:7788/mqtt/publish
    
    
    {
        "id":"<命令id>",
        "from":"<命令来源>",
        "command":"white_list",
        "status":"<ok|err>",
        "d":{
            "result":"<ok|err>",
            "msg":"[错误信息]"
        }
    }

* ### key下发
iot-->epd

    url:localhost:5000/gateway/check_code
    
    
    {
        "id":"<命令id>",
        "from":"<命令来源>",
        "command":"check_code",
        "d":{
            "check_code":<key>,
        }
    }

* ### key下发应答
epd-->iot

    url:localhost:7788/mqtt/publish
    
    
    {
        "id":"<命令id>",
        "from":"<命令来源>",
        "command":"check_code",
        "status":"<ok|err>",
        "d":{
            "result":"<ok|err>",
            "msg":"[错误信息]"
        }
    }

* ### 射频模块配置
web-->epd

    url:localhost:5000/radio/update
    
    
    {
        "from":"local",
        "body":{
            "radio_number":"<射频模块序号>"
        }
    }

* ### 射频模块重启
web-->epd

    url:localhost:5000/radio/restart
    
    
    {
        "from":"local",
        "body":{
            "radio_number":"<射频模块序号>"
        }
    }

## 主动上报
* ### 定时状态上报
epd-->iot

    url://localhost:7788/mqtt/publish
    
    
    {
        "topic":"gateway/report/status",
        "payload":{
            "d":{
                "task_id":<本地已存在的任务id>,
                "whitelist_md5":<本地已存在的白名单md5>,
                "check_code":<本地已存在的key>
            }
        }
    }

* ### 心跳上报
epd-->iot

    url://localhost:7788/mqtt/publish
    
    
    {
        "topic":"dma/report/periph",
        "nid":"<终端id>",
        "payload":{
            "d":{
                "data_id":"<终端当前数据版本>",
                "battery":<终端电量>,
                "status":"<终端状态>",
                "msg":"[终端信息]"
            }
        }
    }

* ### 任务执行状态上报
epd-->iot

    url://localhost:7788/mqtt/publish
    
    
    {
        "topic":"gateway/report/task/result",
        "payload":{
            "d":{
                "task_id":<任务id>,
                "task_status":"<ok|failed>",
                "success_list":[成功列表],
                "fail_list":[失败列表]
            }
        }
    }