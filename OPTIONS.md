# Option Details:

## `--file`

In conflict with `--path`.

`--file [FILENAME]`
`[FILENAME]` is the filename of the video to be scanned.
If the file is not in current directory, please enter filename with path.

## `--path`

In conflict with `--file`.

`--path [DIRECTORY]`
`[DIRECTORY]` is the directory of the videos to be scanned.
Enable the Batch Scan Mode, and set the directory to be scanned.

## `--pattern`

Depends on `--path`.

`--pattern [PATTERN]`
`[PATTERN]` is the wildcard pattern for filenames in Batch Scan Mode, defaulting to `*.mp4`.

## `--container-only`

In conflict with `--decode-only`.

Only obtain frame information through video container.It will not decode even if obtaining fails.

## `--decode-only`

In conflict with `--container-only`.

Only obtain frame information through decoding video.

## `--hardware-decode`

Enable Hardware Decoding (it will disable Software Decoding).

## `--deviation-rate`

In conflict with `--IQR`, `--MAD`.

`--deviation-rate`
Explicitly specify using a fixed tolerance rate to calculate the base.

`--deviation-rate [FLOAT_NUM]`
`[FLOAT_NUM]` is tolerance rate, defaulting to `0.2`.
The higher the tolerance rate, the wider the range of data considered normal.
Explicitly specify using a fixed tolerance rate to calculate the base, and set the tolerance rate at the same time.

## `--IQR`

In conflict with `--deviation-rate`, `--MAD`.

`--IQR`
Specify using Interquartile Range to calculate the tolerance deviation.

`--IQR [FLOAT_NUM]`
`[FLOAT_NUM]` is coefficient, defaulting to `1.5`.
The higher the tolerance coefficient, the wider the range of data considered normal.
Specify using Interquartile Range to calculate the tolerance deviation, and set the tolerance coefficient at the same time.

## `--MAD`

In conflict with `--deviation-rate`, `--IQR`.

`--MAD`
Specify using Median Absolute Deviation to calculate the tolerance deviation.

`--MAD [FLOAT_NUM]`
`[FLOAT_NUM]` is coefficient, defaulting to `3.0`.
The higher the tolerance coefficient, the wider the range of data considered normal.
Specify using the Median Absolute Deviation to calculate the tolerance deviation, and set the tolerance coefficient at the same time.

## `--verbose`

Enable print details.

## `-h, --help`

Print help information.
