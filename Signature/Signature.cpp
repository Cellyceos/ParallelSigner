#include "Signature.hpp"
#include "FileReader.hpp"
#include "FileWriter.hpp"
#include "CRC32.hpp"

#include <cassert>

namespace Signature
{
	MainWorker::MainWorker( const std::filesystem::path& inFilePath, const std::filesystem::path& outFilePath, size_t blockSize ) :
		inFilePath_( inFilePath ), outFilePath_( outFilePath ), blockSize_( blockSize )
	{
		if ( !std::filesystem::exists( inFilePath ) )
		{
			throw std::invalid_argument( "input file doesn't exist" );
		}

		if ( !std::filesystem::exists( outFilePath.parent_path() ) )
		{
			throw std::invalid_argument( "output directory doesn't exist" );
		}

		if ( !blockSize )
		{
			throw std::invalid_argument( "Block size is zero" );
		}

		maxThreadPool_ = std::thread::hardware_concurrency();
		if ( !maxThreadPool_ )
		{
			maxThreadPool_ = defaultThreadCount;
		}

		maxPoolDataZize_ = maxThreadPool_ * 2;
		jobDataPool_ = std::make_unique<Concurency::FastCircularQueue<chunk_data_ptr_t>>( maxPoolDataZize_ );
		freeChunkPool_ = std::make_unique<Concurency::FastCircularQueue<chunk_data_ptr_t>>( maxPoolDataZize_ );

		writerPool_ = std::make_unique<Concurency::FastCircularQueue<result_data_ptr_t>>( maxPoolDataZize_ );
		freeResultPool_ = std::make_unique<Concurency::FastCircularQueue<result_data_ptr_t>>( maxPoolDataZize_ );

		while ( freeChunkPool_->count() < maxPoolDataZize_ )
		{
			freeChunkPool_->push( std::make_unique<chunk_data_t>( blockSize_ ) );
			freeResultPool_->push( std::make_unique<result_data_t>() );
		}

		for ( size_t idx = 0; idx < maxThreadPool_; ++idx )
		{
			threadPool_.push_back( std::async( std::launch::async, &MainWorker::hashWorker, this ) );
		}

		threadPool_.push_back( std::async( std::launch::async, &MainWorker::writeWorker, this ) );
	}

	MainWorker::~MainWorker()
	{
		waitThreads();
	}

	void MainWorker::waitThreads()
	{
		for ( std::future<void>& task : threadPool_ )
		{
			task.get();
		}

		threadPool_.clear();
	}

	int MainWorker::execute()
	try
	{
		FileReader reader( inFilePath_ );
		size_t blockCount = reader.fileSize() / blockSize_ + ( reader.fileSize() % blockSize_ > 0 );

		chunk_data_ptr_t chunk;
		for ( size_t blockIdx = 0; blockIdx < blockCount; ++blockIdx )
		{
			if ( somethingGoesWrong_.load( std::memory_order_relaxed ) )
				return 1;

			//Critical section
			{
				std::unique_lock<std::mutex> lock( chunkMutex_ );
				if ( jobDataPool_->isEmpty() )
				{
					while ( !jobPoolNotFull_.wait_for( lock, threadTimeout, [this]() { return !freeChunkPool_->isEmpty(); } ) )
						continue;
				}

				chunk = freeChunkPool_->pop();
				assert( chunk );
			}

			chunk->blockIndex = blockIdx;
			reader.read( chunk->rawData );

			jobDataPool_->push( std::move( chunk ) );
		}

		prepareToExit_.store( true, std::memory_order_relaxed );
		waitThreads();

		return 0;
	}
	catch ( ... )
	{
		somethingGoesWrong_.store( true, std::memory_order_relaxed );
		throw;
	}

	void MainWorker::hashWorker() try
	{
		chunk_data_ptr_t chunk;
		result_data_ptr_t result;

		while ( true )
		{
			//Critical section
			{
				std::unique_lock<std::mutex> lock( jobMutex_ );
				if ( jobDataPool_->isEmpty() )
				{
					while ( !jobPoolNotEmpty_.wait_for( lock, threadTimeout, [this]() { return !jobDataPool_->isEmpty() || prepareToExit_.load( std::memory_order_relaxed ) || somethingGoesWrong_.load( std::memory_order_relaxed ); } ) )
						continue;

					if ( jobDataPool_->isEmpty() && prepareToExit_.load( std::memory_order_relaxed ) || somethingGoesWrong_.load( std::memory_order_relaxed ) )
						return;
				}

				chunk = jobDataPool_->pop();
				assert( chunk );

				result = freeResultPool_->pop();
				assert( result );
			}

			result->blockIndex = chunk->blockIndex;
			result->hashSum = Security::CRC32::calculate( chunk->rawData );
			writerPool_->push( std::move( result ) );

			//Clear data in chunk
			std::fill( chunk->rawData.begin(), chunk->rawData.end(), 0 );
			chunk->blockIndex = 0;

			//retrun to free chunk pool
			freeChunkPool_->push( std::move( chunk ) );
		}
	}
	catch ( ... )
	{
		somethingGoesWrong_.store( true, std::memory_order_relaxed );
		throw;
	}

	void MainWorker::writeWorker()
	try
	{
		FileWriter writer( outFilePath_ );
		result_data_ptr_t data;

		while ( true )
		{
			{
				std::unique_lock<std::mutex> lock( writerMutex_ );
				if ( writerPool_->isEmpty() )
				{
					while ( !writerPoolNotEmpty_.wait_for( lock, std::chrono::milliseconds( 100 ), [this]() { return !writerPool_->isEmpty() || prepareToExit_.load( std::memory_order_relaxed ) || somethingGoesWrong_.load( std::memory_order_relaxed ); } ) )
						continue;

					if ( writerPool_->isEmpty() && prepareToExit_.load( std::memory_order_relaxed ) || somethingGoesWrong_.load( std::memory_order_relaxed ) )
						return;
				}

				data = writerPool_->pop();
				assert( data );
			}

			writer.write( *data );

			//clear data
			data->blockIndex = 0;
			data->hashSum = 0;

			//return to pool
			freeResultPool_->push( std::move( data ) );
		}
	}
	catch ( ... )
	{
		somethingGoesWrong_.store( true, std::memory_order_relaxed );
		throw;
	}
} // namespace Signature
