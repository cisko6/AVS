#!/usr/bin/env python3

from netmiko import ConnectHandler

IP = "158.193.154.100"
PORT = 1013 #cysco
meno = "admin"
heslo = "admin"

cisco = {
    "device_type":"cisco_ios",
    "host":IP,
    "username":meno,
    "password":heslo,
    "port":PORT
}

client = ConnectHandler(**cisco)
vystup = client.send_command("sh ip int brief")
print(vystup+"\n")

config = {"int lo0","ip add 10.10.10.10 255.255.255.255","no sh"}
client.send_config_set(config)

vystup = client.send_command("sh ip int brief")
print(vystup)

