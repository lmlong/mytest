#! /bin/bash

R_IP=58.221.60.45
R_PORT=56000

R_L_PORT=57000
L_PORT=22
L_IP=192.168.0.100

ssh -CfnNT -R *:$R_L_PORT:$L_IP:$L_PORT -p $R_PORT -o ServerAliveInterval=60 root@$R_IP 
