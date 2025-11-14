#!/bin/bash

while true; do
    echo ""
    echo "===== AVR Blinky CI/CD Menu ====="
    echo "1) Build Docker Image"
    echo "2) Configure (cmake -B build)"
    echo "3) Build (cmake --build build)"
    echo "4) Flash (cmake --build build --target flash)"
    echo "5) Clean (rm -fr build)"
    echo "6) Exit"
    echo "================================="
    echo -n "Select command (1-6): "
    
    read choice
    
    case $choice in
	1)  echo "Building Docker Image..."
	    docker build -t garage-thermostat .
	    ;;
        2)
            echo "Running: Configure..."
            docker run --rm -v $(pwd):/work -w /work garage-thermostat cmake -B build
            ;;
        3)
            echo "Running: Build..."
            docker run --rm -v $(pwd):/work -w /work garage-thermostat cmake --build build
            ;;
        4)
            echo "Running: Flash..."
            docker run --rm -v $(pwd):/work -w /work --device /dev/ttyUSB0:/dev/ttyACM0 garage-thermostat cmake --build build --target flash
            ;;
        5)
            echo "Running: Clean..."
            docker run --rm -v $(pwd):/work -w /work garage-thermostat bash -c "rm -fr build"
            ;;
        6)
            echo "Exiting..."
            exit 0
            ;;
        *)
            echo "Invalid choice. Please select 1-5."
            ;;
    esac
done
