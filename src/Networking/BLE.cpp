// #include "BLE.h"

// BLEClass BLE;
// BLEClass::BluetoothParams BLEClass::_bluetoothParams = {};

// class ServerCallbacks final : public NimBLEServerCallbacks {
//     void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
//         Log.logf("Client connected: %s\n", connInfo.getAddress().toString().c_str());
//         pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 180);
//     }
//
//     void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
//         Log.logf("Client disconnected - start advertising\n");
//         NimBLEDevice::startAdvertising();
//     }
//
//     void onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) override {
//         Log.logf("MTU updated: %u for connection ID: %u\n", MTU, connInfo.getConnHandle());
//     }
//
//     /********************* Security handled here *********************/
//     uint32_t onPassKeyDisplay() override {
//         Log.logf("Server Passkey Display\n");
//         return PASSCODE;
//     }
//
//     void onConfirmPassKey(NimBLEConnInfo& connInfo, uint32_t pass_key) override {
//         Log.logf("The passkey YES/NO number: %" PRIu32 "\n", pass_key);
//         /** Inject false if passkeys don't match. */
//         NimBLEDevice::injectConfirmPasskey(connInfo, true);
//     }
//
//     void onAuthenticationComplete(NimBLEConnInfo& connInfo) override {
//         if (!connInfo.isEncrypted()) {
//             Log.logf("Encryption failed, disconnecting client... ");
//             if (NimBLEDevice::getServer()->disconnect(connInfo.getConnHandle()))
//                 Log.logf("disconnected\n");
//             else
//                 Log.logf("disconnect failed\n");
//         } else {
//             Log.logf("Authenticated with client: %s\n", connInfo.getAddress().toString().c_str());
//         }
//     }
// } serverCallbacks;
//
// class CharacteristicCallbacks final : public NimBLECharacteristicCallbacks {
//     void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
//         Log.logf("%s : onRead(), value: %s\n",
//             pCharacteristic->getUUID().toString().c_str(),
//             pCharacteristic->getValue().c_str()
//         );
//     }
//
//     void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
//         Log.logf("%s : onWrite(), value: %s\n",
//             pCharacteristic->getUUID().toString().c_str(),
//             pCharacteristic->getValue().c_str()
//         );
//         if (pCharacteristic->getUUID() == NimBLEUUID(GATE_ACK_CHARACTERISTIC_UUID)) {
//             pCharacteristic->getService()->getCharacteristic(GATE_CHARACTERISTIC_UUID)->setValue("");
//             pCharacteristic->setValue("");
//         }
//     }
//
//     void onStatus(NimBLECharacteristic* pCharacteristic, int code) override {
//         Log.logf("Notification/Indication return code: %d, %s\n",
//             code,
//             NimBLEUtils::returnCodeToString(code)
//         );
//     }
//
//     /** Peer subscribed to notifications/indications */
//     void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
//         std::string str  = "Client ID: ";
//         str             += std::to_string(connInfo.getConnHandle());
//         str             += " Address: ";
//         str             += connInfo.getAddress().toString();
//         if (subValue == 0) {
//             str += " Unsubscribed to ";
//         } else if (subValue == 1) {
//             str += " Subscribed to notifications for ";
//         } else if (subValue == 2) {
//             str += " Subscribed to indications for ";
//         } else if (subValue == 3) {
//             str += " Subscribed to notifications and indications for ";
//         } else {
//             str += " ¯\\_('')_/¯";
//         }
//         str += std::string(pCharacteristic->getUUID());
//
//         Log.logf("%s\n", str.c_str());
//     }
// } chrCallbacks;

