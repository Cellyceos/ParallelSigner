#pragma once

#include "types.hpp"
#include "Queue.hpp"

#include <chrono>
#include <future>
#include <filesystem>
#include <condition_variable>

namespace Signature
{
	class MainWorker
	{
	public:
		MainWorker( const std::filesystem::path& inFilePath, const std::filesystem::path& outFilePath, size_t blockSize );
		~MainWorker();

		int execute();

	private:
        const std::filesystem::path inFilePath_;
        const std::filesystem::path outFilePath_;

        const size_t blockSize_ = 0;
		
		size_t maxThreadPool_ = 0;
		size_t maxPoolDataZize_ = 0;
        
		std::vector<std::future<void>> threadPool_;
        std::unique_ptr<Concurency::FastCircularQueue<chunk_data_ptr_t>> jobDataPool_ = nullptr;
		std::unique_ptr<Concurency::FastCircularQueue<chunk_data_ptr_t>> freeChunkPool_ = nullptr;

		std::unique_ptr<Concurency::FastCircularQueue<result_data_ptr_t>> writerPool_ = nullptr;
        std::unique_ptr<Concurency::FastCircularQueue<result_data_ptr_t>> freeResultPool_ = nullptr;

		static constexpr uint8_t defaultThreadCount = 4;
		static constexpr std::chrono::milliseconds threadTimeout{ 100 };

		std::condition_variable writerPoolNotEmpty_;
		std::condition_variable jobPoolNotEmpty_;
		std::condition_variable jobPoolNotFull_;

		std::mutex writerMutex_;
		std::mutex chunkMutex_;
		std::mutex jobMutex_;

		std::atomic_bool prepareToExit_ = false;
		std::atomic_bool somethingGoesWrong_ = false;

		void hashWorker();
		void writeWorker();
		void waitThreads();
	};
} // namespace Signature
