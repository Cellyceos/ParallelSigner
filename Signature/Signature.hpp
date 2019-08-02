#pragma once

#include <filesystem>

namespace Signature
{
	class MainWorker
	{
	public:
		MainWorker( const std::filesystem::path& inFilePath, const std::filesystem::path& outFilePath, size_t blockSize );
		~MainWorker();

		int execute();

	private:
        const std::filesystem::path inFilePath_;
        const std::filesystem::path outFilePath_;
        const size_t blockSize_;
        
        
	};
} // namespace Signature
