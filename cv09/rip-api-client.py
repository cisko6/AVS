#!/usr/bin/env python3

import requests

IP = "127.0.0.1:8080"

def vytvor_route(ip,mask,next_hop,metric):
    route = {"ip": ip, "mask":mask,"next_hop":next_hop,"metric":metric}
    resp = requests.post("http://"+IP+"/routes", json = route)
    print(resp.status_code)

for i in range(0,10):
    vytvor_route("192.168."+str(i)+".0","255.255.255.0", "0.0.0.0",i)
    