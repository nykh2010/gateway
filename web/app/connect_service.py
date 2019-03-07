import os
import json
from config import Config

def send_to_service(service_name, path, data=None):
    conf = Config(service_name)
    d = {
        'from':'local'
    }
    if isinstance(data, dict):
        d['body'] = data
    content = json.dumps(d)
    os.system("curl http://%s:%s%s -d '%s'" % (conf.host, conf.port, path, content))

