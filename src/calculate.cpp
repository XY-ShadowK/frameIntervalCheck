#include <cstdio>
#include <algorithm>
#include <functional>
#include <map>
#include <cinttypes>

#include "include/calculate.hpp"

using std::vector;
using std::sort;

static vector<uint64_t> displacementArray;
static vector<uint64_t> sortedUniqueDisplacementArray;
static vector<uint64_t> sumSortedDisplacementArray;
static vector<size_t> sumNumUniqueDisplacementArray;

static bool checkOutlierBaseGreaterThanDeviation(const uint64_t& base, const uint64_t& value, const uint64_t& deviation) {
    return base > value && base > value + deviation;
}
static bool checkOutlierBaseLessThanDeviation(const uint64_t& base, const uint64_t& value, const uint64_t& deviation) {
    return checkOutlierBaseGreaterThanDeviation(value, base, deviation);
}

static void initDisplacements(const size_t& length) {
    sortedUniqueDisplacementArray.clear();
    sumSortedDisplacementArray.clear();
    sumNumUniqueDisplacementArray.clear();
    if(displacementArray.capacity() < length) {
        displacementArray.clear();
        displacementArray.resize(length);
        sortedUniqueDisplacementArray.reserve(length + 2);
        sumSortedDisplacementArray.reserve(length + 2);
        sumNumUniqueDisplacementArray.reserve(length + 2);
    }
}

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
T getMidInRange(const vector<T>& sortedArray, const size_t& indexLeft, const size_t& lengthRight) {
    uint64_t length = lengthRight - indexLeft;
    if(length & 1)
        return sortedArray[indexLeft + (length >> 1)];
    else {
        T temp = sortedArray[indexLeft + (length >> 1)] + sortedArray[indexLeft + (length >> 1) - 1];
        return temp / 2 + temp % 2;
    }
}

static uint64_t preProcess(const vector<int64_t>& sumDisplacementArray) {
    size_t length = sumDisplacementArray.size() - 1;
    initDisplacements(length);
    for(size_t i = 0;i < length;i ++)
        displacementArray[i] = sumDisplacementArray[i + 1] - sumDisplacementArray[i];
    // 不用end()，旧元素不一定删了
    sort(displacementArray.begin(), displacementArray.begin() + length);
    size_t numDifferent = 1;
    sortedUniqueDisplacementArray.emplace_back(0);
    sortedUniqueDisplacementArray.emplace_back(displacementArray[0]);
    sumSortedDisplacementArray.resize(2, 0);
    sumNumUniqueDisplacementArray.emplace_back(0);
    sumNumUniqueDisplacementArray.emplace_back(1);
    for(size_t i = 1;i < length;i ++) {
        if(sortedUniqueDisplacementArray[numDifferent] == displacementArray[i]) {
            sumNumUniqueDisplacementArray[numDifferent] ++;
            continue;
        }
        sumSortedDisplacementArray[numDifferent] += sortedUniqueDisplacementArray[numDifferent] * sumNumUniqueDisplacementArray[numDifferent];
        sumNumUniqueDisplacementArray[numDifferent] += sumNumUniqueDisplacementArray[numDifferent - 1];
        sortedUniqueDisplacementArray.emplace_back(displacementArray[i]);
        sumSortedDisplacementArray.emplace_back(sumSortedDisplacementArray[numDifferent]);
        sumNumUniqueDisplacementArray.emplace_back(1);
        numDifferent ++;
    }
    sumSortedDisplacementArray[numDifferent] += sortedUniqueDisplacementArray[numDifferent] * sumNumUniqueDisplacementArray[numDifferent];
    sumNumUniqueDisplacementArray[numDifferent] += sumNumUniqueDisplacementArray[numDifferent - 1];
    sumNumUniqueDisplacementArray.emplace_back(sumNumUniqueDisplacementArray[numDifferent]);
    return getMidInRange(displacementArray, 0, length);
}

