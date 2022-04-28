#pragma once

#include "PluginProcessor.h"
#include "ui/FoldablePanel.h"
#include "ui/GUI_GLOBALS.h"
#include "ui/Header.h"
#include "ui/ModelExplorer.h"
#include "ui/ModelPanel.h"
#include "ui/MyLookAndFeel.h"
#include <sys/stat.h>

#include <JuceHeader.h>

using namespace juce;

class RaveAPEditor : public juce::AudioProcessorEditor,
                     // public juce::Timer,
                     public juce::ChangeListener {
public:
  RaveAPEditor(RaveAP &, AudioProcessorValueTreeState &);
  ~RaveAPEditor() override;

  void paint(juce::Graphics &) override;
  void resized() override;
  void log(String str);
  void changeListenerCallback(ChangeBroadcaster *source) override;

  /*
  protected:
    void timerCallback() override;
  */

private:
  void getAvailableModelsFromAPI();
  void downloadModelFromAPI();
  String getCleanedString(String str);
  void detectAvailableModels();
  void importModel();

  File _modelsDirPath;
  std::unique_ptr<FileChooser> _fc;

  LightLookAndFeel _lightLookAndFeel;
  DarkLookAndFeel _darkLookAndFeel;
  RaveAP &audioProcessor;
  AudioProcessorValueTreeState &_avts;

  // Main window
  ModelPanel _modelPanel;
  Header _header;
  FoldablePanel _foldablePanel;
  // Model Manager window
  ModelExplorer _modelExplorer;
  Label _console;

  Image _bgFull;

  StringArray _availableModelsPaths;
  StringArray _availableModels;

  juce::var _parsedJson;

  static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                              void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
  }

  static size_t writeToFileCallback(void *ptr, size_t size, size_t nmemb,
                                    FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RaveAPEditor)
};
