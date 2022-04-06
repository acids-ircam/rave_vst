// This file contains all the functions needed by JUCE to interact with the DAWs
#include "PluginEditor.h"
#include "PluginProcessor.h"

const juce::String RaveAP::getName() const { return JucePlugin_Name; }

bool RaveAP::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool RaveAP::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool RaveAP::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double RaveAP::getTailLengthSeconds() const { return 0.0; }

int RaveAP::getNumPrograms() {
  // NB: some hosts don't cope very well if you tell them there are 0
  // programs, so this should be at least 1, even if you're not really
  // implementing programs.
  return 1;
}

int RaveAP::getCurrentProgram() { return 0; }

void RaveAP::setCurrentProgram(int /*index*/) {}

const juce::String RaveAP::getProgramName(int /*index*/) { return {}; }

void RaveAP::changeProgramName(int /*index*/,
                               const juce::String & /*newName*/) {}

bool RaveAP::hasEditor() const {
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *RaveAP::createEditor() {
  return new RaveAPEditor(*this, _avts);
}

void RaveAP::getStateInformation(juce::MemoryBlock &destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
  auto state = _avts.copyState();
  std::unique_ptr<XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void RaveAP::setStateInformation(const void *data, int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
  std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName(_avts.state.getType()))
      _avts.replaceState(ValueTree::fromXml(*xmlState));
}

void RaveAP::mute() { _fadeScheduler.store(muting::mute); }

void RaveAP::unmute() { _fadeScheduler.store(muting::unmute); }

const bool RaveAP::getIsMuted() { return _isMuted.load(); }

#ifndef JucePlugin_PreferredChannelConfigurations
bool RaveAP::isBusesLayoutSupported(const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif
