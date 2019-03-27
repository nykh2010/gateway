from random import randint
from time import time
from config import Config
import uplink
import downlink
import threading
import hashlib
from epd_log import epdlog as LOG
from task import EpdTask
import os

STEP_TIME = 20*60       # 任务间隔时间
handler_interval = 10   # 10秒产生一次定时任务

class Gateway(Config):
    # 网关参数
    __authKey = ""
    __taskId = 0
    __taskStatus = 0
    __whiteListMD5 = ""
    __whitelist = set()
    def __init__(self):
        super().__init__('gateway')
        # 启动定时任务
        self.__whiteListMD5 = self.get_whitelist()
        self.__taskId, self.__taskStatus = self.get_task_status()
        timer = threading.Timer(handler_interval, self.timer_handler)
        timer.start()

    def get_whitelist(self):
        try:
            with open(self.white_list_url, 'r') as whitelistfile:
                content = whitelistfile.read()
                self.__whitelist = set(content.split('\n'))
                hash_obj = hashlib.md5()
                hash_obj.update(content.encode('utf-8'))
                return str(hash_obj.hexdigest())
        except Exception as e:
            LOG.error("get white list failed: %s" % e.__str__())
            return ""

    def create_whitelist(self, url, md5):
        os.system("wget -c %s -O %s.tmp" % (url, self.white_list_url))
        ret = self.check_whitelist_integrity(md5)
        if not ret:
            return False
        else:
            os.system("mv %s.tmp %s" % (self.white_list_url, self.white_list_url))
            self.get_whitelist()
            return True
    
    def check_whitelist_integrity(self, md5):
        hash_obj = hashlib.md5()
        with open("%s.tmp" % self.white_list_url, "rb") as whitelistfile:
            hash_obj.update(whitelistfile.read())
            hash_code = hash_obj.hexdigest()
        if md5 != hash_code:
            return False
        return True

    def is_in_whitelist(self, device_id):
        return device_id in self.__whitelist

    def is_in_executelist(self, device_id, data_id):
        epd_task = EpdTask()
        if epd_task.image_data_id != device_id:
            return False
        if not epd_task.is_in_executelist(device_id):
            return False
        return True
    
    def create_task(self, task_id, image_data_id, image_data_url,\
            image_data_md5, iot_dev_list_md5, iot_dev_list_url, start_time, end_time):
        os.system("wget -c %s -O %s.tmp" % (image_data_url, self.data_url))
        os.system("wget -c %s -O %s.tmp" % (iot_dev_list_url, self.execute_url))
        ret = self.check_task_integrity(image_data_md5, iot_dev_list_md5)
        if not ret:
            return False

        # 0-none 1-sleep 2-ready 3-run 4-finish 5-suspend
        if self.__taskId is None or (task_id != self.__taskId and self.__taskStatus in ('','none','sleep','finish','suspend')):
            # 保存任务状态
            epd_task = EpdTask()
            epd_task.set_item('task_id', str(task_id))
            epd_task.set_item('image_data_id', str(image_data_id))
            epd_task.set_item('image_data_url', image_data_url)
            epd_task.set_item('start_time', start_time)
            epd_task.set_item('end_time', end_time)
            epd_task.set_item('task_status', 'sleep')
            epd_task.save()
            # 保存待更新列表
            os.system("mv %s.tmp %s" % (self.data_url, self.data_url))
            os.system("mv %s.tmp %s" % (self.execute_url, self.execute_url))
            self.__taskId, self.__taskStatus = self.get_task_status()     # 更新任务状态
            return True
        else:
            return False

    def cancel_task(self, task_id):
        if self.__taskId != task_id:
            return False
        epd_task = EpdTask()
        epd_task.set_item('task_status', 'none')
        epd_task.set_item('task_id', '0')
        epd_task.save()
        self.__taskId, self.__taskStatus = self.get_task_status()     # 更新任务状态
        return True

    def set_task_status(self, task_id, task_status):
        if self.__taskId != task_id:
            return False
        epd_task = EpdTask()
        epd_task.set_item('task_status', task_status)
        epd_task.set_item('task_id', '0')
        epd_task.save()
        self.__taskId, self.__taskStatus = self.get_task_status()     # 更新任务状态
        return True

    def check_task_integrity(self, image_data_md5, iot_dev_list_md5):
        hash_obj = hashlib.md5()
        with open("%s.tmp" % self.data_url, "rb") as datafile:
            hash_obj.update(datafile.read())
            hash_code = hash_obj.hexdigest()
        if image_data_md5 != hash_code:
            return False
        
        hash_obj = hashlib.md5()
        with open("%s.tmp" % self.execute_url, "rb") as devfile:
            hash_obj.update(devfile.read())
            hash_code = hash_obj.hexdigest()
        if iot_dev_list_md5 != hash_code:
            return False
        return True

    def get_task_status(self):
        epd_task = EpdTask()
        return int(epd_task.task_id), epd_task.task_status
    
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