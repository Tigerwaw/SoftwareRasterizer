#pragma once
#include <mutex>
#include <queue>

template<typename T>
class ThreadsafeQueue
{
public:
	void Push(T aValue);
	void WaitAndPop(T& aValue);
	bool TryPop(T& aValue);
	bool IsEmpty() const;

private:
	mutable std::mutex myMutex;
	std::queue<T> myDataQueue;
	std::condition_variable myDataCondition;
};

template<typename T>
void ThreadsafeQueue<T>::Push(T aValue)
{
	std::lock_guard<std::mutex> lock(myMutex);
	myDataQueue.push(std::move(aValue));
	myDataCondition.notify_one();
}

template<typename T>
void ThreadsafeQueue<T>::WaitAndPop(T& aValue)
{
	std::unique_lock<std::mutex> lock(myMutex);
	myDataCondition.wait(lock, [this] { return !myDataQueue.empty(); });
	aValue = std::move(myDataQueue.front());
	myDataQueue.pop();
}

template<typename T>
bool ThreadsafeQueue<T>::TryPop(T& aValue)
{
	std::lock_guard<std::mutex> lock(myMutex);
	if (myDataQueue.empty())
		return false;

	aValue = std::move(myDataQueue.front());
	myDataQueue.pop();
	return true;
}

template<typename T>
bool ThreadsafeQueue<T>::IsEmpty() const
{
	std::lock_guard<std::mutex> lock(myMutex);
	return myDataQueue.empty();
}