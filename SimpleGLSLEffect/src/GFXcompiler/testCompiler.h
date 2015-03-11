#pragma once

#include <string>

namespace testCompiler {
	extern std::string vertexShaderInitCode;
	extern std::string geometryShaderInitCode;
	extern std::string fragmentShaderInitCode;

	extern std::string filename;

	bool init();
	void shutdown();

	bool compile(const std::string &vertexShaderCode, const std::string &geometryShaderCode, const std::string &fragmentShaderCode);
}