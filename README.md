# frameIntervalCheck

English | [简体中文](README-zh_CN.md)

Check if the video has any abnormal frame intervals

## How to build?

Run command in the root dir:

```Shell
mkdir build
cmake -B build
cmake --build ./build
```

For MSVC, change `cmake --build ./build` to `cmake --build ./build --config release`.

If your system does not have windowsRub, cxxopts, ffmpeg installed, change `cmake -B build` to below.

**POWERSHELL**

```PowerShell
cmake -B build `
    -DWINDOWS_RUB_INCLUDE="{your windowsRub include dir}" `
    -DCXXOPTS_ROOT="{your cxxopts root dir}" `
    -DFFMPEG_ROOT="{your ffmpeg root dir}"
```

**SHELL**

```Shell
cmake -B build \
    -DWINDOWS_RUB_INCLUDE="{your windowsRub include dir}" \
    -DCXXOPTS_ROOT="{your cxxopts root dir}" \
    -DFFMPEG_ROOT="{your ffmpeg root dir}"
```

## How to use?

1. Open shell/cmd/...
2. Change dir to executable file in
3. Run `frameIntervalCheck -h(或 --help)` to get help information
   Simple Running e.g. : `frameIntervalCheck --path=./` wiil scan frame information for all .mp4 files in current directory

## Option List:

* [`--file`](README-zh_CN.md##--file) : Set the filename of the video to be scanned
* [`--path`](README-zh_CN.md##--path) : Set the directory for Batch Scan Mode
* [`--pattern`](README-zh_CN.md##--pattern) : Set wildcard pattern for filenames in Batch Scan Mode
* [`--container-only`](README-zh_CN.md##--container-only) : Enable Scan Container Only Mode.
* [`--decode-only`](README-zh_CN.md##--decode-only) : Enable Decode Only Mode
* [`--harddware-decode`](README-zh_CN.md##--harddware-decode) : Enable Hardware Decoding (disable Software Decoding)
* [`--deviation-rate`](README-zh_CN.md##--deviation-rate) : Set using fixed tolerance rate
* [`--IQR`](README-zh_CN.md##--IQR) : Set using Interquartile Range
* [`--MAD`](README-zh_CN.md##--MAD) : Set using Median Absolute Deviation
* [`--verbose`](README-zh_CN.md##--verbose) : Enable print details
* [`-h, --help`](README-zh_CN.md##-h,---help) : Print help information

## Meaning of Output information

* `时间基` : The basic unit of time in this video, like `1/10880 (second)`
* `总帧数` : Total number of frames in this video, like `4309 (frames)`
* `帧间隔`
  * `base` : The frame interval base calculated in this video, like `768 (时间基)`
  * `min` : The minimum frame interval calculated in this video, like `768 (时间基)`
  * `max` : The maximum frame interval calculated in this video, like `21504 (时间基)`
  * `num_too_small` : The number of frame intervals considered abnormal in this video, as they are much smaller than the `base`, such as `0`
  * `num_too_large` : The number of frame intervals considered abnormal in this video, as they are much larger than the `base`, such as `237`

```Shell
# Example Output
文件: example.mp4
时间基: 1/10880 总帧数: 4309
帧间隔: base = 768, min = 768, max = 21504, num_too_small = 0, num_too_large = 237
```

## Tips

Compiling with full-build ffmpeg library leads to a significant performance upgrade when software/hardware decoding, compared to the Release. If you do not need to redistribute, it is recommended to use the full-build ffmpeg library to compile it yourself.

NVIDIA hardware decode requires NVIDIA GPU driver, subject to NVIDIA End User License Agreement.
