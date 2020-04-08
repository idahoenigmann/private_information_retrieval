build/server --port 1234 --file src/server/data.txt --verbose &
build/server --port 1235 --file src/server/data.txt --verbose &
build/client --index 0 --server1 localhost --port1 1234 --server2 localhost --port2 1235