cmake_minimum_required(VERSION 3.22)
project(myPthread C)

set(CMAKE_C_STANDARD 99)

#add_executable(myPthread main.c test.c)
#add_executable(mypthread main.c mypthread.c)
add_executable(mypthread mypthread_CAS.c mypthread.c)
