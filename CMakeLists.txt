cmake_minimum_required(VERSION 3.20)

option(AVR "Build for AVR target" ON)

if(AVR)
  # Toolchain for AVR target (MUST be before project())
  set(CMAKE_SYSTEM_NAME Generic)
  set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
  set(CMAKE_C_COMPILER avr-gcc)
  set(CMAKE_ASM_COMPILER avr-gcc)
endif()

project(garageThermostat C ASM)

if(AVR)
  # AVR tools
  set(OBJCOPY avr-objcopy)
  set(SIZE avr-size)
  set(AVRDUDE avrdude)

  # ATmega328P configuration
  set(MCU atmega328p)
  set(F_CPU 16000000UL)  # 16 MHz clock (Arduino Uno standard)

  # Compilation flags
  set(COMMON_FLAGS "-mmcu=${MCU} -DF_CPU=${F_CPU} -Os -Wall -Wextra -fdata-sections -ffunction-sections")
  set(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu99")
  set(CMAKE_ASM_FLAGS "${COMMON_FLAGS} -x assembler-with-cpp")

  # Include directories
  include_directories(${PROJECT_SOURCE_DIR}/lib)

  # Source files
  set(SOURCES
    lib/circularBuffer.c
    avr/garageThermostat.c
    avr/OLED_SSD1306.c
    avr/adc.c
    avr/display.c
    avr/hcsr04.c
    avr/i2c.c
    avr/motionControl.c
    avr/temperature_SHT41.c
    avr/uart.c
  )

  # Executable
  add_executable(${PROJECT_NAME}.elf ${SOURCES})

  target_link_options(${PROJECT_NAME}.elf PRIVATE
    -mmcu=${MCU}
    -Wl,--gc-sections
    -Wl,-Map=${PROJECT_NAME}.map
  )

  # Generate hex file for flashing
  add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${OBJCOPY} -O ihex -R .eeprom $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
    COMMAND ${SIZE} --format=avr --mcu=${MCU} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "Creating ${PROJECT_NAME}.hex and displaying size information"
  )

  # Flash with avrdude (Arduino Uno bootloader settings)
  set(PROGRAMMER arduino)
  set(PORT /dev/ttyACM0)  # Change to COM port on Windows (e.g., COM3)
  set(BAUD 115200)

  add_custom_target(flash
    COMMAND ${AVRDUDE} -p ${MCU} -c ${PROGRAMMER} -P ${PORT} -b ${BAUD} -U flash:w:${PROJECT_NAME}.hex:i
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Flashing ${PROJECT_NAME}.hex via avrdude"
  )

  # Flash using ISP programmer (alternative)
  add_custom_target(flash-isp
    COMMAND ${AVRDUDE} -p ${MCU} -c usbasp -U flash:w:${PROJECT_NAME}.hex:i
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Flashing ${PROJECT_NAME}.hex via ISP programmer"
  )
else()
  # Host build: do not set AVR toolchain or flags here
  # Add host/portable targets or add_subdirectory(lib tests) as needed
endif()
