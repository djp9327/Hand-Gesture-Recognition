# Hand-Gesture-Recognition

This program uses the OpenCV library to track hand movement and count the number of fingers held up.  

## Setup

##### Install OpenCV with homebrew

```
brew tap homebrew/science
brew install opencv
```

Now your OpenCV installation will be located at /usr/local/Cellar/opencv/3.0.0

##### Setup OpenCV with Xcode

Create new project in Xcode and select project description  
Under "Search Paths" section:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1) "Header Search Paths" - add header path of openCV files: /usr/local/include  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2) "Library Search Paths" - add /usr/local/Cellar/opencv/3.0.0 /usr/local/lib  
Under "Linking" section:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1) "Other Linker Flags" - add -lopencv_calib3d -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videoio -lopencv_videostab  
   
