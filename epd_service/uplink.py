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
from epd_log import write_log_file
import sys
import os

sys.path.append(os.path.dirname(__file__))

class Upload:
    __waitRequest = []
    def send(self, payload, topic='/dma/report/periph', wait_event=None, need_wait=False, cache=False):
        print(payload)
        if cache:
            url = r'http://localhost:7788/mqtt/publish/offlinecache'
        else:
            url = r'http://localhost:7788/mqtt/publish/'
        data = {}
        data['topic'] = topic
        data['d'] = payload

        body = json.dumps(data)
        os.system("curl %s -d '%s'" % (url, body))
        
    def add_wait_list(self, device_id, wait_event):
        request = (device_id, wait_event)
        self.__waitRequest.append(request)

    def wait_event(self, device_id, wait_event):
        pass
        
    
    def send_event(self, device_id, event):
        pass

    def check_wait_request(self):
        if self.__waitRequest:
            pass


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
        write_log_file("system", "uplink start...")
        self.listen(self.__port, self.__host)
        tornado.ioloop.IOLoop.current(instance=False).start()

    def end(self):
        write_log_file("system", "uplink stop...")
        
    def send(self, payload):
        # upload data
        pass