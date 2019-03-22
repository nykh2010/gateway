from threading import Thread
import json
from time import time, sleep
from configparser import ConfigParser
from epd_log import write_log_file
import sys
import os
# import task
import uplink
from downlink import dl
import time
from gateway import gw

sys.path.append(os.path.dirname(__file__))

# 下行协议
class Handle:
    def func(self):
        pass
    
    def upload(self, data):
        up = uplink.Upload()
        up.send(data)

class HeartRequest(Handle):
    def func(self, request):
        time_str = time.strftime("%Y-%m-%d %H:%M:%S",time.localtime(time.time()))
        write_log_file('connect', '%s heart beat' % request['device_id'])
        # data = {
        #     "table_name":"white_list",
        #     "method":"connect",
        #     "device_id":request['device_id'],
        #     "time":time_str
        # }
        data = {
            "table_name":"sql",
            "sql_cmd":"update `white_list` set `last_connect_time`='%s' where `device_id`='%s';" % (time_str, request['device_id'])
        }
        dl.send_service('database', data)
        self.upload(request)
    
    def upload(self, data):
        # 上传信息
        send_data = {}
        send_data['nid'] = data['device_id']
        send_data['data_id'] = data['data_id']
        send_data['battery'] = data.get('battery', 100)
        # from uplink import upload
        super().upload(send_data)
        

class RegisterRequest(Handle):
    def func(self, data):
        pass

class TaskRequest(Handle):
    def func(self, data):
        task_id = data['task_id']
        status = data['status']
        success_list = data.get('success_list', None)
        fail_list = data.get('fail_list', None)
        request = {
            "table_name":"sql",
            "sql_cmd":"update `task` set `status`='%d' where `task_id`='%s';" % (status, task_id)
        }
        dl.send_service('database', request)
        
        if success_list:
            chunk = ""
            for success in success_list:
                chunk += "`device_id`=%s or" % success
            chunk = chunk[:-2]
            print(chunk)
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
            # print(chunk)
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
        super().upload(data)

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
            if cmd == 'create':
                data = {
                    "table_name":"sql",
                    "sql_cmd":"select `status` from `task` where `task_id`=%s" % request['task_id']
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
                                    VALUES (%s,%s,%s,%s, %s, %d); " % \
                                    (request['task_id'], request['data_id'], request['data_url'], request['start_time'], request['end_time'], 1)
                            }
                        else:
                            data = {
                                "table_name":"sql",
                                "sql_cmd":"update `task` set (`data_id`=%s, `data_url`=%s, `start_time`=%s, `end_time`=%s, `status`=1) where `task_id`=%s;" %\
                                    (request['data_id'], request['data_url'], request['start_time'], request['end_time'], request['task_id'])
                            }

                        result = dl.send_service('database', data)
                        if result['status'] == 400:
                            raise HTTPError(500, log_message=result['msg'])
                        
                        # 生成执行表
                        task_id = request['task_id']
                        chunk = ""
                        for device in request['device_list']:
                            chunk += '(%s, %s, 1),' % (device, task_id)
                        chunk = chunk[:-1]
                        data = {
                            "table_name":"sql",
                            "sql_cmd":"delete from `execute`;\
                                insert into `execute`(`device_id`,`task_id`,`status`) values \
                                %s;" % chunk
                        }
                        result = dl.send_service('database', data)
                        if result['status'] == 400:
                            raise HTTPError(500, log_message=result['msg'])
                        # 检测文件是否存在
                        if os.path.exists('/media/%s.bin' % request['data_id']):
                            pass
                        else:
                            ret_code = os.system("wget -c %s -t 10 -T 5 -P /media/%s.bin" % (request['data_url'], request['data_id']))
                            write_log_file('system', 'task data download complete, ret code:%d' % ret_code)
                        data = {
                            "cmd":"task",
                            "method":"create",
                            "task_id":request['task_id'],
                            "status":1
                        }
                        dl.send_service('serial', data)
                        raise HTTPError(200)
            elif cmd == 'cancel':
                # cancel task
                data = {
                    "cmd":"task",
                    "method":"cancel",
                    "task_id":request['task_id']
                }
                dl.send_service('serial', data)
                
                data = {
                    "table_name":"sql",
                    "sql_cmd":"delete from task where task_id='%s';" \
                        % request['task_id']
                }
                dl.send_service('database', data)
            elif cmd == 'confirm':
                # confirm task 
                data = {
                    "table_name":"sql",
                    "sql_cmd":"update `task` set (`start_time`='%s', `end_time`='%s', `status`=2) where `task_id`='%s';" % \
                        (request['start_time'], request['end_time'], request['task_id'])
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
            write_log_file('system', '%s setup radio parameters' % src)
            data = {
                "cmd":"update",
                "radio":radio_number
            }
        elif cmd == 'restart':
            write_log_file('system', '%s restart radio' % src)
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
            if cmd == 'group':
                pass
                # gateway.set_group(data.get('gateway_list', None))
            elif cmd == 'auth':
                white_list = request['payload']['d'].get('white_list', None)
                if white_list:
                    chunk = ""
                    for device in white_list:
                        chunk += "('%s')," % device
                    data = {
                        "table_name":"sql",
                        "sql_cmd":"create table add_list(`device_id`);\
                            insert into add_list(`device_id`) values %s;\
                            insert into white_list(`device_id`) select `device_id` from \
                            add_list except select `device_id` from white_list;\
                            drop table add_list;" % chunk[:-1]
                    }
                    dl.send_service('database', data)
                # auth_key = request.get('auth_key', None)
                # if auth_key:
                #     # gw.set_auth_key(auth_key)
        except Exception as e:
            self.write(e.__str__())
