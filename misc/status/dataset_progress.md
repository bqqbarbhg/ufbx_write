# Dataset progress

This file contains a journal of the dataset roundtrip test progress.

## Setup

```
$ ufbx_write
  python misc/check_dataset.py
    --exe <roundtrip.exe>
    --root <fbx-dataset-path>
    --result <fbx-roundtrip-path>

$ ufbx
  python misc/check_dataset.py
    --exe <check_fbx.exe>
    --include-recent
    --root <fbx-dataset-path>
    --override-root <fbx-roundtrip-path>
```

## Results

#### Baseline

```
1088/4454 files passed (159/410 test cases)
```

#### Skin deformers

Initial skin deformer support [(commit)](https://github.com/bqqbarbhg/ufbx_write/commit/9e3451b08c46a86edbfeed31162da3a8e940060a)

```
1704/4454 files passed (193/410 test cases)
```

#### Animation fixes

Fixed animation tangents, added extrapolation support

```
2036/4454 files passed (221/410 test cases)
```

#### Blend deformers

Initial blend deformer support

```
2074/4454 files passed (225/410 test cases)
```

#### Time mode

Fixed time mode so that the scenes have correct FPS

```
3338/4454 files passed (335/410 test cases)
```

#### Bones / inherit type

Added bones to roundtrip, fixed inherit type handling

```
3870/4454 files passed (365/410 test cases)
```

#### Initial materials

Implemented material support

```
3942/4454 files passed (376/410 test cases)
```

#### Fixed index passthrough

Pass out-of-bounds indices as-is in roundtrip

```
4234/4454 files passed (391/410 test cases)
```

#### Base model suffix

Load base model with suffix in roundtrip

```
4244/4454 files passed (393/410 test cases)
```
