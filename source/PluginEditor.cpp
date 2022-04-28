#include "PluginEditor.h"
#include "PluginProcessor.h"

RaveAPEditor::RaveAPEditor(RaveAP &p, AudioProcessorValueTreeState &vts)
    : AudioProcessorEditor(&p), ChangeListener(), _lightLookAndFeel(),
      _darkLookAndFeel(), audioProcessor(p), _avts(vts), _foldablePanel(p),
      _bgFull(ImageCache::getFromMemory(BinaryData::bg_full_png,
                                        BinaryData::bg_full_pngSize)) {

  String path =
      juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
          .getFullPathName();

  if (SystemStats::getOperatingSystemType() ==
      SystemStats::OperatingSystemType::MacOSX)
    path += String("/Application Support");
  path += String("/ACIDS/RAVE/");

  std::cout << path << '\n';
  _modelsDirPath = File(path);
  if (_modelsDirPath.isDirectory() == false) {
    _modelsDirPath.createDirectory();
  }

  getAvailableModelsFromAPI();

  _header.setLookAndFeel(&_darkLookAndFeel);
  _modelPanel.setLookAndFeel(&_darkLookAndFeel);
  _modelExplorer.setLookAndFeel(&_lightLookAndFeel);
  _foldablePanel.setLookAndFeel(&_lightLookAndFeel);

  _modelExplorer._downloadButton.onClick = [this]() { downloadModelFromAPI(); };
  _modelExplorer._importButton.onClick = [this]() { importModel(); };

  _modelPanel.setSampleRate(p.getSampleRate());

  detectAvailableModels();
  // Model manager button stuff
  _header._modelComboBox.onChange = [this]() {
    String modelPath =
        _availableModelsPaths[_header._modelComboBox.indexOfItemId(
            _header._modelComboBox.getSelectedId())];
    audioProcessor.updateEngine(modelPath.toStdString());
  };

  _header.connectVTS(vts);
  _modelPanel.connectVTS(vts);
  _foldablePanel.connectVTS(vts);

  // link to model
  p._rave->addChangeListener(this);
  _modelPanel.setModel(p._rave.get());

  // Model manager button stuff
  _header._modelManagerButton.onClick = [this]() {
    if (_header._modelManagerButton.getToggleState()) {
      _header._modelManagerButton.setButtonText("Play");
      _modelPanel.setVisible(false);
      _foldablePanel.setVisible(false);
      _modelExplorer.setVisible(true);
    } else {
      // Go into play mode
      _header._modelManagerButton.setButtonText("Model Explorer");
      _modelPanel.setVisible(true);
      _foldablePanel.setVisible(true);
      _modelExplorer.setVisible(false);
    }
  };

  addAndMakeVisible(_header);
  addAndMakeVisible(_modelPanel);
  addAndMakeVisible(_foldablePanel);
  addAndMakeVisible(_console);
  addChildComponent(_modelExplorer);

  setResizable(false, false);
  getConstrainer()->setMinimumSize(996, 560);
  setSize(996, 560);
  // startTimer(100.);
}

RaveAPEditor::~RaveAPEditor() {}

void RaveAPEditor::importModel() {
  _fc.reset(new FileChooser(
      "Choose your model file",
      File::getSpecialLocation(File::SpecialLocationType::userHomeDirectory),
      "*.ts", true));

  _fc->launchAsync(
      FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
      [this](const FileChooser &chooser) {
        File sourceFile;
        auto results = chooser.getURLResults();

        for (auto result : results) {
          if (result.isLocalFile()) {
            sourceFile = result.getLocalFile();
          } else {
            return;
          }
        }
        if (sourceFile.getFileExtension() == ".ts" &&
            sourceFile.getSize() > 0) {
          sourceFile.copyFileTo(_modelsDirPath.getNonexistentChildFile(
              sourceFile.getFileName(), ".ts"));
          detectAvailableModels();
        }
      });
}

