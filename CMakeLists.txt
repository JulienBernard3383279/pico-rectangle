cmake_minimum_required(VERSION 3.12)

#set(PICO_SDK_PATH "../../pico-sdk")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(PICO_SDK_FETCH_FROM_GIT 1)

include(pico_sdk_import.cmake)

pico_sdk_init()


project(pico_rectangle CXX C)
include_directories(include pico-joybus-comms/include)
add_executable(
  pico_rectangle
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
target_compile_definitions(pico_rectangle PRIVATE ULT_2IP_WITH_REAC=0)
pico_generate_pio_header(pico_rectangle ${CMAKE_CURRENT_LIST_DIR}/pio/my_pio.pio)
add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/generated/my_pio.pio.h
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/my_pio.pio
        COMMAND Pioasm ${CMAKE_CURRENT_LIST_DIR}/my_pio.pio ${CMAKE_CURRENT_LIST_DIR}/generated/my_pio.pio.h
        )
target_link_libraries(pico_rectangle pico_stdlib hardware_pio pico_time pico_bootrom hardware_resets hardware_timer hardware_irq hardware_sync hardware_flash)
pico_add_extra_outputs(pico_rectangle)


project(pico_rectangle_ult_2ip_with_reac CXX C)
include_directories(include pico-joybus-comms/include)
add_executable(
  pico_rectangle_ult_2ip_with_reac
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
target_compile_definitions(pico_rectangle_ult_2ip_with_reac PRIVATE ULT_2IP_WITH_REAC=1)
pico_generate_pio_header(pico_rectangle_ult_2ip_with_reac ${CMAKE_CURRENT_LIST_DIR}/pio/my_pio.pio)
add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/generated/my_pio.pio.h
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/my_pio.pio
        COMMAND Pioasm ${CMAKE_CURRENT_LIST_DIR}/my_pio.pio ${CMAKE_CURRENT_LIST_DIR}/generated/my_pio.pio.h
        )
target_link_libraries(pico_rectangle_ult_2ip_with_reac pico_stdlib hardware_pio pico_time pico_bootrom hardware_resets hardware_timer hardware_irq hardware_sync hardware_flash)
pico_add_extra_outputs(pico_rectangle_ult_2ip_with_reac)

