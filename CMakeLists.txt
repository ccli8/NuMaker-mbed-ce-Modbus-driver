# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(nu-modbus STATIC EXCLUDE_FROM_ALL)

add_subdirectory(NuMaker-mbed-ce-RS485-driver)

target_include_directories(nu-modbus
    PUBLIC
        .
)

target_sources(nu-modbus
    PRIVATE
        mb.cpp
        mbcrc.cpp
        mbfunccoils.cpp
        mbfuncdiag.cpp
        mbfuncdisc.cpp
        mbfuncholding.cpp
        mbfuncinput.cpp
        mbfuncother.cpp
        mbrtu.cpp
        mbutils.cpp
        portevent.cpp
        porttimer.cpp
)

if("MBED_CONF_NU_MODBUS_PROVIDE_DEFAULT_SERIAL=1" IN_LIST MBED_CONFIG_DEFINITIONS)
    target_sources(nu-modbus
        PRIVATE
            portserial.cpp
    )
endif()

target_link_libraries(nu-modbus
    PUBLIC
        mbed-core-flags
        nu-rs485
)
