cmake_minimum_required(VERSION 3.5)

# Название проекта
project(Template)

# Переменная с именем исполняемого файла
set(EXECUTABLE ${PROJECT_NAME})

# Язык C и C++
enable_language(CXX C)

# Добавление файлов исъодного кода
add_executable(
    ${EXECUTABLE} 
    Core/Src/main.cpp
    MQTT_Packet/MQTTConnectClient.c
    MQTT_Packet/MQTTConnectServer.c
    MQTT_Packet/MQTTDeserializePublish.c
    MQTT_Packet/MQTTFormat.c
    MQTT_Packet/MQTTPacket.c
    MQTT_Packet/MQTTSerializePublish.c
    MQTT_Packet/MQTTSubscribeClient.c
    MQTT_Packet/MQTTSubscribeServer.c
    MQTT_Packet/MQTTUnsubscribeClient.c
    MQTT_Packet/MQTTUnsubscribeServer.c
)

# Директории с *.h файлами
include_directories(
    Core/Inc
    MQTT_Packet
)

# Флаги компиляции
add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:
    -fno-rtti 
    -fno-exceptions 
    $<$<CONFIG:Debug>:
    -Og>
    $<$<CONFIG:Release>:
    -Os>> 
    $<$<COMPILE_LANGUAGE:C>:
    $<$<CONFIG:Debug>:
    -Og>
    $<$<CONFIG:Release>:
    -Os>>
)


# # Пример подключения библиотек
# # Для Windows
# if (WIN32)
#     # Подключение библиотек
#     target_link_libraries(
#         ${EXECUTABLE}  
#         PUBLIC
#         ws2_32
#         wsock32
#         pthread
#     )
# endif ()

# # Для Linux
# if (UNIX)
#     target_link_libraries(
#         ${EXECUTABLE}  
#         PUBLIC
#         pthread
#     )
# endif ()

                    