option(AVR "Build for AVR" ON)
if(AVR)
  set(CMAKE_SYSTEM_NAME Generic)
  set(CMAKE_C_COMPILER avr-gcc)
  set(CMAKE_ASM_COMPILER avr-gcc)
  set(MCU atmega328p)
  set(F_CPU 16000000UL)
  set(COMMON_FLAGS "-mmcu=${MCU} -DF_CPU=${F_CPU} -Os -Wall -Wextra -fdata-sections -ffunction-sections")
  set(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu99")
  set(CMAKE_ASM_FLAGS "${COMMON_FLAGS} -x assembler-with-cpp")

  add_executable(garageThermostat.elf
    ../src/garageThermostat.c
    ../src/OLED_SSD1306.c
    ../src/adc.c
    ../src/display.c
    ../src/hcsr04.c
    ../src/i2c.c
    ../src/motionControl.c
    ../src/temperature_SHT41.c
    ../src/uart.c
  )
  target_link_libraries(garageThermostat.elf PRIVATE garage_lib)
  target_include_directories(garageThermostat.elf PRIVATE ../lib)

  set(OBJCOPY avr-objcopy)
  set(SIZE avr-size)
  set(AVRDUDE avrdude)

  target_link_options(garageThermostat.elf PRIVATE
    -mmcu=${MCU}
    -Wl,--gc-sections
    -Wl,-Map=garageThermostat.map
  )

  add_custom_command(TARGET garageThermostat.elf POST_BUILD
    COMMAND ${OBJCOPY} -O ihex -R .eeprom $<TARGET_FILE:garageThermostat.elf> garageThermostat.hex
    COMMAND ${SIZE} --format=avr --mcu=${MCU} $<TARGET_FILE:garageThermostat.elf>
    COMMENT "Creating garageThermostat.hex and displaying size information"
  )

  set(PROGRAMMER arduino)
  set(PORT /dev/ttyACM0)
  set(BAUD 115200)

  add_custom_target(flash
    COMMAND ${AVRDUDE} -p ${MCU} -c ${PROGRAMMER} -P ${PORT} -b ${BAUD} -U flash:w:garageThermostat.hex:i
    DEPENDS garageThermostat.elf
    COMMENT "Flashing garageThermostat.hex via avrdude"
  )

  add_custom_target(flash-isp
    COMMAND ${AVRDUDE} -p ${MCU} -c usbasp -U flash:w:garageThermostat.hex:i
    DEPENDS garageThermostat.elf
    COMMENT "Flashing garageThermostat.hex via ISP programmer"
  )
endif()
