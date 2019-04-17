import base64
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

with open("data", "rb") as f:
    content = f.read()
    res = base64.b64encode(content)

with open("base_res", "wb") as f:
    f.write(res)