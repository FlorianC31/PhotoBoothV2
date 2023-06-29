import wmi

def get_cpu_temp():
    w = wmi.WMI(namespace=r"root\OpenHardwareMonitor")
    temperature_infos = w.Sensor()
    cpu_temp = 0
    for sensor in temperature_infos:
        if sensor.SensorType == u'Temperature':
            cpu_temp = max(cpu_temp, sensor.Value)
    return cpu_temp
    
    
if __name__ == '__main__':
    print(get_cpu_temp())
