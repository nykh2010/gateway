#!/bin/bash

echo "解压"
tar zxvf gateway.tar.gz -C /tmp/gateway
echo "安装"
echo "配置文件"
cd /tmp/gateway
mkdir -p /etc/gateway
cp config /etc/gateway -rf
echo "数据库服务"
if [ ! -x dbservice ]; then
    chmod +x dbservice
fi
cp dbservice /home/root -rf
echo "web服务"
if [ ! -x app/webserver.py ]; then
    chmod +x app/webserver.py
fi
cp web /home/root -rf
echo "epd应用服务"
if [ ! -x epd_service/main.py ]; then
    chmod +x epd_service/main.py
fi
cp epd_service /home/root -rf
echo "日志服务"
mkdir -p /var/log/gateway

echo "服务启动"
cd /home/root
echo "数据库启动"
gateway/dbservice &
echo "web服务"
gateway/web/app/webserver.py
echo "epd应用服务"
gateway/epd_service/main.py


