import json

def r_get():
    data = {'enabled' : True}
    return json.dumps(data)

def r_set(args):
    return True;