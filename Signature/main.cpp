#include "Signature.hpp"

#include <iostream>

namespace
{
	static constexpr uint64_t inMegabytes = 1048576;
	static constexpr uint64_t DefaultBlockSize = inMegabytes; // 1 Mb
} // namespace

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
		if ( std::strcmp( argv[3], "-bs" ) )
		{
			std::cout << "Error: Wrong argument, launch app with no arguments for help" << std::endl;

			return 1;
		}

		blockSize = std::atol( argv[4] );

		if ( !blockSize )
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
		auto start = std::chrono::high_resolution_clock::now();
		Signature::MainWorker worker( argv[1], argv[2], blockSize );
		exitCode = worker.execute();
		auto stop = std::chrono::high_resolution_clock::now();

		std::cout << "Done, time: " << std::chrono::duration_cast<std::chrono::seconds>( stop - start ).count() << " sec" << std::endl;
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
