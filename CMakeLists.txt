cmake_minimum_required(VERSION 3.10)
project(bcml C)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src/include)
include_directories(${CMAKE_SOURCE_DIR}/src/lib/validator)
include_directories(${CMAKE_SOURCE_DIR}/src/lib/dataconvert)
include_directories(${CMAKE_SOURCE_DIR}/src/lib/sb)

# Source files
file(GLOB ROOT_SRC src/lib/*.c)
file(GLOB CORE_SRC src/lib/core/*.c)
file(GLOB VALIDATOR_SRC src/lib/validator/*.c)
file(GLOB DATACONVERT_SRC src/lib/dataconvert/*.c)

# --- Southbound backend selection via option --- #
option(REST_API_ENABLE "Enable REST API backend" OFF)
option(UCI_API_ENABLE "Enable UCI backend" OFF)

if(REST_API_ENABLE AND UCI_API_ENABLE)
  message(FATAL_ERROR "Cannot enable both REST_API_ENABLE and UCI_API_ENABLE at the same time.")
endif()

if(REST_API_ENABLE)
  add_definitions(-DREST_API_ENABLE)
  set(SB_BACKEND_SRC src/lib/sb/sb_ops.c src/lib/sb/restapi/sb_ops_restapi.c src/lib/sb/restapi/rest_client.c)
elseif(UCI_API_ENABLE)
  add_definitions(-DUCI_API_ENABLE)
  set(SB_BACKEND_SRC src/lib/sb/sb_ops.c src/lib/sb/uci/sb_ops_uci.c)
else()
  message(FATAL_ERROR "You must enable one of REST_API_ENABLE or UCI_API_ENABLE.")
endif()

add_library(bcml STATIC ${ROOT_SRC} ${CORE_SRC} ${VALIDATOR_SRC} ${DATACONVERT_SRC} ${SB_BACKEND_SRC})

# Optional: set C standard
set_target_properties(bcml PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    POSITION_INDEPENDENT_CODE ON
)

# Install rules (optional)
install(TARGETS bcml DESTINATION lib)
install(DIRECTORY src/include/ DESTINATION include)
