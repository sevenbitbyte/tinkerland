#ifndef TIMED_RECIEVE_H
#define TIMED_RECIEVE_H

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>

//This function allows for easy timeout detection.
int timedRecieve(int seconds, int usecs, int file_descriptor){
        fd_set user_input;
        struct timeval timer;
        FD_ZERO(&user_input);
        FD_SET(file_descriptor, &user_input);
        timer.tv_sec = seconds;
        timer.tv_usec = usecs;  
        return select(file_descriptor+1, &user_input, NULL, NULL, &timer);
}

#endif
