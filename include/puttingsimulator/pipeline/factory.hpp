#pragma once

#include <memory>
#include <string>

namespace puttingsimulator {

	class PuttingSimPipeline;
}

std::unique_ptr<puttingsimulator::PuttingSimPipeline> buildPipeline(const std::string& source);