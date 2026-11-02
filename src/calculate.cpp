#include <algorithm>
#include <functional>

#include "include/calculate.hpp"

using std::vector;
using std::sort;

static vector<uint64_t> displacementArray;
static vector<uint64_t> sortedDisplacementArray;
static vector<uint64_t> sumSortedDisplacementArray;
static vector<size_t> sumNumUniqueDisplacementArray;

void initDisplacements(const size_t& length) {
    sortedDisplacementArray.clear();
    sumSortedDisplacementArray.clear();
    sumNumUniqueDisplacementArray.clear();
    if(displacementArray.capacity() < length) {
        displacementArray.clear();
        displacementArray.reserve(length);
        sortedDisplacementArray.reserve(length + 2);
        sumSortedDisplacementArray.reserve(length + 2);
        sumNumUniqueDisplacementArray.reserve(length + 2);
    }
}

uint64_t preProcess(const vector<int64_t>& sumDisplacementArray) {
    size_t length = sumDisplacementArray.size() - 1;
    initDisplacements(length);
    for(size_t i = 0;i < length;i ++)
        displacementArray[i] = sumDisplacementArray[i + 1] - sumDisplacementArray[i];
    // 不用end()，旧元素不一定删了
    sort(displacementArray.begin(), displacementArray.begin() + length);
    size_t numDifferent = 1;
    sortedDisplacementArray.emplace_back(0);
    sortedDisplacementArray.emplace_back(displacementArray[0]);
    sumSortedDisplacementArray.resize(2, 0);
    sumNumUniqueDisplacementArray.emplace_back(0);
    sumNumUniqueDisplacementArray.emplace_back(1);
    for(size_t i = 1;i < length;i ++) {
        if(sortedDisplacementArray[numDifferent] == displacementArray[i]) {
            sumNumUniqueDisplacementArray[numDifferent] ++;
            continue;
        }
        sumSortedDisplacementArray[numDifferent] += sortedDisplacementArray[numDifferent] * sumNumUniqueDisplacementArray[numDifferent];
        sumNumUniqueDisplacementArray[numDifferent] += sumNumUniqueDisplacementArray[numDifferent - 1];
        sortedDisplacementArray.emplace_back(displacementArray[i]);
        sumSortedDisplacementArray.emplace_back(sumSortedDisplacementArray[numDifferent]);
        sumNumUniqueDisplacementArray.emplace_back(1);
        numDifferent ++;
    }
    sumSortedDisplacementArray[numDifferent] += sortedDisplacementArray[numDifferent] * sumNumUniqueDisplacementArray[numDifferent];
    sumNumUniqueDisplacementArray[numDifferent] += sumNumUniqueDisplacementArray[numDifferent - 1];
    sumNumUniqueDisplacementArray.emplace_back(sumNumUniqueDisplacementArray[numDifferent]);
    if(length & 1)
        return displacementArray[length >> 1];
    else {
        uint64_t temp = displacementArray[length >> 1] + displacementArray[(length - 1) >> 1];
        return (temp >> 1) + (temp & 1);
    }
}

ResultBase iterateBase(const uint64_t& initialBase, uint64_t (*getNextDeviation)(const uint64_t&), const bool isVerbose = false) {
    uint64_t base = initialBase, valueDeviation;
    size_t indexLeft = 1, indexRight = sumSortedDisplacementArray.size(), lastIndexLeft, lastIndexRight;
    while(indexLeft < indexRight) {
        lastIndexLeft = indexLeft;
        lastIndexRight = indexRight;
        valueDeviation = getNextDeviation(base);
        if(isVerbose)
            printf("base = %llu, deviation = %llu, left = %llu %llu, right = %llu %llu\n", base, valueDeviation, indexLeft, sortedDisplacementArray[indexLeft], indexRight, sortedDisplacementArray[indexRight - 1]);
        while(base > sortedDisplacementArray[indexLeft] && base - sortedDisplacementArray[indexLeft] > valueDeviation)
            indexLeft ++;
        while(base < sortedDisplacementArray[indexRight - 1] && sortedDisplacementArray[indexRight - 1] - base > valueDeviation)
            indexRight --;
        // 不能提到while条件，需要的是无剔除的base，而不是无剔除后再算出来的base
        if(indexLeft == lastIndexLeft && indexRight == lastIndexRight)
            break;
        base = (sumSortedDisplacementArray[indexRight - 1] - sumSortedDisplacementArray[indexLeft - 1]) / (long double)(sumNumUniqueDisplacementArray[indexRight - 1] - sumNumUniqueDisplacementArray[indexLeft - 1]) + 0.5L;
    }
    return {
        base,
        sortedDisplacementArray[1],
        *sortedDisplacementArray.rbegin(),
        sumNumUniqueDisplacementArray[indexLeft - 1],
        *sumNumUniqueDisplacementArray.rbegin() - sumNumUniqueDisplacementArray[indexRight - 1]
    };
}

ResultBase findBaseFixed(const uint64_t& valueMid, const long double& coefficient, const bool isVerbose = false) {
    static std::function<uint64_t(const uint64_t&)> getDeviationFixedCoefficient;
    getDeviationFixedCoefficient = [&coefficient](const uint64_t& lastBase) {
        return (uint64_t)(lastBase * coefficient + 0.5L);
    };
    return iterateBase(valueMid, [](const uint64_t& lastBase) {
        return getDeviationFixedCoefficient(lastBase);
    }, isVerbose);
}

ResultBase findBase(const vector<int64_t>& sumDisplacementArray, const int opt, const long double& coefficient, const bool isVerbose) {
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
        printf("initialBase = %llu\n", base);
    if(opt & (int)TOLERATED_DEVIATION_GET_WAY::FIXED)
        return findBaseFixed(base, coefficient, isVerbose);
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
    //         fprintf(stderr, "%llu %llu %llu %llu", sumSortedDisplacementArray[numDifferent], sum[numDifferent & 1], sumNumUniqueDisplacementArray[numDifferent], sumNum[numDifferent & 1]);
    //         return false;
    //     }
    //     printf("%llu, %llu\n", sum[numDifferent & 1], sumNum[numDifferent & 1]);
    //     lastVal = displacementArray[i];
    //     sum[(numDifferent + 1) & 1] = sum[numDifferent & 1] + lastVal;
    //     sumNum[(numDifferent + 1) & 1] = sumNum[numDifferent & 1] + 1;
    //     numDifferent ++;
    // }
    // printf("%llu, %llu\n*\n", sum[numDifferent & 1], sumNum[numDifferent & 1]);
    for(int i = 1;i < sumSortedDisplacementArray.size();i ++) {
        printf("%llu, %llu\n", sortedDisplacementArray[i], sumNumUniqueDisplacementArray[i] - sumNumUniqueDisplacementArray[i - 1]);
    }
    return true;
}