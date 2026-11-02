/*
time: 2026/11/1 14:39:06
*/
#pragma once
#ifndef _CALCULATE_HPP_
#define _CALCULATE_HPP_

#include <vector>

enum class TOLERATED_DEVIATION_GET_WAY {
    FIXED = 0b1,
    IQR_CALCULATE = 0b10,
    MAD_CALCULATE = 0b100
};

typedef struct tResultBase {
    uint64_t base;
    uint64_t minn;
    uint64_t maxn;
    uint64_t numOutlierLeft;
    uint64_t numOutlierRight;
}ResultBase;

ResultBase findBase(const std::vector<int64_t>& sumDisplacementArray, const int opt = (int)TOLERATED_DEVIATION_GET_WAY::FIXED, const long double& coefficient = 0.2L, const bool isVerbose = false);
bool testPreProcess(const std::vector<int64_t>& sumDisplacementArray);

#endif