from threading import Thread
import json
from time import time, sleep
from configparser import ConfigParser
from epd_log import write_log_file
import sys
import os
import task
import uplink
from downlink import dl
import time

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
        from gateway import gateway
        if gateway.check_white_list(data['id']):
            ret = {}
            ret['key'] = gateway.get_auth_key()
            return ret 

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
        # request = {
        #     "table_name":"task",
        #     "method":"result",
        #     "task_id":task_id,
        #     "status":status
        # }
        dl.send_service('database', request)
        
        chunk = ""
        for success in success_list:
            chunk += "`device_id`=%s or" % success
        chunk = chunk[:-2]
        print(chunk)
        request = {
            "table_name":"sql",
            "sql_cmd":"update `execute` set `status`=2 where %s;" % chunk
        }

        # request = {
        #     "table_name":"execute",
        #     "method":"result",
        #     "task_id":task_id,
        #     "status":status,
        #     "success_list":success_list,
        #     "fail_list":fail_list
        # }
        dl.send_service('database', request)
        
        chunk = ""
        for fail in fail_list:
            chunk += "`device_id`=%s or" % fail
        chunk = chunk[:-2]
        print(chunk)
        request = {
            "table_name":"sql",
            "sql_cmd":"update `execute` set `status`=3 where %s;" % chunk
        }
        dl.send_service('database', request)
        send_data = {
            "task_id":task_id,
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
                    "sql_cmd":"select `status` from `white_list` where `task_id`=%s" % request['task_id']
                }
                # data = {
                #     "table_name":"task",
                #     "method":"check_id",
                #     "task_id":request['task_id']
                # }
                resp = dl.send_service('database', data)
                print(resp)
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
                            # data = {
                            #     "table_name":"task",
                            #     "method":"create",
                            #     "task_id":request['task_id'],
                            #     "data_id":request['data_id'],
                            #     "data_url":request['data_url']
                            # }
                            data = {
                                "table_name":"sql",
                                "sql_cmd":"update `task` set (`data_id`=%s, `data_url`=%s, `start_time`=%s, `end_time`=%s, `status`=1) where `task_id`=%s;" %\
                                    (request['data_id'], request['data_url'], request['start_time'], request['end_time'], request['task_id'])
                            }

                        result = dl.send_service('database', data)
                        if result['status'] == 400:
                            raise HTTPError(500, log_message=result['msg'])
                        
                        # 生成执行表
                        data = {
                            "table_name":"sql",
                            "sql_cmd":"delete from `execute`;\
                                insert into `execute`()"
                        }
                        
                        # data = {
                        #     "table_name":"execute",
                        #     "method":"create",
                        #     "task_id":request['task_id'],
                        #     "device_list":request['device_list']
                        # }
                        result = dl.send_service('database', data)
                        if result['status'] == 400:
                            raise HTTPError(500, log_message=result['msg'])
                        data = {
                            "cmd":"task",
                            "method":"create",
                            "task_id":request['task_id'],
                            "status":1
                        }
                        dl.send_service('serial', data)
                    # resp['result'][0][]
                    
                # if task.check_status(task_id=data['task_id']):
                #     # task state
                #     result, msg = task.create_task(data)
                #     if not result:
                #         raise HTTPError(400, msg)
                #     else:
                #         self.set_status(200, msg)
                #         return
            elif cmd == 'cancel':
                # cancel task
                data = {
                    "cmd":"task",
                    "method":"cancel",
                    "task_id":request['task_id']
                }
                dl.send_service('serial', data)
                
                data = {
                    "table_name":"task",
                    "method":"cancel",
                    "task_id":request['task_id']
                }
                dl.send_service('database', data)
                # result, msg = task.cancel_task(data['task_id'])
                # if not result:
                #     raise HTTPError(400, msg)
                # else:
                #     self.set_status(200, msg)
                #     return
            elif cmd == 'confirm':
                # confirm task
                pass
                # task.confirm_task(data)
            else:
                raise HTTPError(404)
        except Exception as e:
            self.write(e.__str__())

class RadioApp(RequestHandler):
    def post(self, cmd):
        body = self.request.body.decode('utf-8')
        body = json.loads(body)
        src = "local" if body['from'] == 'local' else 'remote'
        if cmd == 'update':
            # protocol.set_radio()
            write_log_file('system', '%s setup radio parameters' % src)
            # print('radio update')
        elif cmd == 'restart':
            # protocol.restart_service()
            write_log_file('system', '%s restart radio' % src)

class GatewayApp(RequestHandler):
    def post(self, cmd):
        try:
            request = self.request.body.decode('utf-8')
            request = json.loads(request)
            if cmd == 'group':
                pass
                # gateway.set_group(data.get('gateway_list', None))
            elif cmd == 'auth':
                white_list = request.get('white_list', None)
                if white_list:
                    data = {
                        "table_name":"white_list",
                        "method":"create",
                        "device_list":white_list
                    }
                    dl.send_service('database', data)
        except Exception as e:
            self.write(e.__str__())
            
# class Protocol:
#     __app = [
#         ('heart', HeartRequest),
#         ('register', RegisterRequest),
#     ]
#     def __init__(self, ver=None):
#         self.__ver = ver        # 协议版本
#         self.__error = False    # 

#     def route(self, data_dict):
#         for d in self.__app:
#             if data_dict['cmd'] == d[0]:
#                 return d[1]

#     def handle(self, data_dict):
#         c = self.route(data_dict)
#         self.__handle = c()
#         return self.__handle.func(data_dict)

#     def service(self, *args):
#         print('protocol service start...')
#         ver = self.get_ver(timeout=60)
#         if not ver:
#             downlink.set_error()
#             write_log_file('system', 'radio module error')
#             print('radio module error...')
#         while not self.__error:
#             # 读取downlink接收队列
#             data_dict = downlink.read_buff()
#             if data_dict is None:
#                 continue
#             ret = self.handle(data_dict)
#             if ret is not None:
#                 data = self.mutex(ret)
#                 downlink.write_buff(data)

#     def get_ver(self, timeout=None):
#         end_time = None
#         if timeout is not None:
#             start_time = time()
#             end_time = start_time + timeout
#         # data = self.mutex(cmd_type=STATUS)
#         send_data = {}
#         send_data['cmd'] = 'read_ver'
#         while True:
#             if end_time is not None:
#                 if time() - end_time >= 0:
#                     return False
#             downlink.write_buff(send_data)
#             data = downlink.read_buff()
#             try:
#                 if data['cmd'] == 'read_ver':
#                     self.__ver = data['ver']
#                     return True
#             except:
#                 pass
#             sleep(1)

#     def set_radio(self):
#         radioConfig = ConfigParser()
#         radioConfig.read("/etc/config/radio.ini")
#         msg = {}
#         msg['cmd'] = 'set_radio'
#         msg['data'] = {}
#         for k,v in radioConfig.items('radio'):
#             msg['data'][k] = v
#         downlink.write_buff(msg)

#     def restart_service(self):
#         send_data = {}
#         send_data['cmd'] = 'restart'
#         downlink.write_buff(send_data)
#         self.stop_service()
#         sleep(1)
#         self.start_service()

#     def mutex(self, *args, **kwargs):
#         cmd_type = kwargs['cmd']
#         ret = {}
#         ret['cmd'] = int(cmd_type)
#         return ret

#     def stop_service(self):
#         self.__error = True
#         self.__thread.join()

#     def start_service(self):
#         self.__thread = Thread(target=self.service, name='protocol')
#         self.__thread.start()

# protocol = Protocol()