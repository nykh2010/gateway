from random import randint
from time import time

STEP_TIME = 20*60       # 任务间隔时间

class Gateway:
    # 网关参数
    __workTimeQueue = []
    __authKey = None
    __whiteList = set()
    def __init__(self):
        self.gateway_id = str(randint(0,10))
        self.sn = 0
        self.gateway_list = [self.gateway_id]      # 组内只有一个网关
        self.count = len(self.gateway_list)
        with open('/etc/config/white_list', 'w+') as whiteListFile:
            devices = whiteListFile.readlines()
            self.__whiteList = set(devices)
        with open('/etc/config/auth_key', 'w+') as authKeyFile:
            self.__authKey = authKeyFile.read()
    
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
        with open('/etc/config/auth_key', 'w') as authKeyFile:
            authKeyFile.write(auth_key)
        self.__authKey = auth_key

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
        

gateway = Gateway()