import serial
import time

import asyncio
from kasa import Discover
from aioconsole import ainput

SERIAL_PORT = "COM9"
BAUD_RATE = 9600


async def listDevices():
    devices = await Discover.discover()
    for dev in devices.values():
        await dev.update()
        print(dev.device_type, dev.host)

global t_threshold 
global h_threshold
t_threshold = 20.0
h_threshold = 60.0

async def handle_user_input():
    """Background task to handle user input without blocking"""
    global TEMP_THRESHOLD, HUMIDITY_THRESHOLD
    
    while True:
        try:
            print("\n[Press Enter to adjust thresholds]")
            await ainput()  # Non-blocking wait for Enter
            
            temp_str = await ainput(f"New temperature threshold (current: {TEMP_THRESHOLD}): ")
            if temp_str.strip():
                TEMP_THRESHOLD = float(temp_str)
            
            humid_str = await ainput(f"New humidity threshold (current: {HUMIDITY_THRESHOLD}): ")
            if humid_str.strip():
                HUMIDITY_THRESHOLD = float(humid_str)
            
            print(f"Updated thresholds - Temp: {TEMP_THRESHOLD}°C, Humidity: {HUMIDITY_THRESHOLD}%")
        except ValueError:
            print("Invalid input, keeping current thresholds")
        except Exception as e:
            print(f"Input error: {e}")

async def main():
    # If need to find the host ip of the WallSwitch run this for now
    # TODO auto discover and save ip
    await listDevices()

    print(f"Attempting to connect to {SERIAL_PORT}:{BAUD_RATE}")
    sp = ""
    while not sp or sp == "":
        try: 
            sp = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout = 5)
        except:
            print("Attempting to connect...")
            time.sleep(5)
            continue
        


    sp.flush()
    print("Garage Thermostat using port 192.168.1.11 no validation with above yet")

    wallSwitch = await Discover.discover_single("192.168.1.11")

    # Start input handler as backgroud task
    input_task = asyncio.create_task(handle_user_input())

    try:
        while True:

            response_raw = sp.readline()
            response = response_raw.decode('utf-8').strip()

#            try:
                # Temperature: 22.88 C Humidity: 60.47 %

            parts = response.split(" ")
            temp = float(parts[1])
            humidity = float(parts[4])

            if "Temperature" in response:

                if temp > t_threshold or humidity > h_threshold:

                    print("turning on fan")
                    await wallSwitch.turn_on()
                else:
                    print("turning off fan")
                    await wallSwitch.update()

                await wallSwitch.update()

#            except: (IndexError, ValueError) as e:
#                print(f"Error parsing response: {response}")



    except KeyboardInterrupt:
        print("\nInterrupted!")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        await wallSwitch.turn_off()
        await wallSwitch.update()


if __name__ == "__main__":
        asyncio.run(main())
