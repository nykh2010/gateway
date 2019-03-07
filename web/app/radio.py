from tornado.web import RequestHandler
from auth import auth
from config import Config
from connect_service import send_to_service

class RadioHandler(RequestHandler):
    '''射频参数配置'''
    @auth
    def get(self):
        radio = Config("radio")
        self.render("radiosetup.html", radio=radio)

    def post(self):
        ret = {}
        radio = Config("radio")
        try:
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
            send_to_service('epd_service', '/radio/update')
            ret['status'] = 'success'
        except Exception as e:
            ret['status'] = 'failed'
            ret['err_msg'] = e.__repr__()
            print(e)
        self.write(ret)