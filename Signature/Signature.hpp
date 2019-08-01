#pragma once

#include <vector>
#include <string>

namespace Signature
{
	class MainWorker
	{
	public:
		MainWorker( const std::vector<std::string>& args );
		~MainWorker();

		int execute();

	private:
	};
} // namespace Signature
