#include "mount/io_limit_negotiator.h"

#include <unistd.h>

IoLimitNegotiator::IoLimitNegotiator(std::mutex& fdMutex, pthread_cond_t* ioCond) :
		threadId_(0),
		fdMutex_(fdMutex),
		ioLimitsCond_(ioCond),
		working_(false) {
}

void IoLimitNegotiator::start(double refreshTime) {
	if (refreshTime <= 0.0) {
		throw NothingToDoException("Incorrect refresh time");
	}

	std::unique_lock lock(fdMutex_);
	working_ = true;
}

void IoLimitNegotiator::stop() {
}

void IoLimitNegotiator::worker(void* arg) {
	double renegotiationTime = *static_cast<double*>(arg);
	while (working_) {
		int result = pthread_cond_timedwait(ioLimitsCond_, )


	}
}
