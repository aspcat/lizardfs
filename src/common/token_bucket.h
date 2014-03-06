#pragma once

#include <unistd.h>

#include <mutex>

#include <common/time_utils.h>

class TokenBucket {
public:
	TokenBucket(double rate, double budgetCeil) :
		rate_(rate),
		budgetCeil_(budgetCeil),
		budget_(0)
	{}
	TokenBucket() : TokenBucket(0, 0) {}
	TokenBucket(const TokenBucket&) = delete;

	void reconfigure(double rate, double budgetCeil) {
		std::unique_lock<std::mutex> lock(mutex_);
		rate_ = rate;
		budgetCeil_ = budgetCeil;
	}

	double rate() {
		std::unique_lock<std::mutex> lock(mutex_);
		return rate_;
	}

	void wait(double cost) {
		std::unique_lock<std::mutex> lock(mutex_);
		autoUpBudget();
		downBudget(cost);
		const double budget = budget_;
		if (budget < 0) {
			const double rate = rate_;
			lock.unlock();
			const double time = (-budget) / rate;
			const double usecs = time * 1000000.0;
			usleep(usecs);
		}
	}

	bool attempt(double cost) {
		std::unique_lock<std::mutex> lock(mutex_);
		autoUpBudget();
		if (budget_ >= cost) {
			downBudget(cost);
			return true;
		} else {
			return false;
		}
	}
private:
	void autoUpBudget() {
		int64_t time_ns = timer_.lap_ns();
		budget_ += rate_ * time_ns / 1000000000.0;
		if (budget_ > budgetCeil_) {
			budget_ = budgetCeil_;
		}
	}
	void downBudget(double cost) {
		budget_ -= cost;
	}

	double rate_;
	double budgetCeil_;
	double budget_;
	Timer timer_;
	std::mutex mutex_;
};
