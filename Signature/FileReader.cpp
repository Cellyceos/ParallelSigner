#include "FileReader.hpp"

#include <cassert>

namespace Signature
{
	FileReader::FileReader( const std::filesystem::path& filePath ) :
		fileSize_( std::filesystem::file_size( filePath ) )
	{
		open( filePath );
	}

	bool FileReader::open( const std::filesystem::path& filePath )
	{
		stream_.exceptions( std::ifstream::badbit | std::ifstream::failbit );
		stream_.open( filePath, std::ios_base::in | std::ios_base::binary );
	
		return stream_.is_open();
	}

	void FileReader::read( buffer_t& buffer )
	{
		assert( stream_.is_open() );
		assert( buffer.size());

		stream_.read( reinterpret_cast<char*>( buffer.data() ), buffer.size() );
	}

	FileReader::~FileReader()
	{
		if ( stream_.is_open() )
		{
			stream_.close();
		}
	}
} // namespace Signature
