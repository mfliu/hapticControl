
Just some notes for now.
Build rpclib from the `external/rpclib` folder with:
```
mkdir build && cd build && cmake ... && make 
```

RPC Server is built into MessageHandler, both the C++ module and the Python module runs a RPC
client. Same `rpclib` for the server and client in C++, [`msgpack-rpc`](https://github.com/msgpack-rpc/msgpack-rpc-python) for Python client 

Heavily based on and inspired by: [this project](https://github.com/djoshea/haptic-control)
