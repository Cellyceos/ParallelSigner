#pragma once

#include <atomic>
#include <thread>
#include <vector>
#include <cassert>

namespace Signature
{
	namespace Concurency
	{
		/**
		 * A Proof of Concept Thread Safe queue template class template based on a circular buffer
		 * (backed by an array) using atomic operation instead of a global mutex.
		 * In this implementation a single writer thread and multiple reader threads are supported.
		 * If the write index overruns the read index, packets from the front of the queue
		 * (the oldest) are purged to make room for new entries. The queue stores copies of T.
		 *
		 * See the formal spec (TLA+) for algorithm verification.
		 *
		 * @tparam T Type of object stored in the Queue - ideally use a smart pointer of some sort.
		*/
		template <class T>
		class FastCircularQueue final
		{
		public:
			/**
			 * One and only constructor for the PoC Queue.
			 * @param size - The size of the backing array for the queue
			*/
			explicit FastCircularQueue( size_t size );

			/**
			 * Note: This class is final so we don't need a vtable. If this class is changed to
			 * be inheritable then this needs to become virtual.
			*/
			~FastCircularQueue();

			/**
			 * Put an element onto the end of the queue - there can be only one thread doing
			 * this.
			 * @param element - An object of type T
			*/
			bool push( T&& element );

			/**
			 * Pops and element off the front of the queue - this is thread-safe and there can
			 * be multiple readers.
			*/
			T&& pop();

			/**
			 * Reports if there are any elements currently in the queue - ephemeral if
			 * writer and readers are active.
			*/
			bool isEmpty();

			/**
			 * Reports the number of elements in the queue by counting non-null entries.
			 * Only really valid if there are no active readers or writer.
			*/
			size_t count();

		private: //Defaults
			//No reason to copy so disable
			FastCircularQueue( const FastCircularQueue& ) = delete;
			FastCircularQueue& operator=( FastCircularQueue& ) = delete;

			static constexpr size_t readIdx_Lock = -1;
			static constexpr size_t writeIdx_Lock = -1;
			const size_t bufferSize_;

			std::atomic_size_t writeIdx_;
			std::atomic_size_t readIdx_;
			std::atomic_size_t count_;

			std::vector<T> buffer_;
		};

		/*
		 * Constructor
		*/
		template <class T>
		FastCircularQueue<T>::FastCircularQueue( size_t size ) :
			bufferSize_( size ), writeIdx_( 0 ),
			readIdx_( 0 ), count_( 0 ),
			buffer_( size )
		{
		}

		/*
		 * Destructor
		*/
		template <class T>
		FastCircularQueue<T>::~FastCircularQueue()
		{
			buffer_.clear();
		}

		/*
		 * Push an element onto the queue
		*/
		template <class T>
		bool FastCircularQueue<T>::push( T&& element )
		{
			assert( count_ < bufferSize_ );

			while ( true )
			{
				//Load the current read index
				size_t currentWriteIdx = writeIdx_;

				//If the current read index is not held by another thread - lock it.
				//This starts a critical section
				if ( ( currentWriteIdx != writeIdx_Lock ) &&
					 writeIdx_.compare_exchange_weak( currentWriteIdx,
													  writeIdx_Lock,
													  std::memory_order_release,
													  std::memory_order_relaxed ) )
				{
					//Moved element to queue;
					buffer_[currentWriteIdx] = std::move( element );

					//Increment the write index and wrap if necessary
					writeIdx_ = ( currentWriteIdx + 1 ) % bufferSize_;

					//Update count
					++count_;
					return true;
				}
				else
				{
					std::this_thread::yield(); //Yield to the next thread
				}
			}

			return false;
		}

		/*
		 * Pop and element off the queue
		*/
		template <class T>
		T&& FastCircularQueue<T>::pop()
		{
			while ( true )
			{
				//Load the current read index
				size_t currentReadIdx = readIdx_;

				//If the current read index is not held by another thread - lock it.
				//This starts a critical section
				if ( ( currentReadIdx != readIdx_Lock ) &&
					 readIdx_.compare_exchange_weak( currentReadIdx,
													 readIdx_Lock,
													 std::memory_order_release,
													 std::memory_order_relaxed ) )
				{
					// CS: Start of Critical Section for readers
					if ( count_ < 1 )
					{
						//If we're here we've caught up to the front of the queue. Reset the read
						//index and try again.
						readIdx_ = currentReadIdx;
						continue;
					}

					//Decrement the count - this must be atomic and come before
					--count_;

					//Calculate a new read index - does not have to be atomic
					readIdx_ = ( currentReadIdx + 1 ) % bufferSize_;

					//Fetch the value from the buffer
					return std::move( buffer_[currentReadIdx] );
				}
				else
				{
					std::this_thread::yield(); //Yield to the next thread
				}
			}

			return std::move( T() );
		}

		/*
		 * Return a true if the count is 0
		*/
		template <class T>
		bool FastCircularQueue<T>::isEmpty()
		{
			return count_ <= 0;
		}

		/*
		 * Return a count of the non-null element in the queue.s
		*/
		template <class T>
		size_t FastCircularQueue<T>::count()
		{
			return count_;
		}
	} // namespace Concurency
} // namespace Signature
