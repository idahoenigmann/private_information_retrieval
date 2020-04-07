build/server --port 1234 --verbose &
build/server --port 1235 --verbose &
build/client --index 0 --server1 localhost --port1 1234 --server2 localhost --port2 1235