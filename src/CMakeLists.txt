add_executable(eeprom_pc)

target_sources(eeprom_pc PRIVATE
        main.cpp
        sd_page_mem.cpp
        oper_time.cpp
        irs_simulation.cpp
        irsmem.h
)

target_include_directories(eeprom_pc PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
