import serial

ser = serial.Serial('COM6', baudrate=115200)
filename = "serial_data.txt"
f = open(filename, "w+")

while True:
    try:
        data = ser.readline().decode().rstrip()  # Read the data from the serial port
        if data:
            f.write(data + '\n')  # Write the data to a file
            print(data)  # Print the data to the console
    except KeyboardInterrupt:
        break

ser.close()
f.close()