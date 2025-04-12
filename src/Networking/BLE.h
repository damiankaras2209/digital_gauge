//#ifndef _BLE_H
//#define _BLE_H
//
//#include <Log.h>
//#include <NimBLEDevice.h>
//
//#define GATE_SERVICE_UUID               "c829dab0-c262-414c-b226-e696ef985076"
//#define GATE_CHARACTERISTIC_UUID        "b660998e-1b97-46ba-8172-3d30a9e8b96f"
//#define GATE_ACK_CHARACTERISTIC_UUID    "d39e623b-1f13-46a9-9b39-041b29438081"
//
//#define SERVER_NAME "Digital Gauge"
//#define PASSCODE 213769
//
//#define MESSAGE_GATE "GATE_SBS"
//
//class BLEClass {
//
//    friend class NetworkingClass;
//
//    private:
//        typedef struct BluetoothParams {
//            NimBLEServer         *pServer = nullptr;
//            NimBLEService        *pService = nullptr;
//            NimBLECharacteristic *pGateCharacteristic = nullptr;
//            NimBLECharacteristic *pGateAckCharacteristic = nullptr;
//            bool                  deviceConnected = false;
//            bool                  oldDeviceConnected = false;
//            uint32_t              value = 0;
//        } BluetoothParams;
//        static BluetoothParams _bluetoothParams;
//
//        int startBLEServer();
//        bool isConnected();
//        void sendMessage(const std::string& msg);
//
//        // _Noreturn  static void bluetoothLoop(void *);
//
//};
//
//extern BLEClass BLE;
//
//#endif
