# RAVE VST (BETA)
-----
Include RAVE models in your DAW for realtime deep learning based processing

- VST / AU / Standalone plugins available
- Unix & MacOS (We plan to support Windows later on)
- Reconstruction & Prior modes available

![rave_audition](assets/rave_screenshot_audio_panel.png)

-----
### 1) How to use
##### Audio settings
Click on the button with an arrow on the right to open / close the audio settings panel. Here you can adjust:
  - Input: Gain, Channel, Compression threshold & ratio
  - Output: Gain & Dry / Wet mix
  - Buffer size: Internal buffer size used (small buffer size = low latency, more audio clicks)

-----
You can switch between reconstruction & prior modes using the tickbox on the left
##### Reconstruction mode
In reconstruction mode, RAVE will use the audio input given by your DAW and reconstruct it
- Latent bias & scale can be changed for each of the 1st 8 latents: Select the latent you want to edit on the central wheel, then use the two bottom left buttons to change the values

##### Prior mode
In Prior mode, RAVE will move through the latent space using its prior
- You can adjust the latent noise, which will add noise to all latent dimensions

-----
##### Stereo Width
This knob set the audio separation between the two output channels of the RAVE model.  
Those two channels have the same input but the random sampling differences will produce slightly different outputs, resulting in a nice stereo effect

-----
##### Model Explorer
**Feature not yet available:** The Model Explorer Button switches to the model explorer window. When available you will be able to download a selection of models directly from the VST  

Right now you can use this window to select your custom models, this will put them in the right folder and refresh the available models list

-----
### 2) How to build
- **MacOS 10.15.7**: Cmake 3.21.3, Clang 11.0.3, Xcode 11.7  
  - VST & AU & Standalone
- **Ubuntu**: Cmake, g++
  - VST & Standalone  
- **Fedora 33** (Kernel 5.14.1A8): Cmake 3.19.7, g++ 10.3.1  
  - Standalone
  - Needed dependencies:
  git
cmake
g++
libX11-devel
libXrandr-devel
libXinerama-devel
libXcursor-devel
freetype-devel
libcurl-devel
alsa-lib-devel


We're using Cmake for the build process  
It will automatically download the PyTorch libraries (And MKL if you're on UNIX)  

- Get juce:  
`git submodule update --init --recursive`
- Setup the build:  
`mkdir build; cd build`  
`cmake .. -DCMAKE_BUILD_TYPE=Release`

- Build:  
  - Unix: `cmake --build . --config Release -j 4`
  - MacOs:
 `cmake -G Xcode -B build`
