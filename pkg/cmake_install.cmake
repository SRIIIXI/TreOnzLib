# Install script for directory: /home/subrato-roy/Work/TreOnzLib

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.core.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.core.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.core.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz" TYPE SHARED_LIBRARY FILES "/home/subrato-roy/Work/TreOnzLib/pkg/libTreOnzLib.core.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.core.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.core.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.core.so"
         OLD_RPATH "/usr/local/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.core.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/subrato-roy/Work/TreOnzLib/pkg/CMakeFiles/TreOnzLib.core.dir/install-cxx-module-bmi-noconfig.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.comm.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.comm.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.comm.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz" TYPE SHARED_LIBRARY FILES "/home/subrato-roy/Work/TreOnzLib/pkg/libTreOnzLib.comm.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.comm.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.comm.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.comm.so"
         OLD_RPATH "/usr/local/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.comm.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/subrato-roy/Work/TreOnzLib/pkg/CMakeFiles/TreOnzLib.comm.dir/install-cxx-module-bmi-noconfig.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.io.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.io.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.io.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz" TYPE SHARED_LIBRARY FILES "/home/subrato-roy/Work/TreOnzLib/pkg/libTreOnzLib.io.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.io.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.io.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.io.so"
         OLD_RPATH "/usr/local/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/local/lib/TreOnz/libTreOnzLib.io.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/subrato-roy/Work/TreOnzLib/pkg/CMakeFiles/TreOnzLib.io.dir/install-cxx-module-bmi-noconfig.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/include/TreOnz/core" TYPE FILE FILES
    "/home/subrato-roy/Work/TreOnzLib/./core/include/defines.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/base64.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/buffer.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/keyvalue.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/list.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/listdoublelinked.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/queue.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/stack.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/stringex.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/file.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/directory.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/logger.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/signalhandler.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/environment.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/configuration.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/datetime.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/variant.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/dictionary.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/xml.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/json.h"
    "/home/subrato-roy/Work/TreOnzLib/./core/include/treonzlib.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/include/TreOnz/io" TYPE FILE FILES
    "/home/subrato-roy/Work/TreOnzLib/./io/include/hal.h"
    "/home/subrato-roy/Work/TreOnzLib/./io/include/haltypes.h"
    "/home/subrato-roy/Work/TreOnzLib/./io/include/i2c.h"
    "/home/subrato-roy/Work/TreOnzLib/./io/include/spi.h"
    "/home/subrato-roy/Work/TreOnzLib/./io/include/pwm.h"
    "/home/subrato-roy/Work/TreOnzLib/./io/include/adc.h"
    "/home/subrato-roy/Work/TreOnzLib/./io/include/dac.h"
    "/home/subrato-roy/Work/TreOnzLib/./io/include/gpio.h"
    "/home/subrato-roy/Work/TreOnzLib/./io/include/can.h"
    "/home/subrato-roy/Work/TreOnzLib/./io/include/usb.h"
    "/home/subrato-roy/Work/TreOnzLib/./io/include/usb.h"
    "/home/subrato-roy/Work/TreOnzLib/./io/include/ble.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/local/include/TreOnz/comm" TYPE FILE FILES
    "/home/subrato-roy/Work/TreOnzLib/./comm/include/tcpclient.h"
    "/home/subrato-roy/Work/TreOnzLib/./comm/include/mail.h"
    "/home/subrato-roy/Work/TreOnzLib/./comm/include/mime.h"
    "/home/subrato-roy/Work/TreOnzLib/./comm/include/securitytypes.h"
    "/home/subrato-roy/Work/TreOnzLib/./comm/include/imap4.h"
    "/home/subrato-roy/Work/TreOnzLib/./comm/include/smtp.h"
    "/home/subrato-roy/Work/TreOnzLib/./comm/include/mqtt.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/subrato-roy/Work/TreOnzLib/pkg/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
