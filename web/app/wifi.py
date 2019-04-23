from tornado.web import RequestHandler
from auth import auth
from config import Config
import os
import time
from tornado.log import app_log as LOG

class WifiHandler(RequestHandler):
    @auth
    def get(self):
        wifi = Config("wifi")
        with open(wifi.hostap_conf, "r") as conf_f:
            conf = conf_f.readlines()
            for text in conf:
                
        self.render("wifi_setup.html", wifi=wifi)
    
    def post(self, method):
        ret = {}
        if method == 'update':
            wifi = Config("wifi")
            try:
                wifi.set_item('enable', self.get_argument('enable'))
                if wifi.enable == 'true':
                    # 修改用户名和密码
                    wifi.set_item('ssid', self.get_argument('ssid'))
                    wifi.set_item('lbl', self.get_argument('lbl'))
                    if wifi.lbl == '3' or wifi.lbl == '4':
                        wifi.set_item('passwd', self.get_argument('key'))
                    status = self.change_ssid_passwd(wifi.ssid, wifi.passwd)
                    if not status:
                        raise
                else:
                    status = self.close_wifi()
                ret['status'] = status
                wifi.save()
            except Exception as e:
                LOG.error(e.__str__())
                ret['status'] = 'failed'
                ret['err_msg'] = e.__repr__
            finally:
                self.write(ret)
    
    def check_wifi_ps_state(self, check_state):
        try_count = 10
        while try_count:
            with os.popen("ps | grep hostapd") as res:
                result = res.readlines()
            flag = False
            for text in result:
                if "grep hostapd" not in text:
                    flag = True         # 存在进程
                    break
            if flag != check_state:
                try_count = try_count - 1
                time.sleep(1)
                continue
            else:
                return True
        return False

    def change_ssid_passwd(self, ssid, passwd):
        LOG.info("change ssid & passwd")
        os.system("/etc/init.d/hostapd restart")
        ret = self.check_wifi_ps_state(True)
        if ret:
            LOG.info("change wifi ssid & passwd success")
            return 'success'
        else:
            LOG.error("change wifi ssid & passwd failed")
            return 'failed'

    def close_wifi(self):
        LOG.info("close wifi")
        os.system("/etc/init.d/hostapd stop")
        ret = self.check_wifi_ps_state(False)
        if ret:
            LOG.info("close wifi success")
            return 'success'
        else:
            LOG.error("close wifi failed")
            return 'failed'
