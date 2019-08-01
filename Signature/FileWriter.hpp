#pragma once

#include "types.hpp"

#include <fstream>
#include <filesystem>

namespace Signature
{
	class FileWriter final
	{
	public:
		FileWriter() = default;
		FileWriter( const std::filesystem::path& filePath ); 
		~FileWriter();

		bool open( const std::filesystem::path& filePath );
		void write( const result_data_t& data );

	private:
		std::ofstream stream_;
	};
}

