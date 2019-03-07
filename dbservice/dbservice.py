from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from socketserver import UnixStreamServer, StreamRequestHandler
from usertable import Base, WhiteList
import json
import os

# CONFIG_PATH = os.path.join(os.path.dirname(__file__), '../config.json')

# DBPORT = 9000
# DBIP = 'localhost'

Session = None

class ParamError(Exception):
    '''参数错误'''
    def __init__(self, code=400, msg="参数错误"):
        self.code = code
        self.msg = msg
        super().__init__(self, code, msg)

class WhiteListHandler:
    def func(self, data):
        session = Session()
        if data['method'] == 'check_id':
            device_id = data['device_id']
            print(device_id)
            result = session.query(WhiteList).filter_by(device_id=device_id).all()
            if len(result):
                return 'success'
            else:
                return 'failed'
        elif data['method'] == 'add_list':
            add_list = []
            for device in data['device_list']:
                add_list.append(WhiteList(device_id=device))
            session.add_all(add_list)
            session.commit()

class TaskHandler:
    pass

class ExecuteHandler:
    pass

class SqlHandler:
    def func(self, data):
        ret = []
        result = {}
        session = Session()
        results = session.execute(data['sql_cmd']).fetchall()
        for r in results:
            for k,v in zip(r.keys(), r):
                result[k] = v
            ret.append(result)
        return ret

class DBServer(StreamRequestHandler):
    '''数据库服务接口'''
    __tables = [
        ('sql', SqlHandler),
        ('white_list', WhiteListHandler),
        ('task', TaskHandler),
        ('execute', ExecuteHandler)
    ]
    status_code = 200
    status_msg = ""
    status_result = None
    dbsession = None
    wbufsize = 1024
    rbufsize = 1024
    # timeout = 0.5
    def route(self, data):
        for t in self.__tables:
            if t[0] == data['table_name']:
                return t[1]
        return None

    def protocol(self, content):
        '''协议处理'''
        data = json.loads(content.decode('utf-8'))
        TableClass = self.route(data)
        if TableClass is None:
            raise ParamError()
        tableHandle = TableClass()
        result = tableHandle.func(data)
        return result

    def handle(self):
        try:
            # content = self.request.recv(0)
            content = self.rfile.read1(1024)
            # print(help(self.rfile))
            print(content)
            self.status_result = self.protocol(content)
            self.status_code = 200
        except Exception as e:
            print(e)
            self.status_code = 400
            self.status_msg = e.__repr__()
    
    def finish(self):
        r_dict = {
            'status':self.status_code,
            'msg':self.status_msg,
            'result':self.status_result
        }
        r = json.dumps(r_dict)
        self.wfile.write(r.encode('utf-8'))
        super().finish()

if __name__ == "__main__":
    print('%s start...' % __file__)
    path = r'/home/xulingfeng/sqlite3.sock'
    # try:
    #     with open(CONFIG_PATH, 'r') as f:
    #         config = f.read(1024)
    #         c_dict = json.loads(config)
    #         DBPORT = c_dict['dbconfig']['DBPORT']
    #         DBIP = c_dict['dbconfig']['DBIP']
    # except:
    #     DBIP = 'localhost'
    #     DBPORT = 9000
    
    if os.path.exists(path):
        os.unlink(path)
    dbserver = UnixStreamServer(path, DBServer)
        
    try:
        engine = create_engine(r'sqlite:///gateway.db?check_same_thread=False', echo=True)
        Base.metadata.create_all(engine)
        Session = sessionmaker(bind=engine)
    except Exception as e:
        print(e)

    dbserver.serve_forever()