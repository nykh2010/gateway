import tornado
from tornado.httpserver import HTTPServer
from tornado.web import Application
import tornado
from threading import Thread,Semaphore,RLock
from queue import Queue
import json
# from task import task
import os 
# from gateway import gateway
# from protocol import protocol
from protocol import TaskApp, RadioApp, GatewayApp
from configparser import ConfigParser
from epd_log import epdlog as LOG
import sys
import os

sys.path.append(os.path.dirname(__file__))

token = r"MQRROJMAjKxaUy&kGMLoGc7YJDLLaiTu"

class Upload:
    # __waitRequest = []
    def send(self, payload, topic='pc_test', wait_event=None, need_wait=False, cache=False):
        # print(payload)
        if cache:
            url = r'http://127.0.0.1:7788/mqtt/publish/offlinecache'
        else:
            url = r'http://127.0.0.1:7788/mqtt/publish'
        data = {}
        data['topic'] = topic
        data['payload'] = payload

        body = json.dumps(data)
        # print(type(body))
        cmd = "curl -H 'token: %s' -d '%s' '%s'" % (token, body, url)
        LOG.info(cmd)
        os.system(cmd)
        
    # def add_wait_list(self, device_id, wait_event):
    #     request = (device_id, wait_event)
    #     self.__waitRequest.append(request)

    # def wait_event(self, device_id, wait_event):
    #     pass
        
    
    # def send_event(self, device_id, event):
    #     pass

    # def check_wait_request(self):
    #     if self.__waitRequest:
    #         pass


app = Application(
    [
        (r'/task/(.*?)', TaskApp),        
        (r'/radio/(.*?)', RadioApp),      
        (r'/gateway/(.*?)', GatewayApp)
    ]
)

class Uplink(HTTPServer):
    global app
    def initialize(self):
        # get config file
        super().initialize(request_callback=app)
        self.__host = "0.0.0.0"
        self.__port = 5000
        
    def begin(self):
        LOG.info("sever host:%s port:%d", (self.__host, self.__port))
        self.listen(self.__port, self.__host)
        tornado.ioloop.IOLoop.current(instance=False).start()

    def end(self):
        LOG.info("server stop")
        
    def send(self, payload):
        # upload data
        pass