# Code Execution Client Server
A small remote code execution server written in C using sockets and Unix System APIs, demonstrating advanced C programming as well as proficiency in distributed systems.


To run:
1. Navigate to the respective `/Client` and `/Server` directories in separate terminals and run `make` to build.
2. Run `./server` in the `/Server` directory to start the remote server.
3. Run `./client <ip address>` to start a client (replacing with the server ip address, or 127.0.0.1 if on same machine).



## Client Executable Commands
|Command|Args| Action|
|---|---|---|
|`put`| `<program> [sourcefiles]`|Send a program to the server, called `<program>`, with the `[sourcefiles]`.|
|`put`| `<program> [sourcefiles] -f`| Same as `put`, but overwrite if exists.|
|`get`| `<program> <sourcefile>`|To retrieve a sourcefile from `<program>` and read it on the client 40 lines at a time.|
|`run`| `<program>`|Compile (if required) and run `<program>` on the server and return output to client.|
|`run`| `<program> -f localfile`|Same as `run` command, but save output to `<localfile>`.|
|`list`| nil |List programs on server.|
|`list`| `program`|List files in `<program>`.|
|`list`| `program -l`|List files in `<program>`in long list format.|
|`sys`| nil |Retrieve information about server Operating System and CPU type.|
|`exit`| nil|To end the connection between client and server and close the current client.|
