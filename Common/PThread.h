#ifndef PTHREAD_H
#define PTHREAD_H
#include <pthread.h>

static void *runThread(void *pt);

class PThread{
    public:
        PThread();
        ~PThread();
        void start(int priority=NULL);
        bool isRunning();
        enum ThreadState{STOPPED, STARTING, RUNNING, STOPPING, COMPLETE};

    protected:
        virtual void run()=0;
    private:
        friend void *runThread(void *pt);
        void setState(ThreadState state);
        ThreadState getState();
        pthread_mutex_t mutexState;
        ThreadState ptState;
        pthread_t ptid; //Thread id
};

#endif
