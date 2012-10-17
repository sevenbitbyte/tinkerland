#include "PThread.h"
#include <pthread.h>
#include <iostream>

using namespace std;

void *runThread(void *pt){
    PThread* thread=(PThread*)pt;
    thread->setState(PThread::RUNNING);
    thread->run();
    thread->setState(PThread::COMPLETE);
}

PThread::PThread(): ptState(STOPPED){
    pthread_mutex_init(&mutexState, NULL);  //Initialize mutex
}

PThread::~PThread(){
    if(isRunning()){
        setState(STOPPING); //Update state
        int cancelState=pthread_cancel(ptid);
        if(cancelState!=0){ //Error encountered
            cout<<"Error cancelling thread, error "<<cancelState<<endl;
        }
        else{   //Thread is now stopped
            setState(STOPPED);
        }
    }
    pthread_mutex_destroy(&mutexState); //Destroy mutex
    printf("Stopped\n");
}

void PThread::start(int priority){
    setState(STARTING);
    int returnCode=pthread_create(&ptid, NULL, runThread, (void*)this);
    if(returnCode!=0){
        setState(STOPPED);
    }
}

void PThread::setState(ThreadState state){
    int error=pthread_mutex_unlock(&mutexState);    //obtain lock
    if(error){
        cout<<"setState: Error[code: "<<error<<"] obtianing state mutex."<<endl;
        return;
    }
    ptState=state;  //Safe to set state
    error=pthread_mutex_lock(&mutexState);
    if(error){
        cout<<"setState: Error[code: "<<error<<"] locking state mutex."<<endl;
        return;
    }
}

PThread::ThreadState PThread::getState(){
    int error=pthread_mutex_unlock(&mutexState);    //obtain lock
    if(error){
        cout<<"getState: Error[code: "<<error<<"] obtianing state mutex."<<endl;
        return RUNNING;
    }
    ThreadState state=ptState;  //Safe to get state
    error=pthread_mutex_lock(&mutexState);
    if(error){
        cout<<"getState: Error[code: "<<error<<"] locking state mutex."<<endl;
        return state;
    }
    return state;
}

bool PThread::isRunning(){
    switch(getState()){
        case STARTING:
            return true;
        case RUNNING:
            return true;
        case STOPPING:
            return true;
        default:
             break;
    }
    return false;
}
