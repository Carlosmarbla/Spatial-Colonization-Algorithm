# Spatial-Colonization-Algorithm
SCA 

10/01/2020

Here you can find three different folders and a Manual.pdf on how to use spatial_colonization_algorithm program and the Raw_metrics_algorythm.

  - Macros_for_segmentation: contain macros to segment gill images in ImageJ, a model for WEKA and a manual on how to use them.
  - spatial_colonization_algorithm
  - Raw_metrics_generation_algorythm (images.cpp)


In order to be able to use the macros you must have ImageJ or FIJI. The macros are written and tested on a Linux machine. Therefore if using it with windows you may have to do slight changes in the macros.


Raw_metrics_generation_algorythm (images.cpp):

  Subfolders and files needed to execute images.cpp C++ script,this script generates raw metrics data files used then in PCA analysis.Extra information about inputs, outputs and options of the script is contained inthe same script.


spatial_colonization_algorithm:

  - Attractor_points_generator (puntos.cpp):
    - Subfolders and files needed to execute puntos.cpp C++ script, this script generates a matrix of points data fileused then in SCA.cppas input. Extra information about inputs, outputs and options of the script is contained inthe same script.

  - SCA (SCA.cpp):
    - Subfolders and files needed to execute SCA.cpp C++ script, this script develops thereal timeSCA and exports an image ready to be used by images.cppas input to generate metrics. Extra information about inputs, outputs and options of the script is contained inthe same script.



Contacts for more information or questions:

  - Carlos Martín Blanco email: carlant1991@gmail.com
  
  - Tomás Navarro Álvarez email: tnavarroez@gmail.com
