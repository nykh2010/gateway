import logging
import sys
import logging.handlers

Format = "[%(asctime)s]-%(filename)s:%(lineno)s %(message)s"

class EpdLogger(logging.Logger):
    def __init__(self, name='epd'):
        super().__init__(name)
        self.handler = logging.handlers.RotatingFileHandler('epd.log', maxBytes=5*1024*1024, backupCount=5)
        self.formater = logging.Formatter(fmt=Format)
        self.handler.setFormatter(self.formater)
        self.addHandler(self.handler)

epdlog = EpdLogger()


# import logging

# __loggerList = []

# def write_log_file(name, msg, level='info'):
#     logger = logging.getLogger(name=name)
#     # __loggerList.append(name)
#     if name not in __loggerList:
#         __loggerList.append(name)
#         logger.setLevel(logging.INFO)
#         fmt = "[%(asctime)s]-%(message)s"
#         f = logging.Formatter(fmt=fmt)
#         handler = logging.FileHandler('/var/log/%s.log' % name)
#         handler.setFormatter(f)
#         logger.addHandler(handler)
        
#     logger.info(msg)

