#!/bin/bash

# g++ server.cpp -o server
g++ server.cpp hashtable.cpp avl.cpp test_avl.cpp -o server
g++ client.cpp -o client