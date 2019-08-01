#include "Signature.hpp"

#include <iostream>

namespace {
	static constexpr uint64_t inMegabytes = 1048576;
	static constexpr uint64_t DefaultBlockSize = inMegabytes; // 1 Mb
}

int main( int argc, char* argv[] )
{
	if ( argc < 3 || argc == 4 || argc > 5 )
	{
		std::cout << "Usage: <app-name> <input-file-path> <output-file-path> [-bs <block size, 1MB by default>]" << std::endl
		<< "\t- enter block size as a decimal number of bytes, 1024B min, 64MB max" << std::endl;

		return 0;
	}

	size_t blockSize = DefaultBlockSize;
		
	if ( argc > 3 )
	{
		blockSize = std::atol( argv[5] );

		if (!blockSize)
		{
			std::cout << "Error: Wrong block size format, launch app with no arguments for help" << std::endl;

			return 1;
		}
		
		if ( blockSize > 64 * inMegabytes || blockSize < 1024 )
		{
			std::cout << "Error: Wrong block size, launch app with no arguments for help" << std::endl;

			return 1;
		}
	}

	int exitCode = 1;

	try
	{
		Signature::MainWorker worker( argv[1], argv[2], blockSize );
		exitCode = worker.execute();
	}
	catch ( const std::exception& e )
	{
		std::cout << "Error: " << e.what() << std::endl;
	}
	catch ( ... )
	{
		std::cout << "Unknown error" << std::endl;
	}

	return exitCode;
}
