#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <math.h>

RaveAP::RaveAP()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      _avts(*this, nullptr, Identifier("RAVEValueTree"),
            createParameterLayout()),
      _loadedModelName(""), _computeThread(nullptr), _dryWetMixerEffect(BUFFER_LENGTH)
#endif
{
  _inBuffer = std::make_unique<circular_buffer<float, float>[]>(1);
  _outBuffer = std::make_unique<circular_buffer<float, float>[]>(2);
  _inModel.push_back(std::make_unique<float[]>(BUFFER_LENGTH));
  _outModel.push_back(std::make_unique<float[]>(BUFFER_LENGTH));
  _outModel.push_back(std::make_unique<float[]>(BUFFER_LENGTH));

  _inputGainValue = _avts.getRawParameterValue(rave_parameters::input_gain);
  _thresholdValue = _avts.getRawParameterValue(rave_parameters::input_thresh);
  _ratioValue = _avts.getRawParameterValue(rave_parameters::input_ratio);
  _channelMode = _avts.getRawParameterValue(rave_parameters::channel_mode);
  _latentJitterValue =
      _avts.getRawParameterValue(rave_parameters::latent_jitter);
  _widthValue = _avts.getRawParameterValue(rave_parameters::output_width);
  _outputGainValue = _avts.getRawParameterValue(rave_parameters::output_gain);
  _dryWetValue = _avts.getRawParameterValue(rave_parameters::output_drywet);
  _limitValue = _avts.getRawParameterValue(rave_parameters::output_limit);
  _usePrior = _avts.getRawParameterValue(rave_parameters::use_prior);
  _latentScale = new std::array<std::atomic<float> *, AVAILABLE_DIMS>;
  _latentBias = new std::array<std::atomic<float> *, AVAILABLE_DIMS>;
  for (unsigned long i = 0; i < AVAILABLE_DIMS; i++) {
    (*_latentScale)[i] = _avts.getRawParameterValue(
        rave_parameters::latent_scale + String("_") + std::to_string(i));
    (*_latentBias)[i] = _avts.getRawParameterValue(
        rave_parameters::latent_bias + String("_") + std::to_string(i));
  }
  _latencyMode = _avts.getRawParameterValue(rave_parameters::latency_mode);
  _priorTemperature = _avts.getRawParameterValue(rave_parameters::prior_temperature);
  _engineThreadPool = std::make_unique<ThreadPool>(1);
  _rave.reset(new RAVE());

  _avts.addParameterListener(rave_parameters::input_gain, this);
  _avts.addParameterListener(rave_parameters::input_thresh, this);
  _avts.addParameterListener(rave_parameters::input_ratio, this);
  _avts.addParameterListener(rave_parameters::output_gain, this);
  _avts.addParameterListener(rave_parameters::output_limit, this);
  _avts.addParameterListener(rave_parameters::output_drywet, this);
  _avts.addParameterListener(rave_parameters::latency_mode, this);
  _dryWetMixerEffect.setMixingRule(juce::dsp::DryWetMixingRule::balanced);
  _editorReady = false;
}

RaveAP::~RaveAP() {
  if (_computeThread)
    _computeThread->join();
}

void RaveAP::prepareToPlay(double sampleRate, int samplesPerBlock) {
  _sampleRate = sampleRate;
  _inBuffer[0].initialize(BUFFER_LENGTH);
  _outBuffer[0].initialize(BUFFER_LENGTH);
  _outBuffer[1].initialize(BUFFER_LENGTH);
  _smoothedFadeInOut.reset(sampleRate, 0.2);
  juce::dsp::ProcessSpec specs = {
      sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2};
  _inputGainEffect.prepare(specs);
  _compressorEffect.prepare(specs);
  _outputGainEffect.prepare(specs);
  _limiterEffect.prepare(specs);
  _limiterEffect.setThreshold(-1.f);
  _dryWetMixerEffect.prepare(specs);
  _inputGainEffect.setGainDecibels(_inputGainValue->load());
  _compressorEffect.setRatio(_ratioValue->load());
  _compressorEffect.setThreshold(_thresholdValue->load());
  _outputGainEffect.setGainDecibels(_outputGainValue->load());
  _dryWetMixerEffect.setWetMixProportion(_dryWetValue->load() / 100.f);
  setLatencySamples(pow(2, *_latencyMode));
}

void RaveAP::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
  _limiterEffect.reset();
  _inputGainEffect.reset();
  _outputGainEffect.reset();
  _compressorEffect.reset();
  _dryWetMixerEffect.reset();
}

juce::String valueToTextFunction(float value) {
    return juce::String(value);
}

float textToValueFunction (const String &value) {
    return value.getFloatValue();
}



AudioProcessorValueTreeState::ParameterLayout RaveAP::createParameterLayout() {
  std::vector<std::unique_ptr<RangedAudioParameter>> params;
  params.push_back(std::make_unique<AudioParameterFloat>(
      rave_parameters::input_gain, rave_parameters::input_gain,
      rave_ranges::gainRange, 0.f));
  params.push_back(std::make_unique<AudioParameterFloat>(
      rave_parameters::input_thresh, rave_parameters::input_thresh, -60.f, 0.f,
      0.f));
  params.push_back(std::make_unique<AudioParameterFloat>(
      rave_parameters::input_ratio, rave_parameters::input_ratio, 1.f, 10.f,
      1.f));
  params.push_back(std::make_unique<AudioParameterInt>(
      rave_parameters::channel_mode, rave_parameters::channel_mode, 1, 3, 1));
  params.push_back(std::make_unique<AudioParameterFloat>(
      rave_parameters::latent_jitter, rave_parameters::latent_jitter, 0.f, 3.f,
      0.f));
  params.push_back(std::make_unique<AudioParameterFloat>(
      rave_parameters::output_width, rave_parameters::output_width, 0.f, 200.f,
      100.f));
  params.push_back(std::make_unique<AudioParameterFloat>(
      rave_parameters::output_gain, rave_parameters::output_gain,
      rave_ranges::gainRange, 0.f));
  params.push_back(std::make_unique<AudioParameterFloat>(
      rave_parameters::output_drywet, rave_parameters::output_drywet, 0.f,
      100.f, 100.f));
  params.push_back(std::make_unique<AudioParameterBool>(
      rave_parameters::output_limit, rave_parameters::output_limit, true));
  params.push_back(std::make_unique<NAAudioParameterInt>(
      rave_parameters::latency_mode, rave_parameters::latency_mode, 9, 15, 13));
  params.push_back(std::make_unique<AudioParameterBool>(
      rave_parameters::use_prior, rave_parameters::use_prior, false));
  params.push_back(std::make_unique<AudioParameterFloat>(
      rave_parameters::prior_temperature, rave_parameters::prior_temperature,
      0.f, 5.f, 1.f));

  String current_name;
  for (size_t i = 0; i < AVAILABLE_DIMS; i++) {
    current_name =
        rave_parameters::latent_scale + String("_") + std::to_string(i);
    params.push_back(std::make_unique<AudioParameterFloat>(
        current_name, current_name, rave_ranges::latentScaleRange, 1.0));
    current_name =
        rave_parameters::latent_bias + String("_") + std::to_string(i);
    params.push_back(std::make_unique<AudioParameterFloat>(
        current_name, current_name, rave_ranges::latentBiasRange, 0.0));
  }
  return {params.begin(), params.end()};
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  // This creates new instances of the plugin..
  return new RaveAP();
}
