/*
time: 2026/11/1 14:39:06
*/
#pragma once
#ifndef _FILES_HPP_
#define _FILES_HPP_

#include <vector>
#include <string>

std::vector<std::string> getFiles(const std::string& path, const std::string& pattern, const bool isVerbose = false);

#endif