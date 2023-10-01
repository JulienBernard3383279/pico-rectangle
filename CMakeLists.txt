cmake_minimum_required(VERSION 3.12)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(PICO_SDK_FETCH_FROM_GIT 1)

include(pico_sdk_import.cmake)

pico_sdk_init()

project(pico_rectangle CXX C)
include_directories(include pico-joybus-comms/include)

# Define the options as a list
set(ULT_OPTIONS "ULT_2IP_NO_REAC" "ULT_2IP_WITH_REAC" "ULT_NEUTRAL")

# Loop through the options and create a target for each one
foreach(ULT_OPTION ${ULT_OPTIONS})
  # Create a target name for the current option
  set(TARGET_NAME "pico_rectangle_${ULT_OPTION}")
  # If this is the default option, set the output file name to "pico_rectangle"
  if(${ULT_OPTION} STREQUAL "ULT_2IP_NO_REAC")
    set(TARGET_NAME "pico_rectangle")
  endif()
  
  # Add an executable target for the current option
  add_executable(${TARGET_NAME}
    src/main.cpp
    src/communication_protocols/joybus.cpp
    src/communication_protocols/usb.cpp
    src/usb_configurations/gcc_to_usb_adapter.cpp
    src/usb_configurations/hid_with_triggers.cpp
    src/usb_configurations/keyboard_8kro.cpp
    src/usb_configurations/wired_fight_pad_pro.cpp
    src/usb_configurations/xbox_360.cpp
    src/dac_algorithms/melee_F1.cpp
    src/dac_algorithms/project_plus_F1.cpp
    src/dac_algorithms/ultimate_F1.cpp
    src/dac_algorithms/set_of_8_keys.cpp
    src/dac_algorithms/wired_fight_pad_pro_default.cpp
    src/dac_algorithms/xbox_360.cpp
    src/gpio_to_button_sets/F1.cpp
    src/other/runtime_remapping_mode.cpp
  )
  
  # Add compile definitions for the current option
  target_compile_definitions(${TARGET_NAME} PRIVATE ${ULT_OPTION}=1)
  
  # Generate the PIO header
  pico_generate_pio_header(${TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio/my_pio.pio)
  
  # Add the custom command for generating the PIO header
  add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/generated/my_pio.pio.h
          DEPENDS ${CMAKE_CURRENT_LIST_DIR}/my_pio.pio
          COMMAND Pioasm ${CMAKE_CURRENT_LIST_DIR}/my_pio.pio ${CMAKE_CURRENT_LIST_DIR}/generated/my_pio.pio.h
  )
  
  # Link libraries for the current option
  target_link_libraries(${TARGET_NAME} pico_stdlib hardware_pio pico_time pico_bootrom hardware_resets hardware_timer hardware_irq hardware_sync hardware_flash)
  
  # Add extra outputs for the current option
  pico_add_extra_outputs(${TARGET_NAME})
  
endforeach()