ResultBase iterateBase(const uint64_t& initialBase
, uint64_t (*getNextDeviation)(const uint64_t&, const size_t&, const size_t&)
, bool (*checkOutlierLeft)(const uint64_t&, const uint64_t&, const uint64_t&)
, bool (*checkOutlierRight)(const uint64_t&, const uint64_t&, const uint64_t&)
, const bool isVerbose = false) {
    uint64_t base = initialBase, valueDeviation;
    size_t indexUniqueLeft = 1, lengthUniqueRight = sumSortedDisplacementArray.size(), lastIndexUniqueLeft, lastSizeUniqueRight;
    while(indexUniqueLeft < lengthUniqueRight) {
        lastIndexUniqueLeft = indexUniqueLeft;
        lastSizeUniqueRight = lengthUniqueRight;
        valueDeviation = getNextDeviation(base, indexUniqueLeft, lengthUniqueRight);
        if(isVerbose)
            fprintf(stdout, "base = %" PRIu64 ", deviation = %" PRIu64 ", left = %" PRIu64 " | %" PRIu64 ", right = %" PRIu64 " | %" PRIu64 "\n", base, valueDeviation, indexUniqueLeft, sortedUniqueDisplacementArray[indexUniqueLeft], lengthUniqueRight, sortedUniqueDisplacementArray[lengthUniqueRight - 1]);
        while(checkOutlierLeft(base, sortedUniqueDisplacementArray[indexUniqueLeft], valueDeviation))
            indexUniqueLeft ++;
        while(checkOutlierRight(base, sortedUniqueDisplacementArray[lengthUniqueRight - 1], valueDeviation))
            lengthUniqueRight --;
        // 不能提到while条件处，需要的是无剔除的base，而不是无剔除后再算出来的base
        if(indexUniqueLeft == lastIndexUniqueLeft && lengthUniqueRight == lastSizeUniqueRight)
            break;
        // 相信编译器的优化OVO
        base = (sumSortedDisplacementArray[lengthUniqueRight - 1] - sumSortedDisplacementArray[indexUniqueLeft - 1]) / (long double)(sumNumUniqueDisplacementArray[lengthUniqueRight - 1] - sumNumUniqueDisplacementArray[indexUniqueLeft - 1]) + 0.5L;
    }
    return {
        base,
        sortedUniqueDisplacementArray[1],
        *sortedUniqueDisplacementArray.rbegin(),
        sumNumUniqueDisplacementArray[indexUniqueLeft - 1],
        *sumNumUniqueDisplacementArray.rbegin() - sumNumUniqueDisplacementArray[lengthUniqueRight - 1]
    };
}

ResultBase findBaseFixed(const uint64_t& initialBase, const long double& coefficient
, const bool isVerbose = false) {
    if(isVerbose)
        fprintf(stdout, "using fixed tolerated deviation rate %llf to calculate base:\n", coefficient);
    static std::function<uint64_t(const uint64_t&)> getDeviationFixedCoefficient;
    getDeviationFixedCoefficient = [&coefficient](const uint64_t& lastBase) {
        return (uint64_t)(lastBase * coefficient + 0.5L);
    };
    static auto getDeviationFixed = [](const uint64_t& lastBase, const size_t& indexUniqueLeft, const size_t& lengthUniqueRight) {
        return getDeviationFixedCoefficient(lastBase);
    };
    return iterateBase(initialBase, getDeviationFixed, checkOutlierBaseGreaterThanDeviation, checkOutlierBaseLessThanDeviation, isVerbose);
}

