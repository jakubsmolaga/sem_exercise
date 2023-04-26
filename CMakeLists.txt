cmake_minimum_required(VERSION 3.0.0)
project(sem_exercise)

add_executable(sem_exercise src/main.c src/fifo.c)
target_include_directories(sem_exercise PRIVATE include)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(sem_exercise PRIVATE Threads::Threads)