/*
void RaveAPEditor::timerCallback() {
  //_console.setText(String(audioProcessor.getLatencySamples()),
juce::dontSendNotification);
}
*/

void RaveAPEditor::resized() {
  // Child components should not handle margins, do it here
  // Setup the header first
  auto b_area = getLocalBounds().reduced(UI_MARGIN_SIZE);
  _header.setBounds(b_area.removeFromTop(UI_MARGIN_SIZE * 3));
  b_area.removeFromTop(UI_MARGIN_SIZE);

  // Now setup the two main bodies (explorer and viz)
  auto columnWidth = (b_area.getWidth() - (UI_MARGIN_SIZE * 3)) / 4;
  // ModelExplorer is full area, as there is no FoldablePanel here
  _modelExplorer.setBounds(b_area);
  // The model panel however only have the 3 leftmost columns
  _modelPanel.setBounds(
      b_area.removeFromLeft(columnWidth * 3 + (UI_MARGIN_SIZE * 2)));

  // Reset the b_area, as we don't want the right margin
  // because the FoldablePanel must touch the right window border
  b_area = getLocalBounds()
               .withTrimmedTop(UI_MARGIN_SIZE * 5) // Header and top margins
               .withTrimmedBottom(UI_MARGIN_SIZE);
  _foldablePanel.setBounds(
      b_area.removeFromRight(columnWidth + UI_MARGIN_SIZE));
  _console.setBounds(0, getLocalBounds().getHeight() - 20,
                     getLocalBounds().getWidth(), 20);
}

void RaveAPEditor::paint(juce::Graphics &g) {
  g.fillAll(Colour::fromRGBA(185, 185, 185, 255));
  g.drawImageAt(_bgFull, 0, 0);
}

void RaveAPEditor::log(String /*str*/) {}

void RaveAPEditor::changeListenerCallback(ChangeBroadcaster * /*source*/) {
  _modelPanel.updateModel();
  if (audioProcessor._rave != nullptr) {
    // std::cout << "set prior in changeListenerCallback to" <<
    // audioProcessor._rave->hasPrior() << std::endl;
    //_modelPanel.setPriorEnabled(audioProcessor._rave->hasPrior());
    _foldablePanel.setBufferSizeRange(
        audioProcessor._rave->getValidBufferSizes());
  }
}

// Directory search functions

bool pathExist(const std::string &s) {
  struct stat buffer;
  return (stat(s.c_str(), &buffer) == 0);
}

std::string capitalizeFirstLetter(std::string text) {
  for (unsigned int x = 0; x < text.length(); x++) {
    if (x == 0) {
      text[x] = toupper(text[x]);
    } else if (text[x - 1] == ' ') {
      text[x] = toupper(text[x]);
    }
  }
  return text;
}

void RaveAPEditor::detectAvailableModels() {
  std::string ext(".ts");
  // Reset list of available models
  _availableModelsPaths.clear();
  _availableModels.clear();
  if (!pathExist(_modelsDirPath.getFullPathName().toStdString())) {
    std::cerr << "[-] - Model directory not found" << '\n';
    return;
  }
  try {
    for (auto &p : std::filesystem::recursive_directory_iterator(
             _modelsDirPath.getFullPathName().toStdString())) {
      if (p.path().extension() == ext) {
        _availableModelsPaths.add(String(p.path()));
        // Prepare a clean model name for display
        auto tmpModelName = p.path().stem().string();
        std::replace(tmpModelName.begin(), tmpModelName.end(), '_', ' ');
        tmpModelName = capitalizeFirstLetter(tmpModelName);
        _availableModels.add(tmpModelName);
      }
    }
  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "[-] - Model not found" << '\n';
    std::cerr << e.what();
  }
  // Reset the comboBox and repopulate with the new detected values
  _header._modelComboBox.clear();
  _header._modelComboBox.addItemList(_availableModels, 1);
  // TODO: stay on the same model as before
  _header._modelComboBox.setSelectedItemIndex(0);
}