ResultBase findBaseIQR(const uint64_t& initialBase, const long double& coefficient
, const bool isVerbose = false) {
    if(isVerbose)
        fprintf(stdout, "using IQR threshold to calculate base:\n");
    static size_t valueQ1, valueQ3, lengthSub1;
    static uint64_t weightRight1, weightRight3;
    static std::function<uint64_t(const uint64_t&, const size_t&, const size_t&)> getDeviationIQRCoefficient;
    getDeviationIQRCoefficient = [&coefficient, &isVerbose](const uint64_t& lastBase, const size_t& indexUniqueLeft, const size_t& lengthUniqueRight) {
        size_t indexUniqueLeftSub1 = indexUniqueLeft - 1;
        lengthSub1 = sumNumUniqueDisplacementArray[lengthUniqueRight - 1] - sumNumUniqueDisplacementArray[indexUniqueLeftSub1] - 1;
        size_t indexQ1 = lengthSub1 >> 2, indexQ3 = (lengthSub1 * 3) >> 2;
        weightRight1 = indexQ1 & 3;
        weightRight3 = indexQ3 & 3;
        valueQ1 = (displacementArray[sumNumUniqueDisplacementArray[indexUniqueLeftSub1] + indexQ1] * (4 - weightRight1) + displacementArray[sumNumUniqueDisplacementArray[indexUniqueLeftSub1] + indexQ1 + 1] * weightRight1) / 4.0L + 0.5L;
        valueQ3 = (displacementArray[sumNumUniqueDisplacementArray[indexUniqueLeftSub1] + indexQ3] * (4 - weightRight3) + displacementArray[sumNumUniqueDisplacementArray[indexUniqueLeftSub1] + indexQ3 + 1] * weightRight3) / 4.0L + 0.5L;
        if(isVerbose)
            fprintf(stdout, "length = %" PRIu64 ", Q1 = %" PRIu64 ", Q3 = %" PRIu64 "\n", lengthSub1 + 1, valueQ1, valueQ3);
        return (uint64_t)((valueQ3 - valueQ1) * coefficient + 0.5L);
    };
    static auto getDeviationIQR = [](const uint64_t& lastBase, const size_t& indexUniqueLeft, const size_t& lengthUniqueRight) {
        return getDeviationIQRCoefficient(lastBase, indexUniqueLeft, lengthUniqueRight);
    };
    static auto checkOutlierLeft = [](const uint64_t& base, const uint64_t& valueTest, const uint64_t& deviation) {
        return valueQ1 > valueTest + deviation;
    };
    static auto checkOutlierRight = [](const uint64_t& base, const uint64_t& valueTest, const uint64_t& deviation) {
        return valueQ3 + deviation < valueTest;
    };
    return iterateBase(initialBase, getDeviationIQR, checkOutlierLeft, checkOutlierRight, isVerbose);
}

ResultBase findBaseMAD(const uint64_t& initialValueMid, const long double& coefficient
, const bool isVerbose = false) {
    if(isVerbose)
        fprintf(stdout, "using MAD threshold to calculate base:\n");
    static std::map<uint64_t, size_t> absNums;
    static uint64_t valueMid, indexMid, valueABSMid;
    static std::function<uint64_t(const uint64_t&, const size_t&, const size_t&)> getDeviationMADCoefficient;
    getDeviationMADCoefficient = [&coefficient](const uint64_t& lastBase, const size_t& indexUniqueLeft, const size_t& lengthUniqueRight) {
        size_t indexUniqueLeftSub1 = indexUniqueLeft - 1, lengthUniqueRightSub1 = lengthUniqueRight - 1, numNow = 0;
        std::map<uint64_t, size_t>::const_iterator itAbsNum;
        valueMid = getMidInRange(displacementArray, sumNumUniqueDisplacementArray[indexUniqueLeftSub1], sumNumUniqueDisplacementArray[lengthUniqueRightSub1]);
        indexMid = sumNumUniqueDisplacementArray[lengthUniqueRightSub1] - sumNumUniqueDisplacementArray[indexUniqueLeftSub1];
        bool odd = indexMid & 1;
        indexMid >>= 1;
        absNums.clear();
        for(size_t i = indexUniqueLeft;i < lengthUniqueRight;i ++)
            // map默认初始化size_t为0
            absNums[valueMid > sortedUniqueDisplacementArray[i] ? valueMid - sortedUniqueDisplacementArray[i] : sortedUniqueDisplacementArray[i] - valueMid] += sumNumUniqueDisplacementArray[i] - sumNumUniqueDisplacementArray[i - 1];
        itAbsNum = absNums.begin();
        while(numNow <= indexMid && itAbsNum != absNums.end()) {
            numNow += itAbsNum->second;
            itAbsNum ++;
        }
        itAbsNum --;
        if(odd || itAbsNum->second > 1)
            return itAbsNum->first;
        valueABSMid = itAbsNum->first;
        itAbsNum --;
        valueABSMid += itAbsNum->first;
        valueABSMid = (valueABSMid >> 1) + valueABSMid & 1;
        return (uint64_t)(valueABSMid * coefficient);
    };
    static auto getDeviationMAD = [](const uint64_t& lastBase, const size_t& indexUniqueLeft, const size_t& lengthUniqueRight) {
        return getDeviationMADCoefficient(lastBase, indexUniqueLeft, lengthUniqueRight);
    };
    return iterateBase(initialValueMid, getDeviationMAD, checkOutlierBaseGreaterThanDeviation, checkOutlierBaseLessThanDeviation, isVerbose);
}

