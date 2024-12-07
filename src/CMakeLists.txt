add_executable(eeprom_pc)

target_sources(eeprom_pc PRIVATE
        main.cpp
        sd_page_mem.cpp
        irs_simulation.cpp
        irsmem.h
)

target_include_directories(eeprom_pc PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(eeprom_pc PRIVATE EEPROM_FILE=\"${PROJECT_SOURCE_DIR}/eeprom.raw\")
