#!/usr/bin/python
from sseclient import SSEClient
import requests
import json
from time import sleep
SERVER_URI = 'http://[2001:67c:254:b0b0::6]:8080'
API_URI = SERVER_URI + '/api'
CLIENTS_URI = API_URI + '/clients'
EVENT_URI = SERVER_URI + '/event'

HOST_ID = 'client_02'
OBSERVER_ID = 'client_01'
RESOURCE = '3311/0/5706' # Light Control, instance 0, 'Colour' resource

def observe_resource(endpoint, resource):
    r = requests.post(CLIENTS_URI + '/' + endpoint + '/' + resource + '/observe', params={'format': 'TLV'})

def write_resource(endpoint, resource, value):
    r = requests.put(CLIENTS_URI + '/' + endpoint + '/' + resource, json=value, params={'format': 'TLV'})

def main():
    # subscribe to all messages
    print("Observing server messages")
    messages = SSEClient(EVENT_URI)

    observer_connected = False
    host_connected = False

    for m in messages:
        if m.event == 'REGISTRATION':
            data = json.loads(m.data)
            ep = data.get('endpoint', None)
            if ep == HOST_ID:
                print("Host registered")
                host_connected = True
            elif ep == OBSERVER_ID:
                print("Observer registered")
                observer_connected = True

        if host_connected and observer_connected:
            break

    sleep(1)

    # observe interest resource in host
    print(f'Observing resource {RESOURCE} in {HOST_ID}')
    observe_resource(HOST_ID, RESOURCE)

    # subscribe to host messages
    print(f'Observing server messages for {HOST_ID}')
    messages = SSEClient(EVENT_URI, params={'ep':HOST_ID})

    for m in messages:
        if m.event == 'NOTIFICATION':
            data = json.loads(m.data)
            value = data.get('val', None)
            if value is not None:
                print(f'Got a new value {value}, writing to requester')
                write_resource(OBSERVER_ID, RESOURCE, value)


if __name__ == "__main__":
    main()