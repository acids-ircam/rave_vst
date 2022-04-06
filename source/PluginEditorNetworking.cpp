#include "PluginEditor.h"

String RaveAPEditor::getCleanedString(String str) {
  int len = str.length();

  std::string tmp1 = str.toStdString();
  char *tmp = tmp1.data();
  // THE CODE ABOVE WORKS!
  // BUT THE CODE NEXT LINE DOES NOT WORK?????
  // char *tmp = str.toStdString().data();
  // We get a random string everytime if we transform to std::string then
  // char* in the same line. Something something pointers management in juce?

  tmp = curl_easy_escape(_curl, tmp, len);
  String res = String(tmp);
  curl_free(tmp);
  return res;
}

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
  String url = "http://127.0.0.1:8080/get_model?model_name=" +
               getCleanedString(modelName) + String("&model_variation=") +
               getCleanedString(modelVariation);
  curl_easy_setopt(_curl, CURLOPT_URL, url.toStdString().c_str());
  curl_easy_setopt(_curl, CURLOPT_ERRORBUFFER, &_curlErrorBuffer);
  curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, writeToFileCallback);
  // TODO: Clean up the path handling, avoid using String concatenation for that
  String outputFilePath = _modelsDirPath.getFullPathName() + String("/") +
                          modelVariation + String(".ts");
  std::cout << outputFilePath << '\n';
  FILE *fp = fopen(outputFilePath.toStdString().c_str(), "wb");
  curl_easy_setopt(_curl, CURLOPT_WRITEDATA, fp);
  _curlFlag = curl_easy_perform(_curl);
  fclose(fp);

  if (_curlFlag == CURLE_OK) {
    AlertWindow::showAsync(MessageBoxOptions()
                               .withIconType(MessageBoxIconType::WarningIcon)
                               .withTitle("Information:")
                               .withMessage("Model successfully downloaded")
                               .withButton("OK"),
                           nullptr);
    std::cout << "[+] Network - Model downloaded" << std::endl;
    detectAvailableModels();
  } else {
    std::cerr << "[-] Network - Failed to download model: " << _curlErrorBuffer
              << ". (ERRCODE " << _curlFlag << ")" << std::endl
              << url << std::endl;
    AlertWindow::showAsync(MessageBoxOptions()
                               .withIconType(MessageBoxIconType::WarningIcon)
                               .withTitle("Network Warning:")
                               .withMessage("Failed to download model: " +
                                            String(_curlErrorBuffer))
                               .withButton("OK"),
                           nullptr);
    return;
  }
}

void RaveAPEditor::getAvailableModelsFromAPI() {
  curl_easy_setopt(_curl, CURLOPT_URL,
                   "http://127.0.0.1:8080/get_available_models");
  curl_easy_setopt(_curl, CURLOPT_ERRORBUFFER, &_curlErrorBuffer);
  curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &_readBuffer);
  _curlFlag = curl_easy_perform(_curl);

  if (_curlFlag == CURLE_OK) {
    std::cout << "[ ] Network - Received API response" << std::endl;
  } else {
    std::cerr << "[-] Network - No API response: " << _curlErrorBuffer
              << ". (ERRCODE " << _curlFlag << ")" << std::endl;
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

  if (JSON::parse(_readBuffer, _parsedJson).wasOk()) {
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
