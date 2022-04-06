#include "EngineUpdater.h"

UpdateEngineJob::UpdateEngineJob(RaveAP &processor, const std::string modelFile)
    : ThreadPoolJob("UpdateEngineJob"), mProcessor(processor),
      mModelFile(modelFile) {}

UpdateEngineJob::~UpdateEngineJob() {}

bool UpdateEngineJob::waitForFadeOut(size_t waitTimeMs) {
  for (size_t i = 0; i < waitTimeMs && mProcessor.getIsMuted(); ++i) {
    Thread::sleep(1);
  }
  return (mProcessor.getIsMuted());
}

auto UpdateEngineJob::runJob() -> JobStatus {
  if (shouldExit()) {
    return JobStatus::jobNeedsRunningAgain;
  }

  mProcessor.mute();

  while (!waitForFadeOut(1)) {
    if (shouldExit()) {
      return JobStatus::jobNeedsRunningAgain;
    }
  }

  mProcessor._rave->load_model(mModelFile);
  mProcessor.updateBufferSizes();
  mProcessor.unmute();

  DBG("Job finished");

  return JobStatus::jobHasFinished;
}
