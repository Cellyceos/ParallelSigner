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
	};
} // namespace Signature
