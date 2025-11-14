cmake_minimum_required(VERSION 3.20)

# Toolchain for AVR target (MUST be before project())
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_ASM_COMPILER avr-gcc)

project(garageThermostat C ASM)

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
  src/garageThermostat.c
  src/OLED_SSD1306.c
  src/adc.c
  src/circularBuffer.c
  src/display.c
  src/hcsr04.c
  src/i2c.c
  src/motionControl.c
  src/temperature_SHT41.c
  src/uart.c
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
set(PORT /dev/ttyUSB0)  # Change to COM port on Windows (e.g., COM3)
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

# Optional: Build tests (if circularStringBufferTests.c has a main function)
add_executable(tests.elf src/circularStringBufferTests.c src/circularBuffer.c)
target_link_options(tests.elf PRIVATE
  -mmcu=${MCU}
  -Wl,--gc-sections
)
