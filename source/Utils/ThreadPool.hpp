#pragma once
#include "Core/Nyxispch.hpp"

namespace Nyxis
{
	class ThreadPool
	{
	public:
		explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency())
		: pStop(false)
		{
			{
				for (size_t i = 0; i < numThreads; ++i)
					{
						pThreads.emplace_back([this] {
						  while (true)
							  {
								  std::function<void()> task;
								  {
									  std::unique_lock<std::mutex> lock(pQueueMutex);
									  pCondition.wait(lock, [this] { return pStop || !pTasks.empty(); });
									  if (pStop && pTasks.empty())
										  return;
									  task = std::move(pTasks.front());
									  pTasks.pop();
								  }
								  task();
							  }
						});
					}
			}

		}

		~ThreadPool()
		{
			{
				{
					std::unique_lock<std::mutex> lock(pQueueMutex);
					pStop = true;
				}
				pCondition.notify_all();
				for (auto& thread : pThreads)
					thread.join();
			}

		}

		template <class F>
		void enqueue(F&& task)
		{
			{
				std::unique_lock<std::mutex> lock(pQueueMutex);
				pTasks.emplace(std::forward<F>(task));
			}
			pCondition.notify_one();
		}

		void wait()
		{
			for (auto& thread : pThreads)
				thread.join();
		}

	private:
		std::vector<std::thread> pThreads;
		std::queue<std::function<void()>> pTasks;
		std::mutex pQueueMutex;
		std::condition_variable pCondition;
		bool pStop;
	};
} // namespace Nyxis