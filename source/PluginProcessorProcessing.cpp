#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <algorithm>
#include <math.h>

#define DEBUG_PERFORM 0

void RaveAP::modelPerform() {
  if (_rave.get() && !_isMuted.load()) {
    c10::InferenceMode guard(true);
    // encode
    int input_size = static_cast<int>(pow(2, *_latencyMode));

    at::Tensor latent_traj;
    at::Tensor latent_traj_mean;

#if DEBUG_PERFORM
    std::cout << "exp: " << *_latencyMode << " value: " << input_size << '\n';
    std::cout << "has prior : " << _rave->hasPrior()
              << "; use prior : " << *_usePrior << std::endl;
    std::cout << "temperature : " << *_priorTemperature << std::endl;
#endif

    if (_rave->hasPrior() && *_usePrior) {
      auto n_trajs = pow(2, *_latencyMode) / _rave->getModelRatio();
      latent_traj = _rave->sample_prior((int)n_trajs, *_priorTemperature);
      latent_traj_mean = latent_traj;
    } else {
      int64_t sizes = {input_size};
      at::Tensor frame = torch::from_blob(_inModel[0].get(), sizes);
      frame = torch::reshape(frame, {1, 1, input_size});
#if DEBUG_PERFORM
      std::cout << "Current input size : " << frame.sizes() << std::endl;
#endif DEBUG_PERFORM
      std::vector<torch::Tensor> latent_probs = _rave->encode_amortized(frame);
      latent_traj_mean = latent_probs[0];
      at::Tensor latent_traj_std = latent_probs[1];

#if DEBUG_PERFORM
      std::cout << "mean shape" << latent_traj_mean.sizes() << std::endl;
      std::cout << "std shape" << latent_traj_std.sizes() << std::endl;
#endif

      latent_traj = latent_traj_mean +
                    latent_traj_std * torch::randn_like(latent_traj_mean);
    }

#if DEBUG_PERFORM
    std::cout << "latent traj shape" << latent_traj.sizes() << std::endl;
#endif

    // Latent modifications
    // apply scale and bias
    int64_t n_dimensions =
        std::min((int)latent_traj.size(1), (int)AVAILABLE_DIMS);
    for (int i = 0; i < n_dimensions; i++) {
      // The assert and casting here is needed as I got a:
      // warning: conversion to ‘std::array<std::atomic<float>*,
      // 8>::size_type’ {aka ‘long unsigned int’} from ‘int’ may change the
      // sign of the result [-Wsign-conversion]
      // Whatever AVAILABLE_DIMS type I defined
      assert(i >= 0);
      auto i2 = (long unsigned int)i;
      float scale = _latentScale->at(i2)->load();
      float bias = _latentBias->at(i2)->load();
      latent_traj.index_put_({0, i},
                             (latent_traj.index({0, i}) * scale + bias));
      latent_traj_mean.index_put_(
          {0, i}, (latent_traj_mean.index({0, i}) * scale + bias));
    }
    _rave->writeLatentBuffer(latent_traj_mean);

#if DEBUG_PERFORM
    std::cout << "scale & bias applied" << std::endl;
#endif

    // adding latent jitter on meaningful dimensions
    float jitter_amount = _latentJitterValue->load();
    latent_traj = latent_traj + jitter_amount * torch::randn_like(latent_traj);

#if DEBUG_PERFORM
    std::cout << "jitter applied" << std::endl;
#endif

    // filling missing dimensions with width parameter
    torch::Tensor latent_trajL = latent_traj,
                  latent_trajR = latent_traj.clone();
    int missing_dims = _rave->getFullLatentDimensions() - latent_trajL.size(1);
    float width = _widthValue->load() / 100.f;
    at::Tensor latent_noiseL =
        torch::randn({1, missing_dims, latent_trajL.size(2)});
    at::Tensor latent_noiseR =
        (1 - width) * latent_noiseL +
        width * torch::randn({1, missing_dims, latent_trajL.size(2)});

#if DEBUG_PERFORM
    std::cout << "after width : " << latent_noiseL.sizes() << ";"
              << latent_trajL.sizes() << std::endl;
#endif

    latent_trajL = torch::cat({latent_trajL, latent_noiseL}, 1);
    latent_trajR = torch::cat({latent_trajR, latent_noiseR}, 1);

#if DEBUG_PERFORM
    std::cout << "latent processed" << std::endl;
#endif

    // Decode
    latent_traj = torch::cat({latent_trajL, latent_trajR}, 0);
    at::Tensor out = _rave->decode(latent_traj);
    // On windows, I don't get why, but the two first dims are swapped (compared
    // to macOS / UNIX) with the same torch version
    if (out.sizes()[0] == 2) {
      out = out.transpose(0, 1);
    }
    at::Tensor outL = out.index({0, 0, at::indexing::Slice()});
    at::Tensor outR = out.index({0, 1, at::indexing::Slice()});

#if DEBUG_PERFORM
    std::cout << "latent decoded" << std::endl;
#endif

    float *outputDataPtrL, *outputDataPtrR;
    outputDataPtrL = outL.data_ptr<float>();
    outputDataPtrR = outR.data_ptr<float>();

    // Write in buffers
    assert(input_size >= 0);
    for (size_t i = 0; i < (size_t)input_size; i++) {
      _outModel[0][i] = outputDataPtrL[i];
      _outModel[1][i] = outputDataPtrR[i];
    }
    if (_smoothedFadeInOut.getCurrentValue() < EPSILON) {
      _isMuted.store(true);
    }
  }
}

