import os
from socketserver import UnixStreamServer, StreamRequestHandler
from socket import socket, AF_UNIX
from threading import Thread
import json
import sys
from epd_log import epdlog as LOG

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
        ('serial', r'/var/run/serial.sock'),
        ('database', r'/var/run/sqlite3.sock')
    ]
    __path = r'/var/run/epdserver.sock'
    def __init__(self):
        if os.path.exists(self.__path):
            os.unlink(self.__path)
        super().__init__(self.__path, DownlinkServer)
        self.__thread = Thread(target=self.start_service)
        self.__thread.start()

    def start_service(self):
        LOG.info("downlink service start")
        self.serve_forever()

    def stop_service(self):
        LOG.info("downlink service stop")
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
                resp = self.__client.recv(1024*1024)
                if not resp:
                    raise Exception()
                resp = resp.decode('utf-8')
                content = json.loads(resp, encoding='utf-8')
            except Exception as e:
                LOG.error(e.__repr__())
                content = {
                    'status':500,
                    'msg':e.__repr__()
                }
            finally:
                self.__client.close()
                return content

dl = Downlink()