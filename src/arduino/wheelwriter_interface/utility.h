// Utility functions
// Copyright (c) 2024 John Kua <john@kua.fm>
//
#pragma once 

#include <string>
#include <sstream>
#include <vector>

std::vector<std::string> splitString(const std::string& input, char delimiter) {
	std::stringstream ss(input);
	std::string token;
	std::vector<std::string> tokens;
	while (std::getline(ss, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}

class ParameterString {
public:
	ParameterString(const std::string& parameterString, char delimiter=',') {
		tokens_ = splitString(parameterString, delimiter);
	}
	std::string getParameterString(size_t index, std::string defaultValue="") {
		if (index < tokens_.size()) {
			return tokens_[index];
		}
		return defaultValue;
	}
	int getParameterInt(size_t index, int defaultValue=0) {
		if (index < tokens_.size()) {
			return std::stoi(tokens_[index]);
		}
		return defaultValue;
	}
	float getParameterFloat(size_t index, int defaultValue=0.0) {
		if (index < tokens_.size()) {
			return std::stof(tokens_[index]);
		}
		return defaultValue;
	}
	std::vector<std::string> tokens_;
};