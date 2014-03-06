#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "common/token_bucket.h"
#include "mount/io_limit_group.h"

class GlobalIoLimiter {
public:
	GlobalIoLimiter() : request_(), terminate_() {}

	bool init();
	void terminate();

	void waitForRead(const IoLimitGroupId& groupId, uint64_t bytes);
	void waitForWrite(const IoLimitGroupId& groupId, uint64_t bytes);
private:
	struct Request {
		uint64_t bytes;
		uint64_t rate;
		const IoLimitGroupId* groupId;
		std::condition_variable ready;
	};

	typedef std::unordered_map<IoLimitGroupId, TokenBucket> Buckets;

	void negotiateRequest(const IoLimitGroupId& groupId, uint64_t bytes);
	void reconfigure(const std::string& subsystem, const std::vector<IoLimitGroupId>& groups);
	void negotiator();

	Buckets buckets_;
	//std::mutex mutex_;
	std::thread negotiator_;
	Request request_;
	std::condition_variable requestCond_;
	std::mutex requestMutex_;
	std::atomic<bool> terminate_;
};
