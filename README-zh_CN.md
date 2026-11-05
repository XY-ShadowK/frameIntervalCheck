# frameIntervalCheck

[English](README.md) | 简体中文

检查视频是否存在异常帧。

## 如何构建？

在根目录运行命令：

```Shell
mkdir build
cmake -B build
cmake --build ./build
```

对于MSVC编译器，将 `cmake --build ./build` 改为 `cmake --build ./build --config release`。

如果你的系统没有安装windowsRub、cxxopts、ffmpeg等库，将 `cmake -B build` 改为如下

**POWERSHELL（Windows终端 非CMD）**

```PowerShell
cmake -B build `
    -DWINDOWS_RUB_INCLUDE="{你的 windowsRub include 目录}" `
    -DCXXOPTS_ROOT="{你的 cxxopts 根目录}" `
    -DFFMPEG_ROOT="{你的 ffmpeg 根目录}"
```

**SHELL(LINUX终端)**

```Shell
cmake -B build \
    -DWINDOWS_RUB_INCLUDE="{你的 windowsRub include 目录}" \
    -DCXXOPTS_ROOT="{你的 cxxopts 根目录}" \
    -DFFMPEG_ROOT="{你的 ffmpeg 根目录}"
```

## 如何使用？

1. 打开一个终端或命令行
2. cd到可执行文件（widnows下为.exe linux下无后缀）所在的目录
3. 运行 `frameIntervalCheck -h(或 --help)` 来得到帮助信息
   简单运行示例： `frameIntervalCheck --path=./` 将会扫描当前目录下所有mp4的帧信息

## 选项列表：

* [`--file`](README-zh_CN.md##--file) : 设置待扫描视频文件名
* [`--path`](README-zh_CN.md##--path) : 设置批量扫描目录
* [`--pattern`](README-zh_CN.md##--pattern) : 设置批量扫描文件名通配符模式
* [`--container-only`](README-zh_CN.md##--container-only) : 开启仅扫描封装模式
* [`--decode-only`](README-zh_CN.md##--decode-only) : 开启仅解码模式
* [`--harddware-decode`](README-zh_CN.md##--harddware-decode) : 开启硬件解码（将关闭软解）
* [`--deviation-rate`](README-zh_CN.md##--deviation-rate) : 设置使用固定容忍率
* [`--IQR`](README-zh_CN.md##--IQR) : 设置使用四分位距算法
* [`--MAD`](README-zh_CN.md##--MAD) : 设置使用中位数绝对偏差算法
* [`--verbose`](README-zh_CN.md##--verbose) : 开启详细信息显示
* [`-h, --help`](README-zh_CN.md##-h,---help) : 打印帮助信息

## 输出信息意义

* `时间基` ：该视频时间基础单位，如 `1/10880 (秒)`
* `总帧数` ：该视频所有帧总数，如 `4309 (帧)`
* `帧间隔`
  * `base` ：该视频计算得出的基准帧间隔，如`768 (时间基)`
  * `min` ：该视频计算得出的最小帧间隔，如`768 (时间基)`
  * `max` ：该视频计算得出的最大帧间隔，如`21504 (时间基)`
  * `num_too_small` ：该视频因太过小于 `base` 而被判定为异常帧间隔的数量，如`0 (个)`
  * `num_too_large` ：该视频因太过大于 `base` 而被判定为异常帧间隔的数量，如`237 (个)`

```Shell
# 示例输出
文件: example.mp4
时间基: 1/10880 总帧数: 4309
帧间隔: base = 768, min = 768, max = 21504, num_too_small = 0, num_too_large = 237
```

## 提示

使用full build的ffmpeg库进行编译，软件/硬件解码性能相较于Release会有显著的提升。若您没有二次分发需求，建议使用full build版本ffmpeg库自行编译。

NVIDIA硬件解码需要NVDIA GPU驱动，受NVIDIA最终用户协议约束。
