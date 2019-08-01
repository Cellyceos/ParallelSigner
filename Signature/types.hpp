#pragma once

#include <vector>
#include <memory>

namespace Signature
{
	using buffer_t = std::vector<uint8_t>;

	struct chunk_data_t
	{
		size_t blockIndex;
		buffer_t rawData;
	};

	struct result_data_t
	{
		size_t blockIndex;
		uint32_t hashSum;
	};

	using chunk_data_ptr_t = std::unique_ptr<chunk_data_t>;
	using result_data_ptr_t = std::unique_ptr<result_data_t>;
} // namespace Signature
