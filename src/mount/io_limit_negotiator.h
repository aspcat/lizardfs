#pragma once

#include <mutex>
#include <vector>

#include "common/exception.h"

class IoLimitNegotiator {
public:
	LIZARDFS_CREATE_EXCEPTION_CLASS(NothingToDoException, Exception);

	IoLimitNegotiator(std::mutex& fdMutex, pthread_cond_t* ioCond);
	~IoLimitNegotiator();

	void start(double refreshTime);
	void stop();

	

private:
	pthread_t threadId_;
	std::mutex& fdMutex_;
	const pthread_cond_t* ioLimitsCond_;
	bool working_;

	void worker();
};
