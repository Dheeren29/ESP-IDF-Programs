# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/dheer/esp/v5.3/esp-idf/components/bootloader/subproject"
  "C:/Users/dheer/Downloads/vs codes esp_idf/DHT11_Esp32 Project_NO Library/build/bootloader"
  "C:/Users/dheer/Downloads/vs codes esp_idf/DHT11_Esp32 Project_NO Library/build/bootloader-prefix"
  "C:/Users/dheer/Downloads/vs codes esp_idf/DHT11_Esp32 Project_NO Library/build/bootloader-prefix/tmp"
  "C:/Users/dheer/Downloads/vs codes esp_idf/DHT11_Esp32 Project_NO Library/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/dheer/Downloads/vs codes esp_idf/DHT11_Esp32 Project_NO Library/build/bootloader-prefix/src"
  "C:/Users/dheer/Downloads/vs codes esp_idf/DHT11_Esp32 Project_NO Library/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/dheer/Downloads/vs codes esp_idf/DHT11_Esp32 Project_NO Library/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/dheer/Downloads/vs codes esp_idf/DHT11_Esp32 Project_NO Library/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
