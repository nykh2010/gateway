import urllib
import os
import base64
import json
import http

os.chdir(os.path.dirname(os.path.abspath(__file__)))
with open("data", "rb") as f:
    content = f.read() 
    res = base64.b64encode(content)

headers = {
    'Content-Type':'application/json',
    # 'accept-encoding':'gzip,deflate'
}
data = {
    "start_time":"2019-04-16 20:00:00",
    "end_time":"2019-04-16 23:00:00",
    "image_data_id":2,
    "image_data": res.decode('utf-8'),
    "iot_dev_list":[
        "0000000000000001",
        "0102030405060001"
    ],
    "gw_groups":["demo"]
}
request_data = json.dumps(data).encode()

req = urllib.request.Request(r"http://10.252.97.88/iotgw/api/v1/tasks",data=request_data, headers=headers, method='POST')
# resp = urllib.request.urlopen("http://10.252.97.88/iotgw/api/v1/tasks",data=request_data)
resp = urllib.request.urlopen(req)
content = resp.read()
print(content)