// class DescriptorCallbacks final : public NimBLEDescriptorCallbacks {
//     void onWrite(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) override {
//         std::string dscVal = pDescriptor->getValue();
//         Log.logf("Descriptor written value: %s\n", dscVal.c_str());
//     }
//
//     void onRead(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) override {
//         Log.logf("%s Descriptor read\n", pDescriptor->getUUID().toString().c_str());
//     }
// } dscCallbacks;
//
// bool BLEClass::isConnected() {
//     if (_bluetoothParams.pServer == nullptr)
//         return false;
//     return _bluetoothParams.pServer->getConnectedCount() > 0;
// }
//
// int BLEClass::startBLEServer() {
//     NimBLEDevice::init("");
//
//     NimBLEDevice::setSecurityAuth(
//         BLE_SM_PAIR_AUTHREQ_BOND |  // Enable bonding
//         BLE_SM_PAIR_AUTHREQ_MITM |  // Enable Man-In-The-Middle protection
//         BLE_SM_PAIR_AUTHREQ_SC      // Enable Secure Connections
//     );
//
//     NimBLEDevice::setSecurityPasskey(PASSCODE);  // Set a static passkey (optional)
//     NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO);
//
//     _bluetoothParams.pServer = NimBLEDevice::createServer();
//     // _bluetoothParams.pServer->setCallbacks(&serverCallbacks, NULL);
//     _bluetoothParams.pService = _bluetoothParams.pServer->createService(GATE_SERVICE_UUID);
//
//     _bluetoothParams.pGateCharacteristic = _bluetoothParams.pService->createCharacteristic(
//                         GATE_CHARACTERISTIC_UUID,
//                         NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::WRITE_ENC
//                       );
//
//     _bluetoothParams.pGateCharacteristic->setValue("");
//     // _bluetoothParams.pGateCharacteristic->setCallbacks(&chrCallbacks);
//
//     _bluetoothParams.pGateAckCharacteristic = _bluetoothParams.pService->createCharacteristic(
//                     GATE_ACK_CHARACTERISTIC_UUID,
//                     NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::WRITE_ENC
//                   );
//
//     _bluetoothParams.pGateAckCharacteristic->setValue("");
//     // _bluetoothParams.pGateAckCharacteristic->setCallbacks(&chrCallbacks);
//     _bluetoothParams.pService->start();
//
//     NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
//     pAdvertising->addServiceUUID(GATE_SERVICE_UUID);
//     pAdvertising->setName(SERVER_NAME);
//     pAdvertising->start();
//
//     // TaskHandle_t handle;
//     // if (!xTaskCreate(bluetoothLoop,
//     //                  "BLEloop",
//     //                  4 * 1024,
//     //                  &_bluetoothParams,
//     //                  1,
//     //                  &handle))
//     //     Log.logf("Failed to start BLEloop task");
//
//     return 0;
// }
//
// void BLEClass::sendMessage(const std::string& msg) {
//     if (!isConnected()) {
//         Log.logf("BLE connection not established\n");
//         return;
//     }
//     NimBLECharacteristic *pCharacteristic = _bluetoothParams.pGateCharacteristic;
//     if (pCharacteristic == nullptr) {
//         Log.logf("Characteristic not found\n");
//         return;
//     }
//
//     pCharacteristic->setValue(msg.c_str());
//     if (!pCharacteristic->notify()) {
//         Log.logf("Characteristic notify failed\n");
//     }
//
//     Log.logf("Characteristic notified\n");
//     delay(1000);
//     pCharacteristic->setValue("");
// }

// _Noreturn void BLEClass::bluetoothLoop(void * pvParameters) {
//     auto p = static_cast<BluetoothParams*>(pvParameters);
//     for(;;) {
//         delay(2000);
//
// //        if (p->pServer->getConnectedCount()) {
// //            NimBLEService* pSvc = p->pServer->getServiceByUUID(SERVICE_UUID);
// //            if (pSvc) {
// //                NimBLECharacteristic* pChr = pSvc->getCharacteristic(CHARACTERISTIC_UUID);
// //                if (pChr) {
// //                    // pChr->setValue("Fries");
// //                    Serial.println((std::string("Notify ") + (pChr->notify() ? std::string("success") : std::string("failed"))).c_str());
// //                }
// //            }
// //        }
//     }
// }