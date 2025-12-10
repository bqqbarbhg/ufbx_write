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

```
```
