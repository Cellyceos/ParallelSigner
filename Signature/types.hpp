#pragma once

#include <vector>
#include <memory>

namespace Signature
{
	using buffer_t = std::vector<uint8_t>;

	struct chunk_data_t
	{
		size_t blockIndex = 0;
		buffer_t rawData;

		chunk_data_t( size_t reservedSize ) :
			rawData( reservedSize ) {}
	};

	struct result_data_t
	{
		size_t blockIndex = 0;
		uint32_t hashSum = 0;
	};

	using chunk_data_ptr_t = std::unique_ptr<chunk_data_t>;
	using result_data_ptr_t = std::unique_ptr<result_data_t>;
} // namespace Signature
