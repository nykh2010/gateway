from tornado.web import RequestHandler
from auth import auth
from config import Config
from tornado.log import app_log as LOG

class WifiHandler(RequestHandler):
    @auth
    def get(self):
        wifi = Config("wifi") 
        # print(dir(wifi))       
        self.render("wifi_setup.html", wifi=wifi)
    
    def post(self, method):
        ret = {}
        if method == 'update':
            wifi = Config("wifi")
            try:
                wifi.set_item('enable', self.get_argument('enable'))
                if wifi.enable == 'true':
                    wifi.set_item('ssid', self.get_argument('ssid'))
                    wifi.set_item('lbl', self.get_argument('lbl'))
                    if wifi.lbl == '3' or wifi.lbl == '4':
                        wifi.set_item('passwd', self.get_argument('key'))
                ret['status'] = 'success'
                wifi.save()
            except Exception as e:
                LOG.error(e.__str__())
                ret['status'] = 'failed'
                ret['err_msg'] = e.__repr__
            finally:
                self.write(ret)

