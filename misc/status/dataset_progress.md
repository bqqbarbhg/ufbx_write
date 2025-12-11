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
