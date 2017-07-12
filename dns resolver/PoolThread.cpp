#include "PoolThread.h"

PoolThread::PoolThread(SafeQueue& _safe_queue) : safe_queue(_safe_queue) {}
void PoolThread::run()
{
    //getting task and running it
    while (Task* task = safe_queue.nextTask())
        task->run();
}