void modelPerform_callback(RaveAP *ap) { ap->modelPerform(); }

void RaveAP::processBlock(juce::AudioBuffer<float> &buffer,
                          juce::MidiBuffer & /*midiMessages*/) {
  juce::ScopedNoDenormals noDenormals;
  const int nSamples = buffer.getNumSamples();
  const int nChannels = buffer.getNumChannels();

  // mute if pause
  AudioPlayHead *playHead = this->getPlayHead();
  if (playHead != nullptr) {
    std::cout << "has playhead! " << std::endl;
    AudioPlayHead::CurrentPositionInfo info;
    bool hasDawInformation = playHead->getCurrentPosition(info);
    if (hasDawInformation) {
      bool isPlaying = info.isPlaying;
      _plays = isPlaying;
      std::cout << "plays?  " << isPlaying << std::endl;
      if (isPlaying && _isMuted.load()) {
        unmute();
      } else if (!isPlaying && !_isMuted.load()) {
        mute();
        _inBuffer.get()->reset();
        _outBuffer.get()->reset();
      }
    }
  }

  // fade parameters
  const muting muteConfig = _fadeScheduler.load();
  if (muteConfig == muting::mute) {
    _smoothedFadeInOut.setTargetValue(0.f);
  } else if (muteConfig == muting::unmute) {
    _smoothedFadeInOut.setTargetValue(1.f);
    _isMuted.store(false);
  }

  juce::dsp::AudioBlock<float> ab(buffer);
  juce::dsp::ProcessContextReplacing<float> context(ab);
  _compressorEffect.process(context);
  _inputGainEffect.process(context);
  _dryWetMixerEffect.pushDrySamples(ab);

  // Compute input RMS for the GUI
  _inputAmplitudeL = buffer.getRMSLevel(0, 0, nSamples);
  _inputAmplitudeR = buffer.getRMSLevel(1, 0, nSamples);

  // process input effects
  float *channelL = nullptr, *channelR = nullptr;
  if (nChannels == 1) {
    channelL = buffer.getWritePointer(0);
    channelR = channelL;
  } else {
    channelL = buffer.getWritePointer(0);
    channelR = buffer.getWritePointer(1);
  }

  juce::String channelMode =
      channel_modes[static_cast<int>(_channelMode->load()) - 1];
  if (channelMode == "L") {
    _inBuffer[0].put(channelL, nSamples);
  } else if (channelMode == "R") {
    _inBuffer[0].put(channelR, nSamples);
  } else if (channelMode == "L + R") {
    FloatVectorOperations::add(channelL, channelR, nSamples);
    FloatVectorOperations::multiply(channelL, 0.5f, nSamples);
    _inBuffer[0].put(channelL, nSamples);
  }

  // create processing thread
  int currentRefreshRate = pow(2, *_latencyMode);
  if (_inBuffer[0].len() >= currentRefreshRate) {
    if (_computeThread) {

#if DEBUG_PERFORM
      std::cout << "joining..." << std::endl;
#endif

      _computeThread->join();
    }
    _inBuffer[0].get(_inModel[0].get(), currentRefreshRate);
    _outBuffer[0].put(_outModel[0].get(), currentRefreshRate);
    _outBuffer[1].put(_outModel[1].get(), currentRefreshRate);
    _computeThread = std::make_unique<std::thread>(modelPerform_callback, this);
  }

  AudioBuffer<float> out_buffer(2, nSamples);
  juce::dsp::AudioBlock<float> out_ab(out_buffer);
  juce::dsp::ProcessContextReplacing<float> out_context(out_ab);
  if (_outBuffer[0].len() >= nSamples) {
    _outBuffer[0].get(out_buffer.getWritePointer(0), nSamples);
    _outBuffer[1].get(out_buffer.getWritePointer(1), nSamples);
  } else {
    out_buffer.clear();
  }

#if DEBUG
  std::cout << "buffer out : " << out_buffer.getMagnitude(0, nSamples)
            << std::endl;
#endif

  _outputGainEffect.process(out_context);
  bool is_limiting = static_cast<bool>((*_limitValue).load());
  if (is_limiting)
    _limiterEffect.process(out_context);
  _dryWetMixerEffect.mixWetSamples(out_buffer);
  buffer.copyFrom(0, 0, out_buffer, 0, 0, nSamples);
  if (nChannels == 2)
    buffer.copyFrom(1, 0, out_buffer, 1, 0, nSamples);
#if DEBUG_PERFORM
  std::cout << "sortie : " << buffer.getMagnitude(0, nSamples) << std::endl;
#endif
}

