cmake_minimum_required(VERSION 3.10)
project(GPL_Pedals_GUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ВАЖНО: Явно включаем автоматическую обработку Qt макросов (MOC)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Находим Qt6
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Создаем исполняемый файл
add_executable(GPL_Pedals_GUI
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
)

# Debug — консольная подсистема (виден терминал),
# Release и остальные конфигурации — GUI-подсистема (без терминала)
set_target_properties(GPL_Pedals_GUI PROPERTIES
    WIN32_EXECUTABLE $<IF:$<CONFIG:Debug>,FALSE,TRUE>
)

# Подключаем Qt
target_link_libraries(GPL_Pedals_GUI PRIVATE Qt6::Widgets)

# Подключаем vJoy SDK
target_include_directories(GPL_Pedals_GUI PRIVATE ${CMAKE_SOURCE_DIR}/vjoy/include)

set(VJOY_LIB_PATH "${CMAKE_SOURCE_DIR}/vjoy/lib/vJoyInterface.lib")
if(NOT EXISTS ${VJOY_LIB_PATH})
    message(FATAL_ERROR "Файл vJoyInterface.lib не найден!\nПроверь путь: ${VJOY_LIB_PATH}")
endif()

target_link_libraries(GPL_Pedals_GUI PRIVATE ${VJOY_LIB_PATH})

# Показываем файлы в Qt Creator
target_sources(GPL_Pedals_GUI PRIVATE
    ${CMAKE_SOURCE_DIR}/vjoy/include/public.h
    ${CMAKE_SOURCE_DIR}/vjoy/include/vjoyinterface.h
)

# Копируем DLL из папки проекта в папку сборки
if(WIN32)
    set(VJOY_DLL_PATH "${CMAKE_SOURCE_DIR}/vjoy/dll/vJoyInterface.dll")
    
    # Проверка: если DLL нет, выдаём ошибку
    if(NOT EXISTS ${VJOY_DLL_PATH})
        message(FATAL_ERROR "Файл vJoyInterface.dll не найден!\nПроверь путь: ${VJOY_DLL_PATH}\nСкопируй DLL из vJoySDK\\SDK\\lib\\amd64\\ в папку vjoy\\dll\\")
    endif()
    
    add_custom_command(TARGET GPL_Pedals_GUI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${VJOY_DLL_PATH}
        $<TARGET_FILE_DIR:GPL_Pedals_GUI>
    )
endif()