# 选项详情:

## `--file`

与`--path`冲突。

`--file [FILENAME]`
`[FILENAME]`为待扫描视频文件名。
若文件不在当前目录下，请输入带路径的文件名。

## `--path`

与`--file`冲突。

`--path [DIRECTORY]`
`[DIRECTORY]`为待扫描视频目录。
启用批量扫描模式，并设置待扫描目录。

## `--pattern`

依赖`--path`。

`--pattern [PATTERN]`
`[PATTERN]`为批量扫描文件名通配符模式，默认为`*.mp4`。

## `--container-only`

与`--decode-only`冲突。

仅通过视频封装获取帧信息，即使无法获取也不会进行解码。

## `--decode-only`

与`--container-only`冲突。

仅通过视频解码获取帧信息。

## `--hardware-decode`

启用硬件解码（将会关闭软件解码）。

## `--deviation-rate`

与`--IQR`、`--MAD`冲突。

`--deviation-rate`
显式指定使用固定容忍率计算基值。

`--deviation-rate [FLOAT_NUM]`
`[FLOAT_NUM]`为容忍率，默认为`0.2`。
容忍率越大，判定正常的数据范围越大。
显式指定使用固定容忍率计算基值，同时设置容忍率。

## `--IQR`

与`--deviation-rate`、`--MAD`冲突。

`--IQR`
指定使用四分位距算法计算容忍偏差。

`--IQR [FLOAT_NUM]`
`[FLOAT_NUM]`为系数，默认为`1.5`。
容忍系数越大，判定正常的数据范围越大。
指定使用四分位距算法计算容忍偏差，同时设置容忍系数。

## `--MAD`

与`--deviation-rate`、`--IQR`冲突。

`--MAD`
指定使用中位数绝对偏差算法计算容忍偏差。

`--MAD [FLOAT_NUM]`
`[FLOAT_NUM]`为系数，默认为`3.0`。
容忍系数越大，判定正常的数据范围越大。
指定使用中位数绝对偏差算法计算容忍偏差，同时设置容忍系数。

## `--verbose`

开启详细信息显示。

## `-h, --help`

打印帮助信息。
