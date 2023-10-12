# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Volumes/Data_Int/_Documents/ESP/_CODE/esp-idf/components/bootloader/subproject"
  "/Volumes/Data_Int/_Documents/ESP/_CODE/esp_selfie/build/bootloader"
  "/Volumes/Data_Int/_Documents/ESP/_CODE/esp_selfie/build/bootloader-prefix"
  "/Volumes/Data_Int/_Documents/ESP/_CODE/esp_selfie/build/bootloader-prefix/tmp"
  "/Volumes/Data_Int/_Documents/ESP/_CODE/esp_selfie/build/bootloader-prefix/src/bootloader-stamp"
  "/Volumes/Data_Int/_Documents/ESP/_CODE/esp_selfie/build/bootloader-prefix/src"
  "/Volumes/Data_Int/_Documents/ESP/_CODE/esp_selfie/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Volumes/Data_Int/_Documents/ESP/_CODE/esp_selfie/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Volumes/Data_Int/_Documents/ESP/_CODE/esp_selfie/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
