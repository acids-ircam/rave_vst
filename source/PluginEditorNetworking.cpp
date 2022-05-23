#include "PluginEditor.h"

void RaveAPEditor::finished(URL::DownloadTask *task, bool success) {
  if (success) {
    std::cout << "[+] Network - Model downloaded" << std::endl;
    // AlertWindow::showAsync(MessageBoxOptions()
    //                            .withIconType(MessageBoxIconType::InfoIcon)
    //                            .withTitle("Information:")
    //                            .withMessage("Model successfully downloaded")
    //                            .withButton("OK"),
    //                        nullptr);
    detectAvailableModels();
  } else {
    std::cerr << "[-] Network - Failed to download model" << std::endl;
    // AlertWindow::showAsync(MessageBoxOptions()
    //                            .withIconType(MessageBoxIconType::WarningIcon)
    //                            .withTitle("Network Warning:")
    //                            .withMessage("Failed to download model")
    //                            .withButton("OK"),
    //                        nullptr);
  }
}

void RaveAPEditor::progress(URL::DownloadTask *task, int64 bytesDownloaded,
                            int64 totalLength) {
  std::cout << bytesDownloaded << "/" << totalLength << '\n';
}

void RaveAPEditor::downloadModelFromAPI() {
  if (_modelExplorer._ApiModelsData.size() < 1) {
    std::cout << "[ ] Network - No models available for download" << std::endl;
    // AlertWindow::showAsync(MessageBoxOptions()
    //                            .withIconType(MessageBoxIconType::WarningIcon)
    //                            .withTitle("Information:")
    //                            .withMessage("No models available for
    //                            download") .withButton("OK"),
    //                        nullptr);
    return;
  }
  String modelName =
      _modelExplorer
          ._ApiModelsNames[_modelExplorer._modelsList.getSelectedRow()];
  String tmp_url = _apiRoot + String("get_model?model_name=") +
                   URL::addEscapeChars(modelName, true, false);
  URL url = URL(tmp_url);

  std::cout << url.toString(true) << '\n';

  // TODO: Clean up the path handling, avoid using String concatenation for
  // that
  String outputFilePath = _modelsDirPath.getFullPathName() + String("/") +
                          modelName + String(".ts");
  File outputFile = File(outputFilePath);
  std::unique_ptr<URL::DownloadTask> res = url.downloadToFile(
      outputFile, URL::DownloadTaskOptions().withListener(this));

  if (res->hadError()) {
    std::cerr << "[-] Network - Failed to download model" << std::endl;
    // AlertWindow::showAsync(MessageBoxOptions()
    //                            .withIconType(MessageBoxIconType::WarningIcon)
    //                            .withTitle("Network Warning:")
    //                            .withMessage("Failed to download model")
    //                            .withButton("OK"),
    //                        nullptr);
    return;
  }
  while (!res->isFinished())
    Thread::sleep(500);
}

void RaveAPEditor::getAvailableModelsFromAPI() {
  URL url(_apiRoot + String("get_available_models"));
  String res = url.readEntireTextStream();

  if (res.length() > 0) {
    std::cout << "[ ] Network - Received API response" << std::endl;
  } else {
    std::cerr << "[-] Network - No API response" << std::endl;
    return;
  }

  if (JSON::parse(res, _parsedJson).wasOk()) {
    Array<juce::var> &available_models =
        *_parsedJson["available_models"].getArray();
    std::cout << "[+] Network - Successfully parsed JSON, "
              << available_models.size() << " models available online:" << '\n';
    for (int i = 0; i < available_models.size(); i++) {
      // Add model variation name to its array
      String modelName = available_models[i].toString();
      std::cout << "\t- " << modelName << '\n';
      _modelExplorer._ApiModelsNames.add(modelName);
      // Add all the model variation data to its own array
      DynamicObject &modelVariationData =
          *_parsedJson[modelName.toStdString().c_str()].getDynamicObject();
      NamedValueSet &props = modelVariationData.getProperties();
      _modelExplorer._ApiModelsData.add(props);
    }
    _modelExplorer._modelsList.updateContent();
  }
}

String RaveAPEditor::getApiRoot() {
  unsigned char b[] = {104, 116, 116, 112, 115, 58,  47,  47, 112, 108, 97,
                       121, 46,  102, 111, 114, 117, 109, 46, 105, 114, 99,
                       97,  109, 46,  102, 114, 47,  114, 97, 118, 101, 45,
                       118, 115, 116, 45,  97,  112, 105, 47};
  char c[sizeof(b) + 1];
  memcpy(c, b, sizeof(b));
  c[sizeof(b)] = '\0';
  return String(c);
}
