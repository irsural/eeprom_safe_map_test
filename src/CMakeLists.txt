add_executable(eeprom_pc)

target_sources(eeprom_pc PRIVATE
        main.cpp
        raw_file_page_mem.cpp
        page_mem_demo.cpp
        page_mem_demo.h
        safe_map_demo.cpp
        safe_map_demo.h
)

target_include_directories(eeprom_pc PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(eeprom_pc PRIVATE EEPROM_FILE=\"${PROJECT_SOURCE_DIR}/eeprom.raw\")
