import hashlib

with open('md5file.md5', 'w') as f:
    device_id = ["000000000000%04d\n" % i for i in range(0,100)]
    f.writelines(device_id)

with open('md5file.md5', 'r') as f:
    device_ids = f.readlines()
    md5_obj = hashlib.md5()
    for device_id in device_ids:
        md5_obj.update(device_id.encode('utf-8'))
    print(md5_obj.hexdigest())