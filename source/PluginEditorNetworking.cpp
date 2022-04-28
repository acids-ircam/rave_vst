#include "PluginEditor.h"

void RaveAPEditor::downloadModelFromAPI() {
  if (_modelExplorer._modelsVariationsData.size() < 1) {
    std::cout << "[ ] Network - No models available for download" << std::endl;
    AlertWindow::showAsync(MessageBoxOptions()
                               .withIconType(MessageBoxIconType::WarningIcon)
                               .withTitle("Information:")
                               .withMessage("No models available for download")
                               .withButton("OK"),
                           nullptr);
    return;
  }
  String modelName =
      _modelExplorer._modelsVariationsData[_modelExplorer._modelsList
                                               .getSelectedRow()]["model_root"];
  String modelVariation =
      _modelExplorer
          ._modelsVariationsNames[_modelExplorer._modelsList.getSelectedRow()];
  String tmp_url = "http://127.0.0.1:8080/get_model?model_name=" +
                   URL::addEscapeChars(modelName, true, false) +
                   String("&model_variation=") +
                   URL::addEscapeChars(modelVariation, true, false);
  URL url = URL(tmp_url);

  // TODO: Clean up the path handling, avoid using String concatenation for that
  String outputFilePath = _modelsDirPath.getFullPathName() + String("/") +
                          modelVariation + String(".ts");
  std::cout << "DOWNLOADING TO: " << outputFilePath << '\n';
  // FILE *fp = fopen(outputFilePath.toStdString().c_str(), "wb");
  File outputFile = File(outputFilePath);
  std::unique_ptr<URL::DownloadTask> res =
      url.downloadToFile(outputFile, URL::DownloadTaskOptions());

  if (!res->hadError()) {
    AlertWindow::showAsync(MessageBoxOptions()
                               .withIconType(MessageBoxIconType::InfoIcon)
                               .withTitle("Information:")
                               .withMessage("Model successfully downloaded")
                               .withButton("OK"),
                           nullptr);
    std::cout << "[+] Network - Model downloaded" << std::endl;
    detectAvailableModels();
  } else {
    std::cerr << "[-] Network - Failed to download model" << std::endl;
    AlertWindow::showAsync(MessageBoxOptions()
                               .withIconType(MessageBoxIconType::WarningIcon)
                               .withTitle("Network Warning:")
                               .withMessage("Failed to download model")
                               .withButton("OK"),
                           nullptr);
    return;
  }
}

void RaveAPEditor::getAvailableModelsFromAPI() {
  URL url("http://127.0.0.1:8080/get_available_models");
  String res = url.readEntireTextStream();
  if (res.length() > 0) {
    std::cout << "[ ] Network - Received API response" << std::endl;
  } else {
    std::cerr << "[-] Network - No API response" << std::endl;
    // Disabled for the beta, as we won't activate the api
    // AlertWindow::showAsync(
    //     MessageBoxOptions()
    //         .withIconType(MessageBoxIconType::WarningIcon)
    //         .withTitle("Network Warning:")
    //         .withMessage("No API response: " + String(_curlErrorBuffer))
    //         .withButton("OK"),
    //     nullptr);
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
      _modelExplorer._modelsVariationsNames.add(modelName);
      // Add all the model variation data to its own array
      DynamicObject &modelVariationData =
          *_parsedJson[modelName.toStdString().c_str()].getDynamicObject();
      NamedValueSet &props = modelVariationData.getProperties();
      _modelExplorer._modelsVariationsData.add(props);
    }
    _modelExplorer._modelsList.updateContent();
  }
}
