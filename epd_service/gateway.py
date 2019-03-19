from random import randint
from time import time
from config import Config

STEP_TIME = 20*60       # 任务间隔时间

class Gateway(Config):
    # 网关参数
    __authKey = None
    def __init__(self):
        super().__init__('gateway')
        # 启动后检测task内容是否完整
    
    def set_group(self, gateways):
        '''设置组内网关个数'''
        if gateways is None:
            return False
        if self.gateway_id not in gateways:
            return False
        else:
            self.gateway_list = gateways
            self.gateway_list.sort()
            self.sn = self.gateway_list.index(self.gateway_id)
            return True

    def set_white_list(self, white_list):
        if white_list is None:
            return
        with open('/etc/config/white_list', 'w') as whiteListFile:
            for device in white_list:
                whiteListFile.writelines(device)
            self.__whiteList = set(white_list)      # 写入白名单

    def set_auth_key(self, auth_key):
        if auth_key is None:
            return
        self.set_item('auth_key', auth_key)
        self.save()

    def check_white_list(self, id):
        if id in self.__whiteList:
            return True
        else:
            return False
    
    def get_auth_key(self):
        return self.__authKey
    
    def get_worktime(self, task_id):
        if task_id not in self.__workTimeQueue:
            self.__workTimeQueue.append(task_id)
        return self.__workTimeQueue.index(task_id)

    def get_starttime(self, start_time, time_sn):
        t = start_time + time_sn * STEP_TIME * self.count
        return t-time()
        

gw = Gateway()