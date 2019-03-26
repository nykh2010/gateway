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
        sql_cmd = "update `white_list` set `last_connect_time`='%s' where `device_id`='%s';" % (time_str, request['device_id'])
        LOG.info("query database: %s", sql_cmd)
        data = {
            "table_name":"sql",
            "sql_cmd": sql_cmd
        }
        dl.send_service('database', data)
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
        success_list = data.get('success_list', None)
        fail_list = data.get('fail_list', None)
        sql_cmd = "update `task` set `status`='%d' where `task_id`='%s';" % (status, task_id)
        request = {
            "table_name":"sql",
            "sql_cmd": sql_cmd
        }
        LOG.info("query database:%s", sql_cmd)
        ret = dl.send_service('database', request)
        
        if success_list:
            chunk = ""
            for success in success_list:
                chunk += "`device_id`=%s or" % success
            chunk = chunk[:-2]
            # LOG.info(chunk)
            request = {
                "table_name":"sql",
                "sql_cmd":"update `execute` set `status`=2 where %s;" % chunk
            }
            dl.send_service('database', request)
        if fail_list:
            chunk = ""
            for fail in fail_list:
                chunk += "`device_id`=%s or" % fail
            chunk = chunk[:-2]
            LOG.info(chunk)
            request = {
                "table_name":"sql",
                "sql_cmd":"update `execute` set `status`=3 where %s;" % chunk
            }
            dl.send_service('database', request)
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
                data = {
                    "table_name":"sql",
                    "sql_cmd":"select `status` from `task` where `task_id`=%s" % body['task_id']
                }
                resp = dl.send_service('database', data)
                # print(resp)
                if resp['status'] == 200:
                    task_status = None
                    if resp['result']:
                        record = resp['result'][0]
                        task_status = int(record['status'])
                    if task_status in (0,1,4,5) or task_status is None:
                        # 任务处于可覆盖状态
                        if task_status is None:
                            # 如果查询结果位空，则需要先添加记录
                            data = {
                                "table_name":"sql",
                                "sql_cmd":"insert into `task`(`task_id`,`data_id`,`data_url`,`start_time`, `end_time`, `status`) \
                                    VALUES ('%s','%s','%s','%s', '%s', %d); " % \
                                    (body['task_id'], body['image_data_id'], body['image_data_url'], body['start_time'], body['end_time'], 1)
                            }
                        else:
                            data = {
                                "table_name":"sql",
                                "sql_cmd":"update `task` set (`data_id`='%s', `data_url`='%s', `start_time`='%s', `end_time`='%s', `status`=1) where `task_id`='%s';" %\
                                    (body['image_data_id'], body['image_data_url'], body['start_time'], body['end_time'], body['task_id'])
                            }
                        result = dl.send_service('database', data)
                        if result['status'] != 200:
                            raise HTTPError(500, log_message=result['msg'])
                        
                        # 生成执行表
                        task_id = body['task_id']
                        iot_dev_list_url = body['iot_dev_list_url']
                        os.system('wget -c %s -O iot_dev_list' % iot_dev_list_url)
                        hash_obj = hashlib.md5()
                        with open('iot_dev_list', 'rb') as f:
                            dev_list = f.readlines()
                            chunk = ""
                            for dev in dev_list:
                                chunk += '(%s, %s, 1),' % (dev, task_id)
                                hash_obj.update(dev)
                            hash_code = hash_obj.hexdigest()
                        if body['iot_dev_list_md5'] != hash_code:
                            raise HTTPError(400, "iot dev list md5 check failed")
                        
                        data = {
                            "table_name":"sql",
                            "sql_cmd":"delete from `execute`;\
                                insert into `execute`(`device_id`,`task_id`,`status`) values \
                                %s;" % chunk
                        }
                        result = dl.send_service('database', data)
                        if result['status'] != 200:
                            raise HTTPError(500, log_message=result['msg'])
                        # 检测文件是否存在
                        if os.path.exists('/media/%s.bin' % str(body['image_data_id'])):
                            pass
                        else:
                            ret_code = os.system("wget -c %s -O /media/%s.bin" % (body['image_data_url'], str(body['image_data_id'])))
                            LOG.info('task data download complete, ret code:%d', ret_code)
                            with open('/media/%s.bin' % body['image_data_id'], 'rb') as f:
                                hash_obj = hashlib.md5()
                                while True:
                                    content = f.read(1024)
                                    if not content:
                                        break
                                    hash_obj.update(content)
                                hash_code = hash_obj.hexdigest()
                                if body['image_data_md5'] != hash_code:
                                    raise HTTPError(400, "data file md5 check failed")
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
                        LOG.info("task could not create, task status: %d", task_status)
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
                    gw.get_task_status()
                    upload.send(resp, topic='dma/cmd/resp')
            elif cmd == 'cancel':
                # cancel task
                data = {
                    "cmd":"task",
                    "method":"cancel",
                    "task_id":body['task_id']
                }
                dl.send_service('serial', data)
                
                data = {
                    "table_name":"sql",
                    "sql_cmd":"delete from task where task_id='%s';" \
                        % body['task_id']
                }
                dl.send_service('database', data)
                # 上传处理结果
                upload = uplink.Upload()
                resp = {
                    "id": request['id'],
                    "from": request['from'],
                    "status": "ok",
                    "command": request['command'],
                    "d":{
                        "result":"ok"
                    }
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
                white_list_url = body['url']
                white_list_md5 = body['md5']
                LOG.info("get white list update request")
                os.system('wget -c %s -O white_list' % white_list_url)
                hash_obj = hashlib.md5()
                with open('white_list', 'rb') as white_list_file:
                    chunk = ""
                    for line in white_list_file.readlines():
                        hash_obj.update(line)
                        chunk += "('%s')," % (line[:-1].decode('utf-8'))
                hash_code = hash_obj.hexdigest()

                if hash_code != white_list_md5:
                    LOG.error('white list file md5 check failed')
                    raise EpdException(code=400, message="md5 check failed")
                else:
                    data = {
                        "table_name": "sql",
                        "sql_cmd": "delete from `white_list`;\
                            insert into white_list(`device_id`) values %s;" % chunk[:-1]
                    }
                    dl.send_service('database', data)
                    gw.get_whitelist_md5()
                    LOG.info('update database, white_list table')
                    resp_status = "ok"
                    resp_data = {"result":"ok"}
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