void RaveAP::parameterChanged(const String &parameterID, float newValue) {
  if (parameterID == rave_parameters::input_gain) {
    _inputGainEffect.setGainDecibels(newValue);
  } else if (parameterID == rave_parameters::input_ratio) {
    _compressorEffect.setRatio(newValue);
  } else if (parameterID == rave_parameters::input_thresh) {
    _compressorEffect.setThreshold(newValue);
  } else if (parameterID == rave_parameters::output_gain) {
    _outputGainEffect.setGainDecibels(newValue);
  } else if (parameterID == rave_parameters::output_drywet) {
    _dryWetMixerEffect.setWetMixProportion(newValue / 100.f);
  } else if (parameterID == rave_parameters::latency_mode) {
    auto latency_samples = pow(2, *_latencyMode);
    std::cout << "[ ] - latency has changed to " << latency_samples
              << std::endl;
    setLatencySamples(latency_samples);
    _dryWetMixerEffect.setWetLatency(latency_samples);
  }
}

void RaveAP::updateBufferSizes() {
  auto validBufferSizes = _rave->getValidBufferSizes();
  float a = validBufferSizes.getStart();
  float b = validBufferSizes.getEnd();

  if (*_latencyMode < a) {
    std::cout << "to low; setting rate to : " << static_cast<int>(log2(a))
              << std::endl;
    *_latencyMode = static_cast<int>(log2(a));
  } else if (*_latencyMode > b) {
    std::cout << "to high; setting rate to : " << static_cast<int>(log2(b))
              << std::endl;
    *_latencyMode = static_cast<int>(log2(b));
  }
}

void RaveAP::updateEngine(const std::string modelFile) {
  if (modelFile == _loadedModelName)
    return;
  _loadedModelName = modelFile;
  juce::ScopedLock irCalculationlock(_engineUpdateMutex);
  if (_engineThreadPool) {
    _engineThreadPool->removeAllJobs(true, 100);
  }

  _engineThreadPool->addJob(new UpdateEngineJob(*this, modelFile), true);
}
