#ifndef ONEFLOW_CORE_OPERATOR_YOLO_BOX_DIFF_OP_H_
#define ONEFLOW_CORE_OPERATOR_YOLO_BOX_DIFF_OP_H_

#include "oneflow/core/operator/operator.h"

namespace oneflow {

class YoloBoxDiffOp final : public Operator {
 public:
  OF_DISALLOW_COPY_AND_MOVE(YoloBoxDiffOp);
  YoloBoxDiffOp() = default;
  ~YoloBoxDiffOp() = default;

  void InitFromOpConf() override;
  const PbMessage& GetCustomizedConf() const override;

  bool NeedOutBlobWhenBackward() const override { return true; }

  void InferBlobDescs(std::function<BlobDesc*(const std::string&)> GetBlobDesc4BnInOp,
                      const ParallelContext* parallel_ctx) const override;

 private:
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_OPERATOR_YOLO_BOX_DIFF_OP_H_
