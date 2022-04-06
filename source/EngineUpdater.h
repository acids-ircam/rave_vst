#pragma once
#include "PluginProcessor.h"
#include <JuceHeader.h>

class RaveAP; // forward declaration

class UpdateEngineJob : public juce::ThreadPoolJob {
public:
  explicit UpdateEngineJob(RaveAP &processor, const std::string modelPath);
  virtual ~UpdateEngineJob();
  virtual auto runJob() -> JobStatus;
  bool waitForFadeOut(size_t waitTimeMs);

private:
  RaveAP &mProcessor;
  const std::string mModelFile;
  // Prevent uncontrolled usage
  UpdateEngineJob(const UpdateEngineJob &);
  UpdateEngineJob &operator=(const UpdateEngineJob &);
};
