# Exam_Rank_06

The goal of the exam is to write a program that will listen for clients to connect to a specific port on localhost (127.0.0.1) and allow clients to communicate with each other.

Compile and run the program for First terminal (server)
```
clang -Wall -Wextra -Werror mini_serv.c -o mini_serv && ./mini_serv 8080
```
Opens two additional terminals for Second terminal (first client) and Third terminal (second client)
```
nc localhost 8080
nc localhost 8080
```
<img width="1070" alt="image" src="https://user-images.githubusercontent.com/58044383/204136683-34119db9-c4ec-446e-b1b8-73ac2ea7c2d5.png">

Sometimes the test sends a string of 87kb and the decision is not made, then in line 11 (mini_serv.c) we change the value from 1024 to 100000

<img width="300" alt="image" src="https://user-images.githubusercontent.com/58044383/206531101-9dcb9dc3-c4b5-4efb-a4ee-7b6b0c4f3544.png">
