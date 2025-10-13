import { BleManager } from "react-native-ble-plx";
import { Buffer } from "buffer";
import { PermissionsAndroid, Platform } from "react-native";

const UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
const TX_CHARACTERISTIC_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";

class BLEHelper {
  constructor() {
    this.manager = new BleManager();
    this.device = null;
    this.connectionStatus = false;
    this.triggerTimeouts = [];
  }

  setConnectionStatus(status) {
    this.connectionStatus = status;
  }

  async requestPermissions() {
    if (Platform.OS === "android" && Platform.Version >= 23) {
      const permissions = [];
  
      permissions.push(PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION);
  
      if (Platform.Version >= 31) {
        // Android 12+ additional BLE permissions
        permissions.push(PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN);
        permissions.push(PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT);
      }
  
      const granted = await PermissionsAndroid.requestMultiple(permissions);
      const allGranted = Object.values(granted).every(
        status => status === PermissionsAndroid.RESULTS.GRANTED
      );
  
      if (!allGranted) {
        console.warn("Not all BLE permissions granted.");
      }
    }
  }

  async scanAndConnect() {
    return new Promise((resolve, reject) => {
      this.manager.startDeviceScan(null, null, async (error, scannedDevice) => {
        if (error) {
          console.error("Scan error:", error);
          reject(error);
          return;
        }

        if (scannedDevice?.name?.includes("ESP32")) {
          this.manager.stopDeviceScan();
          try {
            const connectedDevice = await scannedDevice.connect();
            await connectedDevice.discoverAllServicesAndCharacteristics();
            this.device = connectedDevice;
            this.setConnectionStatus(true);
            console.log("Connected to ESP32:", connectedDevice.name);
            resolve(connectedDevice);
          } catch (err) {
            console.error("Connection error:", err);
            reject(err);
          }
        }
      });

      setTimeout(() => {
        this.manager.stopDeviceScan();
        reject(new Error("No ESP32 device found"));
      }, 4000);
    });
  }

  async sendPacer(color, duration, distance) {
    if (!this.device) {
      console.error("No device connected.");
      return;
    }
    this.clearPendingTriggers();

    const lapDuration = parseFloat(duration) / parseFloat(distance);
    console.log(`Duration: ${duration}`);
    console.log(`Lap duration: ${lapDuration}`);
    const segmentDelayMs = (lapDuration * 1000) / 4;
    const fullLapDelayMs = lapDuration * 1000;
  
    for (let lap = 0; lap < distance; lap++) {
      console.log(`Starting lap ${lap + 1}/${distance}`);
  
      const firstPacer = `start ${color} ${lapDuration.toFixed(2)} ${distance}`;
      console.log(`${lapDuration}`)
      console.log(`Segment Distance: ${segmentDelayMs}`);
      console.log(`Sending pacer command: ${firstPacer}`);
  
      try {
        const base64Message = Buffer.from(firstPacer, "utf-8").toString("base64");
        await this.device.writeCharacteristicWithResponseForService(
          UART_SERVICE_UUID,
          TX_CHARACTERISTIC_UUID,
          base64Message
        );
        console.log("Sent start command to Unit 1");
  
        this.scheduleTrigger("two", segmentDelayMs);
        this.scheduleTrigger("three", segmentDelayMs * 2);
        this.scheduleTrigger("four", segmentDelayMs * 3);
        this.scheduleTrigger("five", segmentDelayMs * 4);
  
      } catch (error) {
        console.error("Error sending pacer sequence:", error);
      }
  
      if (lap < distance - 1) {
        console.log(`Waiting ${fullLapDelayMs}ms before next lap...`);
        await new Promise(resolve => setTimeout(resolve, fullLapDelayMs));
      }
    }
  }
  
  async sendStop() {
    const stop = `stop`;
    this.clearPendingTriggers();
    if (!this.device) {
      console.error("No device connected.");
      return;
    }
    try {
      const base64Message = Buffer.from(stop, "utf-8").toString("base64");
      await this.device.writeCharacteristicWithResponseForService(
        UART_SERVICE_UUID,
        TX_CHARACTERISTIC_UUID,
        base64Message
      );
      console.log(`Sent stop command: ${stop}`);
    } catch (error) {
      console.error("Error sending stop command:", error);
    }
  }

  async disconnect() {
    if (this.device) {
      try {
        await this.device.cancelConnection();
        console.log("Disconnected from ESP32.");
        this.device = null;
        this.setConnectionStatus(false);
      } catch (error) {
        console.error("Error disconnecting from ESP32:", error);
        throw error;
      }
    } else {
      console.log("No device to disconnect.");
    }
  }
  
  
  
  getConnectionStatus() {
    return this.connectionStatus;
  }

  clearPendingTriggers() {
    this.triggerTimeouts.forEach((timeout) => clearTimeout(timeout));
    this.triggerTimeouts = [];
  }

  scheduleTrigger(message, delay) {
    const handle = setTimeout(async () => {
      if (!this.device) {
        return;
      }
      try {
        console.log(`Sending to Unit (${message.toUpperCase()}): ${message}`);
        const encoded = Buffer.from(message, "utf-8").toString("base64");
        await this.device.writeCharacteristicWithResponseForService(
          UART_SERVICE_UUID,
          TX_CHARACTERISTIC_UUID,
          encoded
        );
        console.log(`Sent ${message} command`);
      } catch (error) {
        console.error(`Error sending ${message} command:`, error);
      } finally {
        this.triggerTimeouts = this.triggerTimeouts.filter(
          (storedHandle) => storedHandle !== handle
        );
      }
    }, delay);

    this.triggerTimeouts.push(handle);
  }
}

const bleHelper = new BLEHelper();
export default bleHelper;
