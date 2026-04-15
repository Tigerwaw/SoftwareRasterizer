#pragma once
#include <thread>
#include <vector>

class ThreadJoiner
{
public:
	explicit ThreadJoiner(std::vector<std::thread>& aThreads);
	~ThreadJoiner();
private:
	std::vector<std::thread>& myThreads;
};

inline ThreadJoiner::ThreadJoiner(std::vector<std::thread>& aThreads) : myThreads(aThreads)
{
}

inline ThreadJoiner::~ThreadJoiner()
{
	for (size_t i = 0; i < myThreads.size(); i++)
	{
		if (myThreads[i].joinable())
			myThreads[i].join();
	}
}