from configparser import ConfigParser
import os
# import json

PATH = os.path.join(os.path.dirname(__file__), '../../config')

class Config:
    def __init__(self,name):
        try:
            self.__name = name
            self.__config = ConfigParser()
            self.__config.read(os.path.join(PATH, "%s.ini" % name))
            for k,v in self.__config.items(name):
                self.__setattr__(k, v)
        except Exception as e:
            print(e)
            self.__config.add_section(self.__name)
            self.save()
    
    def set_item(self, name, value):
        self.__setattr__(name, value)
        self.__config.set(self.__name, name, value)
    
    def save(self):
        with open(os.path.join(PATH, "%s.ini" % self.__name), "w") as f:
            self.__config.write(f)

# SERVICE_PATH = os.path.join(os.path.dirname(__file__), '../../config.json')

# class Config:
#     old_config = {}
#     def __init__(self, path=PATH):
#         with open(path, 'r') as f:
#             data = f.read(-1)
#             constr = json.loads(data)
#             # print(constr['default-config'])
#             self.__dict__ = constr

#     def save(self):
#         with open(PATH, 'w') as f:
#             conf = {}
#             for k, v in self.__dict__.items():
#                 conf[k] = v
#             # self.old_config[name] = self.__dict__
#             data = json.dumps(conf, indent=4)
#             f.write(data)


# class Service:
#     confdict = Config(path=SERVICE_PATH)
#     def __init__(self):
#         service_config = self.confdict.systemconfig
#         for k,v in service_config.items():
#             self.__setattr__(k, v)


# class Firmware:
#     confdict = Config()
#     def __init__(self):
#         firmware_config = self.confdict.firmware
#         for k,v in firmware_config.items():
#             self.__setattr__(k, v)
    
#     def set_item(self, name, value):
#         self.__setattr__(name, value)

#     def save(self):
#         self.confdict.firmware = self.__dict__
#         self.confdict.save()

# class Server:
#     confdict = Config()
#     def __init__(self):
#         server_config = self.confdict.server
#         for k,v in server_config.items():
#             self.__setattr__(k, v)
    
#     def set_item(self, name, value):
#         self.__setattr__(name, value)

#     def save(self):
#         self.confdict.server = self.__dict__
#         self.confdict.save()

class Gateway(Config):
    def __init__(self):
        super().__init__("gateway")

        gateway_config = self.confdict.gateway
        for k,v in gateway_config.items():
            self.__setattr__(k, v)
    
    def set_item(self, name, value):
        self.__setattr__(name, value)

    def save(self):
        self.confdict.gateway = self.__dict__
        self.confdict.save()

# class Wifi:
#     confdict = Config()
#     def __init__(self):
#         wifi_config = self.confdict.wifi
#         for k,v in wifi_config.items():
#             self.__setattr__(k, v)
    
#     def set_item(self, name, value):
#         self.__setattr__(name, value)

#     def save(self):
#         self.confdict.wifi = self.__dict__
#         self.confdict.save()

# class Radio:
#     confdict = Config()
#     def __init__(self):
#         radio_config = self.confdict.radio
#         for k,v in radio_config.items():
#             self.__setattr__(k, v)
    
#     def set_item(self, name, value):
#         self.__setattr__(name, value)
    
#     def save(self):
#         self.confdict.radio = self.__dict__
#         self.confdict.save()

# condict = Config("auth")
# print(dir(condict))
# print(condict.host)
# condict.set_item("host", "0.0.0.0")
# condict.save()