from random import randint
from time import time
from config import Config
import uplink
import downlink
import threading
import hashlib
from epd_log import epdlog as LOG

STEP_TIME = 20*60       # 任务间隔时间
handler_interval = 10   # 10秒产生一次定时任务

class Gateway(Config):
    # 网关参数
    __authKey = None
    __taskId = None
    __taskStatus = None
    __whiteListMD5 = None
    def __init__(self):
        super().__init__('gateway')
        # 启动定时任务
        self.__whiteListMD5 = self.get_whitelist_md5()
        self.__taskId, self.__taskStatus = self.get_task_status()
        timer = threading.Timer(handler_interval, self.timer_handler)
        timer.start()

    def get_whitelist_md5(self):
        data = {
            "table_name":"sql",
            "sql_cmd":"select `device_id` from white_list;"
        }
        result = downlink.dl.send_service('database', data)
        if result is None:
            return ""
        else:
            md5_obj = hashlib.md5()
            for res in result['result']:
                md5_obj.update(("%s\n" % res['device_id']).encode('utf-8'))
            md5_code = md5_obj.hexdigest()
            LOG.info("white_list md5 code:%s", md5_code)
            self.__whiteListMD5 = md5_code
            return md5_code
    
    def get_task_status(self):
        data = {
            'table_name': 'sql',
            'sql_cmd': 'select `task_id`,`status` from task;'
        }
        result = downlink.dl.send_service('database', data)
        LOG.info('get task status result: %s', result['status'])
        if result['result']:
            self.__taskId = result['task_id']
            self.__taskStatus = result['status']
            return result['task_id'], result['status']
        return 0, 0
    
    def get_auth_key(self):
        # 从文件中获取auth_key
        return self.key

    def set_auth_key(self, key):
        self.set_item('key', str(key))
        self.save()
    
    def timer_handler(self, args=None):
        '''
            定时任务
        '''
        data = {
            "d": {
                'task_id': self.__taskId,
                'whitelist_md5': self.__whiteListMD5,
                'check_code': int(self.key)
            }
        }
        upload = uplink.Upload()
        upload.send(data, topic='gateway/report/status')
        timer = threading.Timer(handler_interval, self.timer_handler)
        timer.start()
    
    # def set_group(self, gateways):
    #     '''设置组内网关个数'''
    #     if gateways is None:
    #         return False
    #     if self.gateway_id not in gateways:
    #         return False
    #     else:
    #         self.gateway_list = gateways
    #         self.gateway_list.sort()
    #         self.sn = self.gateway_list.index(self.gateway_id)
    #         return True

    # def set_white_list(self, white_list):
    #     if white_list is None:
    #         return
    #     with open('/etc/config/white_list', 'w') as whiteListFile:
    #         for device in white_list:
    #             whiteListFile.writelines(device)
    #         self.__whiteList = set(white_list)      # 写入白名单

    # def set_auth_key(self, auth_key):
    #     if auth_key is None:
    #         return
    #     self.set_item('auth_key', auth_key)
    #     self.save()

    # def check_white_list(self, id):
    #     if id in self.__whiteList:
    #         return True
    #     else:
    #         return False
    
    # def get_auth_key(self):
    #     return self.__authKey

    
    
    # def get_worktime(self, task_id):
    #     if task_id not in self.__workTimeQueue:
    #         self.__workTimeQueue.append(task_id)
    #     return self.__workTimeQueue.index(task_id)

    # def get_starttime(self, start_time, time_sn):
    #     t = start_time + time_sn * STEP_TIME * self.count
    #     return t-time()
        

gw = Gateway()