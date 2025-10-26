import serial
import time

import asyncio
from kasa import Discover
from aioconsole import ainput

SERIAL_PORT = "COM15"
BAUD_RATE = 9600


async def listDevices():
    devices = await Discover.discover()
    for dev in devices.values():
        await dev.update()
        print(dev.device_type, dev.host)

global TEMP_THRESHOLD 
global HUMIDITY_THRESHOLD
TEMP_THRESHOLD = 61.0
HUMIDITY_THRESHOLD = 60.0

paused = False

async def handle_user_input():
    """Background task to handle user input without blocking"""
    global TEMP_THRESHOLD, HUMIDITY_THRESHOLD
    
    while True:
        print("\n[Press Enter to adjust thresholds]")
        await ainput()  # Wait for Enter
        
        paused = True # ENTER pressed, stop main loop
        
        try:
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
        finally:
            paused = False  # resume main loop

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
    asyncio.create_task(handle_user_input()) # background task
    # Read values before while

    # TODO ----------------------------------------------
    # Use interrupt for enter key set flag, then in while loop block program, get input and resume

    # OR 
    # CReate two threads, one for handling user input, the other processing. Join the threads at the end

    try:
        while True:
            if paused:
                await asyncio.sleep(0.1)
                continue
                
            #await handle_user_input()
            response_raw = sp.readline()
            response = response_raw.decode('utf-8').strip()
            print(response)

#            try:

            # TODO check to see if thresholds have changed


            # Temperature: 22.88 C Humidity: 60.47 %
            if "Temperature" in response:

                parts = response.split(" ")
                #print(parts)
                temp = float(parts[1])
                humidity = float(parts[4])
                celcius = round((TEMP_THRESHOLD - 32) / (9/5), 1) # or 1.8 what is this number?
                print(f"Current thresholds: {TEMP_THRESHOLD} F {celcius} C {HUMIDITY_THRESHOLD} %")

                if temp > TEMP_THRESHOLD or humidity > HUMIDITY_THRESHOLD:

                    print("turning on fan")
                    await wallSwitch.turn_on()
                else:
                    print("turning off fan")
                    await wallSwitch.turn_off()

                await wallSwitch.update()
            
            await asyncio.sleep(1)

#            except: (IndexError, ValueError) as e:
#                print(f"Error parsing response: {response}")

            # TODO call again here to update the value


    except KeyboardInterrupt:
        print("\nInterrupted!")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        await wallSwitch.turn_off()
        await wallSwitch.update()


if __name__ == "__main__":
        asyncio.run(main())
