from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from socketserver import UnixStreamServer, StreamRequestHandler
from usertable import Base, WhiteList, Execute, Task
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
        elif data['method'] == 'connect':
            device_id = data['device_id']
            res = session.query(WhiteList).filter_by(device_id=device_id).first()
            if res:
                res.last_connect_time = data['time']
                session.add(res)
                session.commit()
        elif data['method'] == 'create':
            # 以覆盖方式添加
            res = session.query(WhiteList).all()
            for record in res:
                session.delete(record)
            for device in data['device_list']:
                new_device = WhiteList(device_id=device)
                session.add(new_device)
            session.commit()  

class TaskHandler:
    def func(self, data):
        method = data['method']
        if method == 'check_id':
            session = Session()
            result = session.query(Task).filter_by(task_id=data['task_id']).all()
            ret = []
            for r in result:
                d = r.__dict__
                d.pop('_sa_instance_state')
                ret.append(d)
            print(ret)
            return ret
        elif method == 'create':
            session = Session()
            res = session.query(Task).all()
            for record in res:
                session.delete(record)
            record = Task(task_id=data['task_id'],data_id=data['data_id'],data_url=data['data_url'],status=1)
            session.add(record)
            session.commit()
        elif method == 'cancel':
            session = Session()
            res = session.query(Task).filter_by(task_id=data['task_id']).first()
            res.status = 5
            session.add(res)
            session.commit()
        elif method == 'result':
            session = Session()
            res = session.query(Task).filter_by(task_id=data['task_id']).first()
            res.status = data['status']
            session.add(res)
            session.commit()
            

class ExecuteHandler:
    def func(self, data):
        session = Session()
        method = data['method']
        if method == 'create':
            task_id = data['task_id']
            res = session.query(Execute).all()
            for record in res:
                session.delete(record)
            for device_id in data['device_list']:
                record = Execute(device_id=device_id, task_id=task_id, status=0)
                session.add(record)
            session.commit()
            return 'success'
        elif method == 'result':
            fail_list = data.get('fail_list', [])
            success_list = data.get('success_list',[])
            task_id = data['task_id']
            for device in success_list:
                res = session.query(Execute).filter_by(task_id=task_id, device_id=device_id).first()
                res.status = 2
                session.add(res)
            for device in fail_list:
                res = session.query(Execute).filter_by(task_id=task_id, device_id=device_id).first()
                res.status = 3
                session.add(res)
            session.commit()
            return 'success'

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
            content = self.request.recv(1024)
            # content = self.rfile.read1(1024)
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
    path = r'/home/nykh2010/sqlite3.sock'
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