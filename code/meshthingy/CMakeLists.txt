# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
set(QEMU_EXTRA_FLAGS -s)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(meshthingy)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/board.c)
target_sources(app PRIVATE src/scu_sensors.c)
target_sources(app PRIVATE src/battery.c)


if(NODE_ADDR)
  zephyr_compile_definitions(NODE_ADDR=${NODE_ADDR})
endif()