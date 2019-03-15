import logging

__loggerList = []

def write_log_file(name, msg, level='info'):
    logger = logging.getLogger(name=name)
    # __loggerList.append(name)
    if name not in __loggerList:
        __loggerList.append(name)
        logger.setLevel(logging.INFO)
        fmt = "[%(asctime)s]-%(message)s"
        f = logging.Formatter(fmt=fmt)
        handler = logging.FileHandler('/var/log/gateway/%s.log' % name)
        handler.setFormatter(f)
        logger.addHandler(handler)
        
    logger.info(msg)

    
    
# for i in range(100):
#     write_log_file('system', str(i))



