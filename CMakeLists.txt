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
    Core/Src/MQTT_Client.cpp
    Middlewares/MQTT_Packet/MQTTConnectClient.c
    Middlewares/MQTT_Packet/MQTTConnectServer.c
    Middlewares/MQTT_Packet/MQTTDeserializePublish.c
    Middlewares/MQTT_Packet/MQTTFormat.c
    Middlewares/MQTT_Packet/MQTTPacket.c
    Middlewares/MQTT_Packet/MQTTSerializePublish.c
    Middlewares/MQTT_Packet/MQTTSubscribeClient.c
    Middlewares/MQTT_Packet/MQTTSubscribeServer.c
    Middlewares/MQTT_Packet/MQTTUnsubscribeClient.c
    Middlewares/MQTT_Packet/MQTTUnsubscribeServer.c
    Middlewares/TCP_Client/Src/TCP_Client.cpp
)

# Директории с *.h файлами
include_directories(
    Core/Inc
    Middlewares/MQTT_Packet
    Middlewares/TCP_Client/Inc
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


# Пример подключения библиотек
# Для Windows
if (WIN32)
    # Подключение библиотек
    target_link_libraries(
        ${EXECUTABLE}  
        PUBLIC
        ws2_32
        wsock32
        pthread
    )
endif ()

# Для Linux
if (UNIX)
    target_link_libraries(
        ${EXECUTABLE}  
        PUBLIC
        pthread
    )
endif ()

                    