#!/bin/bash

while true; do
    echo ""
    echo "===== Garage Thermostat CI/CD Menu ====="
    echo "1) Build AVR Docker Image"
    echo "2) Build Test Docker Image"
    echo "3) Configure AVR (cmake -B build-avr -DAVR=ON)"
    echo "4) Build AVR Firmware"
    echo "5) Flash AVR Firmware"
    echo "6) Clean AVR Build"
    echo "7) Configure Host Tests (cmake -B build-host -DAVR=OFF)"
    echo "8) Build Host Tests"
    echo "9) Run Host Unit Tests"
    echo "10) Clean Host Test Build"
    echo "0) Exit"
    echo "========================================"
    echo -n "Select command (0-10): "
    read choice

    case $choice in
        1)
            echo "Building AVR Docker Image..."
            docker build -f Dockerfile -t garage-thermostat .
            ;;
        2)
            echo "Building Test Docker Image..."
            docker build -f Docker.test -t garage-thermostat-test .
            ;;
        3)
            echo "Configuring AVR build..."
            docker run --rm -v $(pwd):/work -w /work/build-avr garage-thermostat cmake -DAVR=ON ..
            ;;
        4)
            echo "Building AVR firmware..."
            docker run --rm -v $(pwd):/work -w /work/build-avr garage-thermostat make
            ;;
        5)
            echo "Flashing AVR firmware..."
            docker run --rm --device=/dev/ttyACM0 -v $(pwd):/work -w /work/build-avr garage-thermostat make flash
            ;;
        6)
            echo "Cleaning AVR build..."
            docker run --rm -v $(pwd):/work -w /work garage-thermostat bash -c 'rm -rf build-avr'
            ;;
        7)
            echo "Configuring host test build..."
            docker run --rm -v $(pwd):/work -w /work/build-host garage-thermostat-test cmake -DAVR=OFF ..
            ;;
        8)
            echo "Building host unit tests..."
            docker run --rm -v $(pwd):/work -w /work/build-host garage-thermostat-test make
            ;;
        9)
            echo "Running host unit tests..."
            docker run --rm -v $(pwd):/work -w /work/build-host garage-thermostat-test ctest
            ;;
        10)
            echo "Cleaning host test build..."
            docker run --rm -v $(pwd):/work -w /work garage-thermostat-test bash -c 'rm -rf build-host'
            ;;
        0)
            echo "Exiting..."
            exit 0
            ;;
        *)
            echo "Invalid choice. Please select 0-10."
            ;;
    esac
done
