#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>
#include <functional>
#include <type_traits>
#include <memory>
#include "ThreadsafeQueue.hpp"
#include "ThreadJoiner.hpp"

class TaskSystem
{
public:
	~TaskSystem();
	TaskSystem(TaskSystem&) = delete;
	void operator=(TaskSystem&) = delete;

	static TaskSystem& GetInstance()
	{
		static TaskSystem instance;
		return instance;
	}

	template<typename F, typename... Args>
	auto SubmitTask(F&& aTask, Args&&... aArgs)->std::future<decltype(aTask(aArgs...))>;

private:
	TaskSystem();
	std::atomic_bool myIsDone;
	ThreadsafeQueue<std::function<void()>> myTaskQueue;
	std::vector<std::thread> myThreads;
	ThreadJoiner myThreadJoiner;
	void WorkerThread();
};

inline TaskSystem::TaskSystem() : myIsDone(false), myThreadJoiner(myThreads)
{
	unsigned threadCount = std::thread::hardware_concurrency();
	try
	{
		for (unsigned i = 0; i < threadCount; i++)
		{
			myThreads.push_back(std::thread(&TaskSystem::WorkerThread, this));
		}
	}
	catch (...)
	{
		myIsDone = true;
		throw;
	}
}

inline TaskSystem::~TaskSystem()
{
	myIsDone = true;
}

template<typename F, typename... Args>
inline auto TaskSystem::SubmitTask(F&& aTask, Args&&... aArgs)->std::future<decltype(aTask(aArgs...))>
{
	auto function = std::bind(std::forward<F>(aTask), std::forward<Args>(aArgs)...);
	auto encapsulated = std::make_shared<std::packaged_task<decltype(aTask(aArgs...))()>>(function);

	auto futureObject = encapsulated->get_future();
	myTaskQueue.Push([encapsulated]()
	{
		(*encapsulated)();
	});
	return futureObject;
}

inline void TaskSystem::WorkerThread()
{
	while (!myIsDone)
	{
		std::function<void()> task;
		if (myTaskQueue.TryPop(task))
		{
			task();
		}
		else
		{
			std::this_thread::yield();
		}
	}
}