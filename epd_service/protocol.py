from threading import Thread
import json
from time import time, sleep
from configparser import ConfigParser
from epd_log import epdlog as LOG
import sys
import os
# import task
import uplink
from downlink import dl
import time
from gateway import gw
import hashlib
from epd_exception import EpdException

sys.path.append(os.path.dirname(__file__))

# 下行协议
class Handle:
    def func(self):
        pass
    
    def upload(self, topic, data):
        up = uplink.Upload()
        up.send(topic=topic, payload=data)

class HeartRequest(Handle):
    def func(self, request):
        time_str = time.strftime("%Y-%m-%d %H:%M:%S",time.localtime(time.time()))
        LOG.info('%s heart beat', request['device_id'])
        # data = {
        #     "table_name":"white_list",
        #     "method":"connect",
        #     "device_id":request['device_id'],
        #     "time":time_str
        # }
        # sql_cmd = "update `white_list` set `last_connect_time`='%s' where `device_id`='%s';" % (time_str, request['device_id'])
        # LOG.info("query database: %s", sql_cmd)
        # data = {
        #     "table_name":"sql",
        #     "sql_cmd": sql_cmd
        # }
        # dl.send_service('database', data)
        self.upload(request)
    
    def upload(self, data):
        # 上传信息
        send_data = {
            "nid": data["device_id"],
            "d":{
                "data_id": data["data_id"],
                "battery": data.get('battery', 100),
                "status": data.get('status', ""),
                "msg": data.get('msg', "")
            }
        }
        # from uplink import upload
        super().upload('dma/report/periph', send_data)
        

class RegisterRequest(Handle):
    def func(self, data):
        pass

class TaskRequest(Handle):
    def func(self, data):
        task_id = data['task_id']
        status = data['status']
        success_list = data.get('success_list', [])
        fail_list = data.get('fail_list', [])
        # sql_cmd = "update `task` set `status`='%d' where `task_id`='%s';" % (status, task_id)
        # request = {
        #     "table_name":"sql",
        #     "sql_cmd": sql_cmd
        # }
        # LOG.info("query database:%s", sql_cmd)
        # ret = dl.send_service('database', request)
        
        # if success_list:
        #     chunk = ""
        #     for success in success_list:
        #         chunk += "`device_id`=%s or" % success
        #     chunk = chunk[:-2]
        #     # LOG.info(chunk)
        #     request = {
        #         "table_name":"sql",
        #         "sql_cmd":"update `execute` set `status`=2 where %s;" % chunk
        #     }
        #     dl.send_service('database', request)
        # if fail_list:
        #     chunk = ""
        #     for fail in fail_list:
        #         chunk += "`device_id`=%s or" % fail
        #     chunk = chunk[:-2]
        #     LOG.info(chunk)
        #     request = {
        #         "table_name":"sql",
        #         "sql_cmd":"update `execute` set `status`=3 where %s;" % chunk
        #     }
        #     dl.send_service('database', request)
        # 上传执行状态
        send_data = {
            "task_id":task_id,
            "task_status":status,
            "success_list":success_list,
            "fail_list":fail_list
        }
        self.upload(send_data)
    
    def upload(self, data):
        send_data = {
            "d":{
                "task_id": data['task_id'],
                "status": data['task_status'],
                "success_list": data['success_list'],
                "failed_list": data['fail_list']
            }
        }
        super().upload('gateway/report/task/result', send_data)

apps = [
    ("heart", HeartRequest),
    ("register", RegisterRequest),
    ("task", TaskRequest)
]

