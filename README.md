# ip-info-service

Minimal HTTP/REST server in C/C++ which provides information about requester's IP address.

Accepts and serves one connection at a time.

Listens on port 8000.

Services include:

* Country of IP
* Timezone of IP
* UTC Time in milliseconds

## How to Use

```
> g++ -o httpd httpd.cpp
> ./httpd
Server started http://127.0.0.1:8000

> curl http://127.0.0.1:8000/time
123456789
> 
```

## Links

Based on <https://github.com/steschu77/simple-http>.
