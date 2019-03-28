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
        LOG.info('%s heart beat', request['device_id'])
        if gw.is_in_executelist(request['device_id'], request['data_id']):
            LOG.info("%s is in execute list", request['device_id'])
            status = "ok"
        else:
            status = "error"
        send_data = {
            "status":status
        }
        dl.send_service('serial', send_data)
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
        super().upload('dma/report/periph', send_data)

class RegisterRequest(Handle):
    def func(self, data):
        device_id = data['device_id']
        LOG.info("%s register", device_id)
        ret = gw.is_in_whitelist(device_id)
        if ret:
            LOG.info("%s is in white list", device_id)
            status = "ok"
        else:
            status = "error"
        send_data = {
            "status":status
        }
        dl.send_service('serial', send_data)
        

class TaskRequest(Handle):
    def func(self, data):
        task_id = data['task_id']
        status = data['status']
        # success_list = data.get('success_list', [])
        # fail_list = data.get('fail_list', [])
        try:
            ret = gw.set_task_status(task_id, status)
            if not ret:
                raise Exception()
            with open("/tmp/success") as successfile:
                content = successfile.read()
                success_list = content.split('\n')
            with open("/tmp/fail") as failfile:
                content = failfile.read()
                fail_list = content.split('\n')
        except Exception as e:
            LOG.error(e.__str__())
            return
        
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
                        "start_time":body['start_time'],
                        "end_time":body['end_time']
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

