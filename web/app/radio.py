from tornado.web import RequestHandler
from auth import auth
from config import Config
from connect_service import send_to_service

class RadioHandler(RequestHandler):
    '''射频参数配置'''
    @auth
    def get(self):
        radio1 = Config("radio1")
        radio2 = Config("radio2")
        if radio1.enable:
            radio_1_param = self.render_string("radio1_param.html", radio=radio1)
        else:
            radio_1_param = '''
                <p>射频模块1无效</p>
            '''
        if radio2.enable:
            radio_2_param = self.render_string("radio2_param.html", radio=radio2)
        else:
            radio_2_param = '''
                <p>射频模块2无效</p>
            '''
        self.render("radiosetup.html", radio_1=radio_1_param, radio_2=radio_2_param)

    def post(self, cmd):
        ret = {}
        try:
            if cmd == "update":
                radio_number = self.get_argument("radio_number")
                if (radio_number == '1'):
                    # print("radio1")
                    radio = Config("radio1")
                    mode = self.get_argument("lora_mode")
                    preamble = self.get_argument("preamble")
                    spread = self.get_argument("spread")
                    base_band = self.get_argument("base_band")
                    program_radio = self.get_argument("program_ratio")
                    frequency = self.get_argument("frequency")
                    crc_enable = self.get_argument("crc_enable_value")
                    power = self.get_argument("power")
                    sync = self.get_argument("sync")
                    radio.set_item("mode", mode)
                    radio.set_item("preamble",preamble)
                    radio.set_item("sf", spread)
                    radio.set_item("bw", base_band)
                    radio.set_item("cr", program_radio)
                    radio.set_item("frequency", frequency)
                    radio.set_item("crc", crc_enable)
                    radio.set_item("power", power)
                    radio.set_item("sync", sync)
                    radio.save()
                    data = {
                        "radio_number":"1"
                    }
                    send_to_service('epd_service', '/radio/update', data=data)
                else:
                    radio = Config("radio2")
                    data = {
                        "radio_number":"2"
                    }
                    send_to_service('epd_service', '/radio/update', data=data)
                ret['status'] = 'success'
            elif cmd == "restart":
                radio_number = self.get_argument("radio_number")
                data = {
                    "radio_number":radio_number
                }
                send_to_service('epd_service', 'radio/restart', data=data)
        except Exception as e:
            ret['status'] = 'failed'
            ret['err_msg'] = e.__repr__()
            print(e)
        self.write(ret)