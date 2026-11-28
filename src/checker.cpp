#ifdef CMAKE_BUILD
#include "version.h"
#else
#include "compiler/version.h.in"
#endif

#include "include/config.h"

#include <iostream>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <cinttypes>

#include "Universe_Entry.hpp"
#include "cxxopts.hpp"

#include "include/files.hpp"
#include "include/ffutils.hpp"
#include "include/calculate.hpp"

using std::string;
using std::to_string;
using std::vector;

std::unordered_map<string, int> optionMasks = {
    {"deviation-rate", (int)TOLERATED_DEVIATION_GET_WAY::FIXED},
    {"IQR", (int)TOLERATED_DEVIATION_GET_WAY::IQR_CALCULATE},
    {"MAD", (int)TOLERATED_DEVIATION_GET_WAY::MAD_CALCULATE},
};
cxxopts::Options options("frameIntervalCheck", "检查帧间隔异常情况");
cxxopts::ParseResult settings;

int optGetPTS = (int)PTS_GET_WAY::CONTAINER | (int)PTS_GET_WAY::SOFTWARE_DECODE;
int optGetBase = (int)TOLERATED_DEVIATION_GET_WAY::FIXED;
long double coefficient = 0.2L;

bool mutuallyExclusiveCheck(const vector<string>& optionNames, void(*callback)(const string&) = nullptr) {
    bool isMutuallyExclusive = false;
    string listName = "";
    for(const string& optionName : optionNames) {
        if(settings.count(optionName)) {
            if(isMutuallyExclusive) {
                fprintf(stderr, "不可同时设置 %s 。\n", listName.c_str());
                return false;
            }
            if(callback)
                callback(optionName);
            isMutuallyExclusive = true;
        }
        listName += optionName;
    }
    return true;
}

int parseArgs(int argc, char* const* const argv) {
    try {
        options
        .set_width(140)
        .add_options()
            ("file", "文件名", cxxopts::value<string>(), "FILE")
            ("path", "扫描路径（与file冲突）", cxxopts::value<string>(), "PATH")
            ("pattern", "扫描匹配模式（必须启用path）", cxxopts::value<string>()->default_value("*.mp4"), "PATTERN")
            ("container-only", "仅扫描封装帧信息", cxxopts::value<int>()->default_value("0")->implicit_value(to_string((int)PTS_GET_WAY::SOFTWARE_DECODE), cxxopts::ImplicitArgPolicy::Disabled), " ")
            ("decode-only", "仅解码获取帧信息", cxxopts::value<int>()->default_value("0")->implicit_value(to_string((int)PTS_GET_WAY::CONTAINER), cxxopts::ImplicitArgPolicy::Disabled), " ")
            ("hardware-decode", "启用硬件解码（将会关闭软解）", cxxopts::value<int>()->default_value("0")->implicit_value(to_string((int)PTS_GET_WAY::HARDWARE_DECODE), cxxopts::ImplicitArgPolicy::Disabled), " ")
            ("deviation-rate", "可容忍的偏差率（与iqr、mad冲突）", cxxopts::value<long double>()->default_value("0.2"), "TOLERATED_DEVIATION_RATE")
            ("IQR", "使用四分位距算法计算容忍偏差（与deviation-rate、mad冲突）", cxxopts::value<long double>()->implicit_value("1.5"), "COEFFICIENT")
            ("MAD", "使用中位数绝对偏差算法计算容忍偏差（与deviation-rate、iqr冲突）", cxxopts::value<long double>()->implicit_value("3.0"), "COEFFICIENT")
            ("verbose", "显示详细信息")
            ("v,version", "打印当前版本")
            ("h,help", "打印帮助信息");
        settings = options.parse(argc, argv);
        if(argc == 1 || settings.count("help")) {
            printf("%s", options.help().c_str());
            return 1;
        }
        if(settings.count("version"))
            printf("%s " APP_VERSION "\n", options.program().c_str());
        mutuallyExclusiveCheck({
            "file",
            "path"
        });
        if(settings.count("pattern"))
            if(!settings.count("path")) {
                fprintf(stderr, "缺少选项 path。\n");
                return -1;
            }
        optGetPTS ^= settings["container-only"].as<int>() ^ settings["decode-only"].as<int>() ^ settings["hardware-decode"].as<int>();
        mutuallyExclusiveCheck({
            "deviation-rate",
            "IQR",
            "MAD"
        }, [](const string& optionName) {
            coefficient = settings[optionName].as<long double>();
            optGetBase = optionMasks[optionName];
        });
        if(settings.unmatched().size()) {
            fprintf(stderr, "%s 不是可识别的选项或参数。\n", settings.unmatched()[0].c_str());
            return -1;
        }
    } catch (const cxxopts::exceptions::exception& e)
    {
        fprintf(stderr, "error parsing options: %s\n", e.what());
        return -1;
    }
    return 0;
}

int resolve() {
    vector<string> files;
    vector<int64_t> ptsArray;
    vector<size_t> indexAbnormalArray;
    // vector<uint64_t> numAbnormalArray;
    std::pair<int ,int> timeBase;
    ResultBase resultPTSTest;
    uint64_t temp;
    size_t numError = 0;
    bool isVerbose = settings["verbose"].as<bool>();
    string path = ".";
    if(settings.count("path")) {
        path = settings["path"].as<string>();
        files = getFiles(path, settings["pattern"].as<string>(), isVerbose);
    }
    else if(settings.count("file"))
        files = { settings["file"].as<string>() };
    else if(settings.count("version"))
        return 0;
    else
        return -1;

    printf("\n");
    indexAbnormalArray.reserve(files.size());
    // numAbnormalArray.reserve(files.size());
    for(size_t i = 0;i < files.size();i ++) {
        const string& file = files[i];
        printf("文件: %s\n", file.c_str());
        const string& filenameFull = path + '/' + file;
        if(getPTSArray(ptsArray, filenameFull, optGetPTS, isVerbose)) {
            timeBase = getTimeBase(filenameFull);
            resultPTSTest = findBase(ptsArray, settings["deviation-rate"].as<long double>(), optGetBase, isVerbose);
            // printf("%d\n", testPreProcess(ptsArray));
            temp = resultPTSTest.numOutlierLeft + resultPTSTest.numOutlierRight;
            if(temp) {
                indexAbnormalArray.emplace_back(i);
                // numAbnormalArray.emplace_back(temp);
            }
            printf("时间基: %d/%d 总帧数: %" PRIu64 "\n帧间隔: base = %" PRIu64 ", min = %" PRIu64 ", max = %" PRIu64 ", num_too_small = %" PRIu64 ", num_too_large = %" PRIu64 "\n", timeBase.first, timeBase.second, ptsArray.size(), resultPTSTest.base, resultPTSTest.minn, resultPTSTest.maxn, resultPTSTest.numOutlierLeft, resultPTSTest.numOutlierRight);
        }
        else {
            printf("获取帧时间戳失败。\n");
            numError ++;
        }
        ptsArray.clear();
    }
    printf("\n共扫描 %" PRIu64 " 个文件，共 %" PRIu64 " 个获取帧信息失败，共 %" PRIu64 " 个存在异常帧间隔。\n",files.size(), numError, indexAbnormalArray.size());
    return 0;
}

UNIVERSE_MAIN
    int ret = 0;
    if((ret = parseArgs(argc, argv)) == 0) {
        UNIVERSE_MAIN_RETURN(resolve());
    }
    if(ret < 0) {
        UNIVERSE_MAIN_RETURN(ret);
    }
    UNIVERSE_MAIN_RETURN(0);
UNIVERSE_MAINEND