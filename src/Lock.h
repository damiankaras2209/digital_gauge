#ifndef _LOCK_H
#define _LOCK_H

class Lock {
    private:
        volatile bool isBusy = false;
    public:
        void lock();
        void release();
};

#endif
