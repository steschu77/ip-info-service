# simple-http

Minimal HTTP server in C/C++.

Accepts and serves one connection at a time.

Listens on port 8000.

## How to Use

```
> g++ -o httpd httpd.cpp
> ./httpd
Server started http://127.0.0.1:8000

> curl http://127.0.0.1:8000/
Hello!
> 
```

## Links

Refactored from <https://gist.github.com/laobubu/d6d0e9beb934b60b2e552c2d03e1409e>.

Based on <http://blog.abhijeetr.com/2010/04/very-simple-http-server-writen-in-c.html>.
