cmake_minimum_required(VERSION 3.27)
project(test_eeprom_pc)

set(CMAKE_CXX_STANDARD 17)

add_executable(test_eeprom_pc main.cpp sd_page_mem.cpp oper_time.cpp irs_simulation.cpp)
target_include_directories(test_eeprom_pc PUBLIC
        .
)
