import os
from socketserver import UnixStreamServer, StreamRequestHandler
from socket import socket, AF_UNIX
from threading import Thread
import json
import sys

sys.path.append(os.path.dirname(__file__))

class DownlinkServer(StreamRequestHandler):
    
    def route(self, data):
        from protocol import apps        
        for app in apps:
            if data['cmd'] == app[0]:
                return app[1]
        return None
    def handle(self):
        data = self.request.recv(1024)
        data = data.decode('utf-8')
        data = json.loads(data, encoding='utf-8')
        HandleClass = self.route(data)
        handler = HandleClass()
        handler.func(data)

class Downlink(UnixStreamServer):
    __error = False
    services = [
        ('serial', r'/home/xulingfeng/serial.sock'),
        ('database', r'/home/xulingfeng/sqlite3.sock')
    ]
    # __client = socket(family=AF_UNIX)
    __path = r'/home/xulingfeng/epdserver.sock'
    def __init__(self):
        if os.path.exists(self.__path):
            os.unlink(self.__path)
        super().__init__(self.__path, DownlinkServer)
        self.__thread = Thread(target=self.start_service)
        self.__thread.start()

    def start_service(self):
        self.serve_forever()

    def stop_service(self):
        self.server_close()
        self.__thread.join()

    def send_service(self, service_name, data):
        path = ""
        for service in self.services:
            if service_name == service[0]:
                path = service[1]
                break
        if path:
            try:
                self.__client = socket(family=AF_UNIX)
                self.__client.connect(path)
                content = json.dumps(data)
                self.__client.send(content.encode('utf-8'))
                resp = self.__client.recv(1024)
                if not resp:
                    raise Exception()
                resp = resp.decode('utf-8')
                content = json.loads(resp, encoding='utf-8')
            except Exception as e:
                print(e.__repr__())
                content = None
            finally:
                self.__client.close()
                return content
            


# from serial import SerialException
# from task import task
# from threading import Thread
# import json
# from queue import Queue
# import os
# import time
# from ctypes import *

# tx_path = os.path.join(os.path.dirname(__file__), 'tx')
# rx_path = os.path.join(os.path.dirname(__file__), 'rx')

# libpath = "/usr/lib/libserialapp.so"
# '''
# int open_serial_bus (char * dev);
# int serial_bus_send_data (const char * data, int size);
# int serial_bus_receive_data (char * data, int msec);
# void serial_bus_close (void);
# '''
# serial = CDLL(libpath)

# HEAD_0 = 0x55
# HEAD_1 = 0xaa

# class Downlink:
#     __readQueue = Queue(10)
#     __errorFlag = False
#     def __init__(self):
#         try:
#             status = serial.open_serial_bus(c_char_p(bytes("/dev/ttyUSB1", 'utf-8')))
#             if status > 0:
#                 print('serial open failed')
#                 raise SerialException
#         except:
#             self.__errorFlag = True

#     def start(self):
#         self._recvThread = Thread(target=self.recive, name="serial")
#         self._recvThread.start()

#     def read_buff(self):
#         # 字节码转字典
#         try:
#             return self.__readQueue.get(timeout=3)
#         except:
#             return None

#     def write_buff(self, data):
#         # 字典转字节码
#         print(data)
#         if not isinstance(data, dict):
#             return
#         content = json.dumps(data)
#         s = len(content)
#         status = serial.serial_bus_send_data(c_char_p(bytes(content,'utf-8')), s)
#         print(status)
#         # content = json.dumps(data)
#         # os.write(self.__serialTx, content.encode('utf-8'))
    
#     def recive(self):
#         print("serial recv start...")
#         while True:
#             data = bytes(256)
#             status = serial.serial_bus_receive_data(c_char_p(data), 500)
#             if status:
#                 content = data[:status]
#                 print("serial recv:")
#                 for d in content:
#                     print("%s" % hex(d),end=' ')
#                 print()
#                 content = self.parse(content)
#                 self.__readQueue.put_nowait(content)

#     def parse(self, content):
#         ret = {}
#         ret['cmd'] = content[0]
#         ret['data'] = content[2:]
#         return ret

#     def set_error(self):
#         self.__errorFlag = True

#     def stop(self):
#         self._recvThread.join()
#         # self.serial_fd.close()

dl = Downlink()