from tornado.web import RequestHandler
# from tornado.httpserver import HTTPServer
from tornado.web import HTTPError
# 上行接口
class TaskApp(RequestHandler):
    def post(self, cmd):
        try:
            request = self.request.body.decode('utf-8')
            request = json.loads(request)
            body = request['d']
            if cmd == 'create':
                ret = gw.create_task(body['task_id'], body['image_data_id'], body['image_data_url'], \
                    body['image_data_md5'], body['iot_dev_list_md5'], body['iot_dev_list_url'], \
                    body['start_time'], body['end_time'])
                if ret:
                    data = {
                        "cmd":"task",
                        "method":"create",
                        "task_id":body['task_id'],
                        "status":1
                    }
                    dl.send_service('serial', data)
                    # raise HTTPError(200)
                    status = 'ok'
                    msg = "task_id %s create success" % body['task_id']
                else:
                    # 不可创建任务
                    LOG.info("task could not create")
                    status = 'failed'
                    msg = "task_id %s create not allowed" % body['task_id']
                    # 上传命令处理结果
                upload = uplink.Upload()
                resp = {
                    "id": request['id'],
                    "from": request['from'],
                    "status": status,
                    "command": request['command'],
                    "d":{
                        "code":"task_status",
                        "msg":msg
                    }
                }
                # gw.get_task_status()
                upload.send(resp, topic='dma/cmd/resp')
            elif cmd == 'cancel':
                # cancel task
                data = {
                    "cmd":"task",
                    "method":"cancel",
                    "task_id":body['task_id']
                }
                dl.send_service('serial', data)
                
                ret = gw.cancel_task(body['task_id'])
                if ret:
                    status = 'ok'
                    result = {'result':'ok'}
                else:
                    status = 'err'
                    result = {'code':'cancel_failed', 'msg':'task not found'}
                

                # data = {
                #     "table_name":"sql",
                #     "sql_cmd":"delete from task where task_id='%s';" \
                #         % body['task_id']
                # }
                # dl.send_service('database', data)
                # 上传处理结果
                upload = uplink.Upload()
                resp = {
                    "id": request['id'],
                    "from": request['from'],
                    "status": status,
                    "command": request['command'],
                    "d":result
                }
                upload.send(resp, topic='dma/cmd/resp')
            elif cmd == 'confirm':
                # confirm task 
                data = {
                    "table_name":"sql",
                    "sql_cmd":"update `task` set (`start_time`='%s', `end_time`='%s', `status`=2) where `task_id`='%s';" % \
                        (body['start_time'], body['end_time'], body['task_id'])
                }
                dl.send_service('database', data)
            else:
                raise HTTPError(404)
        except Exception as e:
            self.write(e.__str__())
            LOG.error("%s" % e.__str__())

class RadioApp(RequestHandler):
    def post(self, cmd):
        body = self.request.body.decode('utf-8')
        body = json.loads(body)
        src = "local" if body['from'] == 'local' else 'remote'
        radio_number = body.get('radio_number')
        if cmd == 'update':
            LOG.info('%s setup radio parameters', src)
            data = {
                "cmd":"update",
                "radio":radio_number
            }
        elif cmd == 'restart':
            LOG.info('%s restart radio', src)
            data = {
                "cmd":"restart",
                "radio":radio_number
            }
            dl.send_service("serial", data)     # 向串口发送消息

class GatewayApp(RequestHandler):
    def post(self, cmd):
        try:
            request = self.request.body.decode('utf-8')
            request = json.loads(request)
            body= request["d"]
            if cmd == 'group':
                pass
                # gateway.set_group(data.get('gateway_list', None))
            elif cmd == 'white_list':
                ret = gw.create_whitelist(body['url'], body['md5'])
                if ret:
                    LOG.info("white list create success")
                    resp_status = "ok"
                    resp_data = {"result": "ok"}
            elif cmd == 'check_code':
                gw.set_auth_key(body['check_code'])
                resp_status = "ok"
                resp_data = {"result":"ok"}
        except EpdException as e:
            LOG.error(e.__repr__())
            resp_status = "err",
            resp_data = {"msg":e.message}
        finally:
            upload = uplink.Upload()
            resp = {
                "id": request['id'],
                "from": request['from'],
                "status": resp_status,
                "command": request['command'],
                "d": resp_data
            }
            upload.send(resp, topic="dma/cmd/resp")
            # self.write(e.__str__())

