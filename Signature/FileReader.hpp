#pragma once

#include "types.hpp"

#include <fstream>
#include <filesystem>

namespace Signature
{
	class FileReader final
	{
	public:
		FileReader() = default;
		FileReader( const std::filesystem::path& filePath );
		~FileReader();

		bool open( const std::filesystem::path& filePath );
		void read( buffer_t& buffer );

		uintmax_t fileSize() const { return fileSize_; }

	private:
		uintmax_t fileSize_;
		std::ifstream stream_;

		FileReader( const FileReader& ) = delete;
		FileReader( FileReader&& ) = delete;
	};
} // namespace Signature
