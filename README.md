# HW2 Simple Key-value Store

## Directories
- /server ->	server program related sources
- /client ->	client program related sources
- /common ->	common inclusions
- /util ->	common utilization
- /build ->	target build directory

## Building the Project
Code out your `/server/server.c` and `/client/client.c`, then
```shell
$ make
```
Test your `/build/server` and `build/client`.

## Implementations
### Please briefly describe your multi-threading design pattern
- The multi-threading design pattern I used is "Thread-per-Message Model"
- Each time when the socket server get a request from different client, it will create a new thread which is detached
Then, this thread will serve the client until it send message "EXIT"
After receive message "EXIT", the thread will exit
- Then, the process also have another thread to receive message from server terminal. It will close socket and exit the process if it receive message "EXIT"
The thread wouldn't react if it get other message 

### Please briefly describe your data structure implementation
- When thread SET a (key, value) pair, create a "[KEY].txt" file. Inside the file is its value
- When thread GET a key, I just check if I have file "[KEY].txt"
- When thread DELETE a key, I remove the file "[KEY].txt"
- race condition handle:
  - I use read/write lock in pthread to handle race condition 
  - When thread is doing "SET" and "DELETE", we should enter critical section with wrlock mode
    - Other thread couldn't enter the section
  - When thread is doing "GET", we should enter critical section with rdlock mode
    - Other read thread could enter the section, but write thread couldn't

## References
* [POSIX thread man pages](https://man7.org/linux/man-pages/man7/pthreads.7.html)
* [socket man pages](https://linux.die.net/man/7/socket)

