unset KMP_AFFINITY
unset KMP_BLOCKTIME
unset LD_PRELOAD
unset MALLOC_CONF
unset KMP_SETTINGS
unset USE_DEFAULT_MEMKIND
unset USE_NUMA_MEMKIND

#sync; echo 3 > /proc/sys/vm/drop_caches

export PATH=/home/lesliefang/pytorch_1_7_1/anaconda3/bin:$PATH
source /home/lesliefang/pytorch_1_7_1/anaconda3/bin/activate ssd-rn34-training
export CMAKE_PREFIX_PATH=${CONDA_PREFIX:-"$(dirname $(which conda))/../"}

export KMP_AFFINITY="verbose, granularity=fine,compact, 1, 0"
export KMP_BLOCKTIME=1
export LD_PRELOAD=$LD_PRELOAD:/home/lesliefang/pytorch_1_7_1/anaconda3/pkgs/intel-openmp-2021.2.0-h06a4308_610/lib/libiomp5.so
# export LD_PRELOAD=$LD_PRELOAD:/home/leslie/runtime/intel-extension-for-pytorch/third_party/memkind/.libs/libmemkind.so
# #export LD_PRELOAD=$LD_PRELOAD:/home/leslie/runtime/jemalloc/lib/libjemalloc.so
# export MALLOC_CONF="oversize_threshold:1,background_thread:false,metadata_thp:auto,dirty_decay_ms:9000000000,muzzy_decay_ms:9000000000"
export KMP_SETTINGS=1

numactl -C 0 -m 0 ./main
