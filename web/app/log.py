from tornado.web import RequestHandler

class LogHandler(RequestHandler):
    def get(self):
        self.render('log.html')