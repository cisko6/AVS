#!/usr/bin/env python3

import requests

IP = "158.193.154.100"
PORT = 2012 #mikrotik api port
meno = "admin"
heslo = "admin"

#get
r = requests.get("https://{}:{}/rest/ip/addr".format(IP,PORT), auth=(meno,heslo),verify=False)
if r.status_code == 200:
    print(r.json())

# for i in range(2,11):
#     print("\n")
#     #put - vytvorenie loopu
#     nazov = "lo"+ str(i)
#     data = {"name":nazov,"disabled":"no"}
#     r = requests.put("https://{}:{}/rest/int/bridge".format(IP,PORT), auth=(meno,heslo),verify=False, json=data)
#     if r.status_code == 201:
#         print(r.json())

#     print("\n")
#     #ip na loop
#     nazov = "lo"+ str(i)
#     ip_loop = "10.12.0.{}/32".format(i)
#     data = {"interface":nazov,"address": ip_loop}
#     r = requests.put("https://{}:{}/rest/ip/addr".format(IP,PORT), auth=(meno,heslo),verify=False, json=data)
#     if r.status_code == 201:
#         print(r.json())

#rip
data = {"name":"rip","disabled": "no"}
r = requests.put("https://{}:{}/rest/routing/rip/instance".format(IP,PORT), auth=(meno,heslo),verify=False, json=data)
if r.status_code == 201:
    print(r.json())

data = {"instance":"rip","disabled": "no","interfaces":"all"}
r = requests.put("https://{}:{}/rest/routing/rip/interface-template".format(IP,PORT), auth=(meno,heslo),verify=False, json=data)
if r.status_code == 201:
    print(r.json())

