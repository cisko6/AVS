#!/usr/bin/env python3

from fastapi import FastAPI
from pydantic import BaseModel
from typing import Union

class RIP_route():
    def __init__(self,ip,mask,next_hop,metric):
        self._ip = ip
        self._mask = mask
        self._next_hop = next_hop
        self._metric = metric

    def print(self):
        return {
            "ip": self._ip,
            "mask": self._mask,
            "next_hop": self._next_hop,
            "metric": self._metric
        }


class RIP():
    def __init__(self):
        self.route_count = 0
    def daj_id(self):
        #self.route_count += 1
        #return route_count

class Route(BaseModel):
    id: Union[int,None] = None
    ip:str
    mask:str
    next_hop:str
    metric:int


rip = list()
ripClass = RIP()

app = FastAPI()

@app.get("/")
def root():
    return {"msg": "Hello from API"}

@app.get("/routes")
def dajRouty():
    return rip


@app.post("/routes")
def create_route(route: Route):
    #route.id = ripClass.daj_id
    rip.append(route)
    return route
    



