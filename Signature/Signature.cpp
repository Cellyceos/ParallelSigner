#include "Signature.hpp"
#include "FileReader.hpp"

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
	}

	MainWorker::~MainWorker()
	{
	}

	int MainWorker::execute()
	{
        FileReader reader( inFilePath_ );
        size_t blockCount = reader.fileSize() / blockSize_ + ( reader.fileSize() % blockSize_ > 0 );
		return 0;
	}
} // namespace Signature
