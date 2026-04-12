# TCP CLI Chat App (C)

A simple real-time TCP chat application written in C using sockets and multithreading.

## Features

- TCP-based client-server communication
- Real-time two-way messaging
- Multithreading (send & receive simultaneously)
- Clean CLI interface (no messy overlapping text)
- Optional logging to file
- Timestamped messages
- Built-in commands system

---

##  Build

```bash
clang -o client-a client-a.c -lpthread
clang -o client-b client-b.c -lpthread
```

## Usage
```
./client-a [option]
./client-b [option]
```
## Help
```
./client-a -h
./client-b -h
```
## Notes

- Built for learning low-level networking and multithreading in C
- Communication is plaintext (no encryption yet)
- Optimized for Linux environments
- Uses TCP, following a client-server communication model
- `client-b` acts as the server (bind/listen)
- `client-a` acts as the client (connect)

## Example usage

```
┌─[woahuh@parrot]─[~/Documents/c-project/tcp-chat-app]
└──╼ $./client-a
[System]:Please enter Client B ip: 127.0.0.1
[System]:Please enter Client B port: 2345

Debug log:

[System]:Client B port: 2345
[System]:Client B ip: 127.0.0.1
[System]:Creating socket...
[System]:Setting up address...
[System]:Valid ip address. Continue...
[System]:Connecting to client B...
[System]:Set timeout to 15 seconds
[System]:Successfully connected to Client B
[System]:Starting threads...
[System]:Started session


[18:56:46][Client A]: Hello
[18:56:48][Client B]: Hi
[18:56:59][Client A]: Very great chat app
[Input][Client A]: /help

Commands:
/help        : Show this help message
/disconnect  : Close the connection
/logon       : Enable message logging (requires -w option)
/logoff      : Disable message logging (requires -w option)
[18:57:07][Client B]: Yea
[Input][Client A]: /disconnect

[System]:Connection closed
[System]:Cleaning up...
[System]:Exiting...
```
