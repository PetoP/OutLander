Table of Contents
─────────────────

1 About OutLander
2 Development status
3 Install instructions
4 Manual


1 About OutLander
═════════════════

  This program applies methodology of land evaluation for glider
  outlanding. This methodology was tested using atmospherically
  corrected Sentinel-2 and Landsat 8 imagery. Program performs land
  cover classification using user defined training samples,
  classification quality assessment using user defined control samples
  and glider outlanding criteria applications using user defined
  reclassification file, DEM source and more.


2 Development status
════════════════════

  At this stage this program offers only basic functionality necessary
  for execution of land evaluation for glides outlandig methodology.


3 Install instructions
══════════════════════

  This program directly depends on OrfeoToolbox, GDAL and
  Boost. OrfeoToolbox should be compiled with OpenCV and LibSVM.

  To build this program create separate build directory and from there
  run:

  `cmake PATH_TO_OUTLANDER_ROOT -DCMAKE_INSTALL_PREFIX=/usr/; make'

  To install program system wide run:

  `make install'

  afterwards.


4 Manual
════════

  Standard manual is installed with this program. To see it, just run:

  `man olc'

  after successful build and installation.
