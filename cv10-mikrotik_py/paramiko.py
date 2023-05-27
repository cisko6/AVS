#!/usr/bin/env python3

from paramiko import SSHClient, AutoAddPolicy

IP = "158.193.154.100"
PORT = 1012 #mikrotik
meno = "admin"
heslo = "admin"

client = SSHClient()
client.set_missing_host_key_policy(AutoAddPolicy())
client.connect(IP,port=PORT,username=meno,password=heslo)


(vstup,vystup,chyba) = client.exec_command("/ip add print terse")

for riadok in vystup:
    tokeny = riadok.strip("\n").strip("\r").split(" ")
    vystup_riadok = dict()
    for token in tokeny:
        conf_parameter = token.split("=")
        if len (conf_parameter) == 2:
            kluc = conf_parameter[0]
            hodnota = conf_parameter[1]
            vystup_riadok[kluc] = hodnota
        print(vystup_riadok)

#(vstup,vystup,chyba) = client.exec_command("ip add add address=192.168.1.110/24 interface=ether2")
#(vstup,vystup,chyba) = client.exec_command("ip address/remove 1")
#for riadok in vystup:
#    print(riadok)

client.close()
