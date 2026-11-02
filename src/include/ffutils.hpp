/*
time: 2026/10/31 23:08:51
*/
#pragma once
#ifndef _PTS_HPP_
#define _PTS_HPP_

#include <string>
#include <vector>
#include <utility>

enum class PTS_GET_WAY {
    ENCAPSULATION = 0b1,
    SOFTWARE_DECODE = 0b10,
    HARDWARE_DECODE = 0b100
};

bool getPTSArray(std::vector<int64_t>& ptsArray, const std::string& filename, const int opt = (int)PTS_GET_WAY::ENCAPSULATION | (int)PTS_GET_WAY::SOFTWARE_DECODE | (int)PTS_GET_WAY::HARDWARE_DECODE, const bool isVerbose = false);
std::pair<int, int> getTimeBase(const std::string& filename);

#endif