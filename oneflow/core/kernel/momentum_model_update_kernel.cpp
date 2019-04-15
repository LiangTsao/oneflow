#include "oneflow/core/kernel/momentum_model_update_kernel.h"
#include "oneflow/core/kernel/normal_model_update_kernel.cuh"
#include <iomanip>

namespace oneflow {

template<DeviceType device_type, typename T>
void MomentumMdUpdateKernel<device_type, T>::InitModelBlobsWithRandomSeed(
    DeviceCtx* ctx, std::mt19937* random_seed_gen,
    std::function<Blob*(const std::string&)> BnInOp2Blob) const {
  InitializerConf momentum_initializer_conf;
  momentum_initializer_conf.mutable_constant_conf()->set_value(0.0f);
  KernelUtil<device_type, T>::InitializeWithConf(ctx, momentum_initializer_conf, 0,
                                                 BnInOp2Blob("momentum"));
}

template<DeviceType device_type, typename T>
void MomentumMdUpdateKernel<device_type, T>::InitModelBlobsWithDir(
    DeviceCtx* ctx, int32_t part_id, int32_t part_num, const std::string& model_load_dir,
    std::function<Blob*(const std::string&)> BnInOp2Blob) const {
  return;
  Blob* momentum_blob = BnInOp2Blob("momentum");
  KernelUtil<device_type, T>::InitializeWithDir(
      ctx, part_id, part_num, model_load_dir, momentum_blob, "momentum",
      momentum_blob->shape().At(0), momentum_blob->shape().Count(1));
}

template<DeviceType device_type, typename T>
void MomentumMdUpdateKernel<device_type, T>::UpdateModel(
    DeviceCtx* ctx, const T* batch_instance_num_ptr, T learning_rate, T l1, T l2,
    int64_t next_model_vid, std::function<Blob*(const std::string&)> BnInOp2Blob) const {
  const Blob* model_diff_blob = BnInOp2Blob("model_diff");
  Blob* model_blob = BnInOp2Blob("model");
  Blob* momentum_blob = BnInOp2Blob("momentum");
  float beta = this->op_conf().normal_mdupdt_conf().user_conf().momentum_conf().beta();
  if (next_model_vid == 1) {
    Memset<device_type>(ctx, momentum_blob->mut_dptr<T>(), 0,
                        momentum_blob->ByteSizeOfDataContentField());
  }
  std::cout << this->op_conf().name() << " lr: " << std::fixed << std::setprecision(6)
            << learning_rate << std::endl;
  MomentumMdUpdateKernelUtil<device_type, T>::UpdateModel(
      ctx, model_blob->shape().elem_cnt(), batch_instance_num_ptr, static_cast<T>(beta),
      learning_rate, l1, l2, model_diff_blob->dptr<T>(), model_blob->mut_dptr<T>(),
      momentum_blob->mut_dptr<T>());
}

template<typename T>
class MomentumMdUpdateKernelUtil<DeviceType::kCPU, T> final {
 public:
  static void UpdateModel(DeviceCtx*, int64_t n, const T* batch_instance_num_ptr, T beta,
                          T learning_rate, T l1, T l2, const T* model_diff, T* model, T* momentum) {
    for (int64_t i = 0; i != n; ++i) {
      int32_t batch_size = static_cast<int32_t>(*batch_instance_num_ptr);
      T reg_diff =
          model_diff[i] + batch_size * (l1 * ((model[i] >= 0) - (model[i] <= 0)) + l2 * model[i]);
      momentum[i] = beta * momentum[i] - reg_diff;
      model[i] = model[i] + learning_rate / batch_size * momentum[i];
    }
  }
};

DEFINE_MDUPDT_KERNEL_CREATOR(Momentum);

}  // namespace oneflow
