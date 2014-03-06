#include "mount/global_io_limiter.h"

#include "common/cltoma_communication.h"
#include "common/matocl_communication.h"
#include "mount/mastercomm.h"

bool GlobalIoLimiter::init() {
	try {
		negotiator_ = std::thread(&GlobalIoLimiter::negotiator, this);
	} catch (...) {
		return false;
	}
	// XXX: hack hack hack
	const std::string unclassified = "unclassified";
	buckets_.emplace(std::piecewise_construct,
			std::forward_as_tuple(unclassified),
			std::forward_as_tuple());
	//buckets_.at(unclassified).reconfigure(1024, 256);
	return true;
}

void GlobalIoLimiter::terminate() {
	terminate_ = true;
}

void GlobalIoLimiter::waitForRead(const IoLimitGroupId& groupId, uint64_t bytes) {
	// TODO: locking against reconfigure
	TokenBucket& bucket = buckets_.at(groupId);
	if (!bucket.attempt(bytes)) {
		negotiateRequest(groupId, bytes);
		bucket.wait(bytes);
	}
}

void GlobalIoLimiter::negotiateRequest(const IoLimitGroupId& groupId, uint64_t bytes) {
	std::unique_lock<std::mutex> lock(requestMutex_);
	request_.bytes = bytes;
	request_.groupId = &groupId;
	requestCond_.notify_one();
	request_.ready.wait(lock);
}

void GlobalIoLimiter::negotiator() {
	std::unique_lock<std::mutex> lock(requestMutex_);
	while (!terminate_) {
		if (request_.bytes == 0) {
			requestCond_.wait_for(lock, std::chrono::milliseconds(1000));
		}
		std::vector<uint8_t> buffer;
		if (request_.bytes > 0) {
			cltoma::iolimit::serialize(buffer, *request_.groupId, true,
					buckets_.at(*request_.groupId).rate(), 0);
		} else {
			// TODO: all groups
			cltoma::iolimit::serialize(buffer, "unclassified", false,
					buckets_.at("unclassified").rate(), 0);
		}
		request_.bytes = 0;
		uint32_t recv_cmd, ans_leng;
		const uint8_t *ans = fs_sendandreceive_dupa(std::move(buffer), recv_cmd, ans_leng);
		std::string ignore;
		uint64_t rate;
		matocl::iolimit::deserialize(ans, ans_leng, ignore, rate);
		buckets_.at("unclassified").reconfigure(rate, rate/4);
		request_.ready.notify_one();
	}
}
