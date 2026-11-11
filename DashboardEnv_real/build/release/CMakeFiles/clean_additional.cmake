# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\appDashboardEnv_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\appDashboardEnv_autogen.dir\\ParseCache.txt"
  "appDashboardEnv_autogen"
  )
endif()
