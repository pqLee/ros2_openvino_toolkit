/*
 * Copyright (c) 2018 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @brief A header file with declaration for BaseInference Class
 * @file base_inference.h
 */
#ifndef DYNAMIC_VINO_LIB__INFERENCES__BASE_INFERENCE_HPP_
#define DYNAMIC_VINO_LIB__INFERENCES__BASE_INFERENCE_HPP_

#include <memory>
#include <string>

#include "dynamic_vino_lib/engines/engine.hpp"
#include "dynamic_vino_lib/slog.hpp"
#include "inference_engine.hpp"
#include "opencv2/opencv.hpp"

namespace Outputs {
class BaseOutput;
}
/**
 * @brief Load a frame into the input blob(memory).
 * @param[in] orig_image frame to be put.
 * @param[in] blob Blob that points to memory.
 * @param[in] scale_factor Scale factor for loading.
 * @param[in] batch_index Indicates the batch index for the frame.
 */
template <typename T>
void matU8ToBlob(const cv::Mat& orig_image, InferenceEngine::Blob::Ptr& blob,
                 float scale_factor = 1.0, int batch_index = 0) {
  InferenceEngine::SizeVector blob_size = blob->getTensorDesc().getDims();
  const size_t width = blob_size[3];
  const size_t height = blob_size[2];
  const size_t channels = blob_size[1];
  T* blob_data = blob->buffer().as<T*>();

  cv::Mat resized_image(orig_image);
  if (width != orig_image.size().width || height != orig_image.size().height) {
    cv::resize(orig_image, resized_image, cv::Size(width, height));
  }
  int batchOffset = batch_index * width * height * channels;

  for (size_t c = 0; c < channels; c++) {
    for (size_t h = 0; h < height; h++) {
      for (size_t w = 0; w < width; w++) {
        blob_data[batchOffset + c * width * height + h * width + w] =
            resized_image.at<cv::Vec3b>(h, w)[c] * scale_factor;
      }
    }
  }
}

namespace dynamic_vino_lib {
/**
 * @class Result
 * @brief Base class for detection result.
 */
class Result {
 public:
  friend class BaseInference;
  explicit Result(const cv::Rect& location);
  inline const cv::Rect getLocation() const { return location_; }

 private:
  cv::Rect location_;
};

/**
 * @class BaseInference
 * @brief Base class for network inference.
 */
class BaseInference {
 public:
  BaseInference();
  virtual ~BaseInference();
  /**
   * @brief load the Engine instance that contains the request for
   * running netwrok on target calculation device.
   */
  void loadEngine(std::shared_ptr<Engines::Engine> engine);
  /**
   * @brief Get the loaded Engine instance.
   * @return The loaded Engine instance.
   */
  inline const std::shared_ptr<Engines::Engine> getEngine() const {
    return engine_;
  }
  /**
   * @brief Get the number of enqueued frames to be infered.
   * @return The number of enqueued frames to be infered.
   */
  inline const int getEnqueuedNum() const { return enqueued_frames; }
  /**
   * @brief Enqueue a frame to this class.
   * The frame will be buffered but not infered yet.
   * @param[in] frame The frame to be enqueued.
   * @param[in] input_frame_loc The location of the enqueued frame with respect
   * to the frame generated by the input device.
   * @return Whether this operation is successful.
   */
  virtual bool enqueue(const cv::Mat& frame,
                       const cv::Rect& input_frame_loc) = 0;
  /**
   * @brief Start inference for all buffered frames.
   * @return Whether this operation is successful.
   */
  virtual bool submitRequest();
  virtual bool SynchronousRequest();

  virtual const void observeOutput(
      const std::shared_ptr<Outputs::BaseOutput>& output) = 0;

  /**
   * @brief This function will fetch the results of the previous inference and
   * stores the results in a result buffer array. All buffered frames will be
   * cleared.
   * @return Whether the Inference object fetches a result this time
   */
  virtual bool fetchResults();
  /**
   * @brief Get the length of the buffer result array.
   */
  virtual const int getResultsLength() const = 0;
  /**
   * @brief Get the location of result with respect
   * to the frame generated by the input device.
   * @param[in] idx The index of the result.
   */
  virtual const dynamic_vino_lib::Result* getLocationResult(int idx) const = 0;
  /**
   * @brief Get the name of the Inference instance.
   * @return The name of the Inference instance.
   */
  virtual const std::string getName() const = 0;

 protected:
  /**
    * @brief Enqueue the fram into the input blob of the target calculation
    * device. Check OpenVINO document for detailed information.
    * @return Whether this operation is successful.
    */
  template <typename T>
  bool enqueue(const cv::Mat& frame, const cv::Rect&, float scale_factor,
               int batch_index, const std::string& input_name) {
    if (enqueued_frames == max_batch_size_) {
      slog::warn << "Number of " << getName() << "input more than maximum("
                 << max_batch_size_ << ") processed by inference" << slog::endl;
      return false;
    }
    InferenceEngine::Blob::Ptr input_blob =
        engine_->getRequest()->GetBlob(input_name);
    matU8ToBlob<T>(frame, input_blob, scale_factor, batch_index);
    enqueued_frames += 1;
    return true;
  }
  /**
   * @brief Set the max batch size for one inference.
   */
  inline void setMaxBatchSize(int max_batch_size) {
    max_batch_size_ = max_batch_size;
  }
  std::vector<Result> results_;

 private:
  std::shared_ptr<Engines::Engine> engine_;
  int max_batch_size_ = 1;
  int enqueued_frames = 0;
  bool results_fetched_ = false;
};
}  // namespace dynamic_vino_lib

#endif  // DYNAMIC_VINO_LIB__INFERENCES__BASE_INFERENCE_HPP_
