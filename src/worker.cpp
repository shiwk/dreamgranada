#include "worker.hpp"
#include <glog/logging.h>

namespace granada
{
    Worker::Worker()
    {
    }

    Worker::~Worker()
    {
    }

    void Worker::handle(ioloop::EventPtr event)
    {
        DLOG(INFO) << "hello, event [" << event->type() << "]";
    }
}