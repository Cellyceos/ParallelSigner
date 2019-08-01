#include "Signature.hpp"

#include <iostream>

int main( int argc, char* argv[] )
{
	std::vector<std::string> args;
	for ( int idx = 0; idx < argc; ++idx )
	{
		args.push_back( argv[idx] );
	}

	int exitCode = 1;

	try
	{
		Signature::MainWorker worker( args );
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
