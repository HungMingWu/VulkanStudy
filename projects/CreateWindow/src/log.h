#pragma once

#ifdef _DEBUG
#include <string>
#include <iostream>
static int __tabCount = 0;
static std::string __tabString = "";
#define LOG(x) { std::cout << __tabString << x << std::endl; }
#define FUNCNAME() LogFuncName __logFuncName(__FUNCTION__);
#define TAB() { __tabCount++; __tabString = std::string(__tabCount, '\t'); }
#define UNTAB() { __tabCount--; __tabString = std::string(__tabCount, '\t'); }
struct LogFuncName {
	LogFuncName(const char* fname) {
		LOG(fname)
			TAB()
	}
	~LogFuncName() { UNTAB() }
};
#else
#define LOG(x) {}
#define FUNCNAME() {}
#define TAB() {}
#define UNTAB() {}
#endif