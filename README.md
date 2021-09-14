## How to use threadpoll with pytorch
1. Build Pytorch from source
```
eda2ddb5b06dce13bafd2a745e4634802e4640ef
```

2. Build this test
```
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=`python -c 'import torch;print(torch.utils.cmake_prefix_path)'`
make
```

## Run with 2 threads
```
LD_PRELOAD=$LD_PRELOAD:/home/lesliefang/pytorch_1_7_1/anaconda3/pkgs/intel-openmp-2021.2.0-h06a4308_610/lib/libiomp5.so ./main
```
