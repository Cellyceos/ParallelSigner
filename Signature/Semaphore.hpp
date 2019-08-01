#pragma once

#include <mutex>
#include <condition_variable>

namespace Signature
{
	namespace Concurency
	{
		class Semaphore
		{
		private:
			std::mutex mutex_;
			std::condition_variable condition_;

			size_t freeSlots_;
			const size_t count_;

		public:
			Semaphore( size_t count ) :
				count_( count ), freeSlots_( count ) {}

			~Semaphore()
			{
				//notify all waiting threads
				condition_.notify_all();
			}

			void notify()
			{
				std::unique_lock<std::mutex> lock( mutex_ );

				if ( freeSlots_ < count_ )
				{
					++freeSlots_;

					//notify the waiting thread
					condition_.notify_one();
				}
			}

			void wait()
			{
				std::unique_lock<std::mutex> lock( mutex_ );

				while ( freeSlots_ == 0 )
				{
					//wait on the mutex until notify is called
					condition_.wait( lock );
				}

				--freeSlots_;
			}
		};
	} // namespace Concurency
} // namespace Signature
