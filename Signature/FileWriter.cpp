#include "FileWriter.hpp"

#include <cassert>

namespace Signature
{
	FileWriter::FileWriter( const std::filesystem::path& filePath )
	{
		open( filePath );
	}

	bool FileWriter::open( const std::filesystem::path& filePath )
	{
		stream_.exceptions( std::ifstream::badbit | std::ifstream::failbit );
		stream_.open( filePath, std::ios_base::out | std::ios_base::binary );

		return stream_.is_open();
	}

	void FileWriter::write( const result_data_t& data )
	{
		static constexpr size_t dataSize = sizeof( data.hashSum );

		assert( stream_.is_open() );
		stream_.seekp( data.blockIndex * dataSize, std::ios_base::beg );
		stream_.write( reinterpret_cast<const char*>( &data.hashSum ), dataSize );
	}

	FileWriter::~FileWriter()
	{
		if ( stream_.is_open() )
		{
			stream_.close();
		}
	}
} // namespace Signature
