from sqlalchemy import Column, Integer, String, ForeignKey
from sqlalchemy.ext.declarative import declarative_base

Base = declarative_base()

class WhiteList(Base):
    '''白名单'''
    __tablename__ = 'white_list'

    device_id = Column(String, primary_key=True)      # 终端id
    last_connect_time = Column(String)                  # 最近一次连接时间
    status = Column(Integer, default=0)             # 默认状态
    msg = Column(String)                            # 消息


class Task(Base):
    '''任务表'''
    __tablename__ = 'task'
    task_id = Column(String, primary_key=True)  # 任务id
    time = Column(Integer)                      # 任务执行时段
    status = Column(String, default='Sleep')    # 任务状态

class Excute(Base):
    '''任务执行表'''
    __tablename__ = 'execute'

    device_id = Column(String, primary_key=True)    # 终端id
    task_id = Column(String,ForeignKey('task.task_id'))      # 任务id
    status = Column(Integer, default=2)             # 执行状态