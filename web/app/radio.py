from tornado.web import RequestHandler
from auth import auth
from config import Radio

class RadioHandler(RequestHandler):
    '''射频参数配置'''
    @auth
    def get(self):
        radio = Radio()
        self.render("radiosetup.html", radio=radio)

    def post(self):
        ret = {}
        radio = Radio()
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
            radio.mode = mode
            radio.preamble = preamble
            radio.sf = spread
            radio.bw = base_band
            radio.cr = program_radio
            radio.frequency = frequency
            radio.crc = crc_enable
            radio.power = power
            radio.sync = sync
            radio.save()
            ret['status'] = 'success'
        except Exception as e:
            ret['status'] = 'failed'
            ret['err_msg'] = e.__repr__()
            print(e)
        self.write(ret)