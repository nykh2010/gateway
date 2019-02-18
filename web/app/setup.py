from tornado.web import RequestHandler
from auth import auth


class SetupHandler(RequestHandler):
    # def set_default_headers(self):
    #     self.set_header("Access-Control-Allow-Origin", "*")
    #     self.set_header("Access-Control-Allow-Headers", "x-requested-with")
    #     self.set_header("Access-Control-Allow-Methods", 'POST,GET,PUT,DELETE,OPTIONS')
    #     self.set_header("Access-Control-Allow_Credentials", "true")
    @auth
    def get(self, *args, **kwargs):
        self.render('setup.html')
        # if method == 'wifi':
        #     self.render('wifi_setup.html')
        # elif method == 'server':
        #     self.render('server_setup.html')
        # elif method == 'restart':
        #     self.render('restart.html')
        # elif method == 'changepwd':
        #     self.render('change_passwd.html')
        # elif method == 'gatewaysetup':
        #     self.render('gateway_setup.html')
        # elif method == 'upgrade':
        #     self.render('upgrade.html')
        # elif method == 'settime':
        #     self.render('settime.html')
        # elif method == 'restore':
        #     self.render('restore.html')
        # elif method == 'radiosetup':
        #     self.render('radiosetup.html')
        # elif method == 'log':
        #     self.render('log.html')
        # elif method == 'info':
        #     self.render('info.html')
    
    def post(self, method):
        if method == 'wifi':
            pass
        elif method == 'server':
            pass
        elif method == 'restart':
            pass
        self.write("['SUCCESS']")
    
    def options(self, method):
        pass
