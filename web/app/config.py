import os
import json

PATH = os.path.join(os.path.dirname(__file__), '../../default_config.json')
SERVICE_PATH = os.path.join(os.path.dirname(__file__), '../../config.json')

class Config:
    old_config = {}
    def __init__(self, path=PATH):
        with open(path, 'r') as f:
            data = f.read(-1)
            constr = json.loads(data)
            # print(constr['default-config'])
            self.__dict__ = constr

    def save(self):
        with open(PATH, 'w') as f:
            conf = {}
            for k, v in self.__dict__.items():
                conf[k] = v
            # self.old_config[name] = self.__dict__
            data = json.dumps(conf, indent=4)
            f.write(data)


class Service:
    confdict = Config(path=SERVICE_PATH)
    def __init__(self):
        service_config = self.confdict.systemconfig
        for k,v in service_config.items():
            self.__setattr__(k, v)


class Firmware:
    confdict = Config()
    def __init__(self):
        firmware_config = self.confdict.firmware
        for k,v in firmware_config.items():
            self.__setattr__(k, v)
    
    def set_item(self, name, value):
        self.__setattr__(name, value)

    def save(self):
        self.confdict.firmware = self.__dict__
        self.confdict.save()

class Server:
    confdict = Config()
    def __init__(self):
        server_config = self.confdict.server
        for k,v in server_config.items():
            self.__setattr__(k, v)
    
    def set_item(self, name, value):
        self.__setattr__(name, value)

    def save(self):
        self.confdict.server = self.__dict__
        self.confdict.save()

class Gateway:
    confdict = Config()
    def __init__(self):
        gateway_config = self.confdict.gateway
        for k,v in gateway_config.items():
            self.__setattr__(k, v)
    
    def set_item(self, name, value):
        self.__setattr__(name, value)

    def save(self):
        self.confdict.gateway = self.__dict__
        self.confdict.save()

class Wifi:
    confdict = Config()
    def __init__(self):
        wifi_config = self.confdict.wifi
        for k,v in wifi_config.items():
            self.__setattr__(k, v)
    
    def set_item(self, name, value):
        self.__setattr__(name, value)

    def save(self):
        self.confdict.wifi = self.__dict__
        self.confdict.save()

class Radio:
    confdict = Config()
    def __init__(self):
        radio_config = self.confdict.radio
        for k,v in radio_config.items():
            self.__setattr__(k, v)
    
    def set_item(self, name, value):
        self.__setattr__(name, value)
    
    def save(self):
        self.confdict.radio = self.__dict__
        self.confdict.save()

# condict = Config()
# print(dir(condict))
# print(condict.auth)