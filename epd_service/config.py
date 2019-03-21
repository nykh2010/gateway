from configparser import ConfigParser
import os
# import json

PATH = '/etc/gateway/config'

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