ResultBase findBase(const vector<int64_t>& sumDisplacementArray, const long double& coefficient
, const int opt
, const bool isVerbose) {
    if(sumDisplacementArray.size() < 3) {
        uint64_t temp = *sumDisplacementArray.rbegin() - *sumDisplacementArray.begin();
        return {
            temp,
            temp,
            temp,
            0,
            0
        };
    }
    uint64_t base = preProcess(sumDisplacementArray);
    if(isVerbose)
        fprintf(stdout, "initialBase = %" PRIu64 "\n", base);
    switch(opt) {
        case (int)TOLERATED_DEVIATION_GET_WAY::FIXED:
            return findBaseFixed(base, coefficient, isVerbose);
        case (int)TOLERATED_DEVIATION_GET_WAY::IQR_CALCULATE:
            return findBaseIQR(base, coefficient, isVerbose);
        case (int)TOLERATED_DEVIATION_GET_WAY::MAD_CALCULATE:
            return findBaseMAD(base, coefficient, isVerbose);
    }
    return {};
}

bool testPreProcess(const vector<int64_t>& sumDisplacementArray) {
    preProcess(sumDisplacementArray);
    // uint64_t sum[2], lastVal = displacementArray[0];
    // size_t sumNum[2], numDifferent = 1, length = sumDisplacementArray.size() - 1;
    // sum[0] = 0;
    // sum[1] = lastVal;
    // sumNum[0] = 0;
    // sumNum[1] = 1;
    // for(size_t i = 1;i < length;i ++) {
    //     if(lastVal == displacementArray[i]) {
    //         sum[numDifferent & 1] += lastVal;
    //         sumNum[numDifferent & 1] ++;
    //         continue;
    //     }
    //     if(sumSortedDisplacementArray[numDifferent] != sum[numDifferent & 1] && sumNumUniqueDisplacementArray[numDifferent] != sumNum[numDifferent & 1]) {
    //         fprintf(stderr, "%" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "", sumSortedDisplacementArray[numDifferent], sum[numDifferent & 1], sumNumUniqueDisplacementArray[numDifferent], sumNum[numDifferent & 1]);
    //         return false;
    //     }
    //     fprintf(stdout, "%" PRIu64 ", %" PRIu64 "\n", sum[numDifferent & 1], sumNum[numDifferent & 1]);
    //     lastVal = displacementArray[i];
    //     sum[(numDifferent + 1) & 1] = sum[numDifferent & 1] + lastVal;
    //     sumNum[(numDifferent + 1) & 1] = sumNum[numDifferent & 1] + 1;
    //     numDifferent ++;
    // }
    // fprintf(stdout, "%" PRIu64 ", %" PRIu64 "\n*\n", sum[numDifferent & 1], sumNum[numDifferent & 1]);
    for(int i = 1;i < sumSortedDisplacementArray.size();i ++) {
        fprintf(stdout, "%" PRIu64 ", %" PRIu64 "\n", sortedUniqueDisplacementArray[i], sumNumUniqueDisplacementArray[i] - sumNumUniqueDisplacementArray[i - 1]);
    }
    return true;
}