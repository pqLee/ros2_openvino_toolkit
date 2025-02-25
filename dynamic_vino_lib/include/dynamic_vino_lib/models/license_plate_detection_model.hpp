// Copyright (c) 2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @brief A header file with declaration for LicensePlateDetectionModel Class
 * @file vehicle_attribs_detection_model.h
 */
#ifndef DYNAMIC_VINO_LIB__MODELS__LICENSE_PLATE_DETECTION_MODEL_HPP_
#define DYNAMIC_VINO_LIB__MODELS__LICENSE_PLATE_DETECTION_MODEL_HPP_
#include <string>
#include "dynamic_vino_lib/models/base_model.hpp"
namespace Models
{
/**
 * @class LicensePlateDetectionModel
 * @brief This class generates the license plate detection model.
 */
class LicensePlateDetectionModel : public BaseModel
{
public:
  LicensePlateDetectionModel(const std::string & model_loc, int batch_size = 1);
  inline const std::string getInputName() {return input_;}
  inline const std::string getSeqInputName() {return seq_input_;}
  inline const std::string getOutputName() {return output_;}
  inline int getMaxSequenceSize() const {return max_sequence_size_;}
  /**
   * @brief Get the name of this detection model.
   * @return Name of the model.
   */
  const std::string getModelCategory() const override;

protected:
  //void checkLayerProperty(const InferenceEngine::CNNNetReader::Ptr &) override;
  //void setLayerProperty(InferenceEngine::CNNNetReader::Ptr) override;
  bool updateLayerProperty(InferenceEngine::CNNNetwork&) override;
  // up to 88 items per license plate, ended with "-1"
  const int max_sequence_size_ = 88;
  std::string input_;
  std::string seq_input_;
  std::string output_;
};
}  // namespace Models
#endif  // DYNAMIC_VINO_LIB__MODELS__LICENSE_PLATE_DETECTION_MODEL_HPP_
