#pragma once

#include "Semaphore.hpp"

#include <atomic>
#include <thread>
#include <vector>


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
			void push( const T& element );
            void push( T&& element );

			/**
			 * Pops and element off the front of the queue - this is thread-safe and there can
			 * be multiple readers.
			*/
			bool pop( T& result );

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
			const size_t bufferSize_;

			size_t writeIdx_;

			std::atomic_size_t readIdx_;
			std::atomic_size_t count_;

			std::vector<T> buffer_;
			Semaphore condition_;
		};

		/*
		 * Constructor
		*/
		template <class T>
		FastCircularQueue<T>::FastCircularQueue( size_t size ) :
			bufferSize_( size ), writeIdx_( 0 ),
			readIdx_( 0 ), count_( 0 ),
			buffer_( size ), condition_( size )
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
        void FastCircularQueue<T>::push( const T& element )
        {
            push( std::move( element ) );
        }
    
		template <class T>
		void FastCircularQueue<T>::push( T&& element )
		{ 
			condition_.wait();

			//Moved element to queue;
            buffer_[writeIdx_] = std::move( element );

			//Increment the write index and wrap if necessary
			writeIdx_ = ( writeIdx_ + 1 ) % bufferSize_;

			//Update count
			++count_;
		}

		/*
		 * Pop and element off the queue
		*/
		template <class T>
		bool FastCircularQueue<T>::pop( T& result )
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

					//Fetch the value from the buffer
					result = std::move( buffer_[currentReadIdx] );

					//Decrement the count - this must be atomic and come before
					//update count
					--count_;
					condition_.notify();

					//Calculate a new read index - does not have to be atomic
					readIdx_ = ( currentReadIdx + 1 ) % bufferSize_;
					return true;
				}
				else
				{
					std::this_thread::yield(); //Yield to the next thread
					if ( isEmpty() )
						return false; //If the queue is empty, break out.
				}
			}
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
