// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

// An example that implements the light bulb HomeKit profile. It can serve as a basic implementation for
// any platform. The accessory logic implementation is reduced to internal state updates and log output.
//
// This implementation is platform-independent.
//
// The code consists of multiple parts:
//
//   1. The definition of the accessory configuration and its internal state.
//
//   2. Helper functions to load and save the state of the accessory.
//
//   3. The definitions for the HomeKit attribute database.
//
//   4. The callbacks that implement the actual behavior of the accessory, in this
//      case here they merely access the global accessory state variable and write
//      to the log to make the behavior easily observable.
//
//   5. The initialization of the accessory state.
//
//   6. Callbacks that notify the server in case their associated value has changed.

#include "HAP.h"

#include "App.h"
#include "DB.h"
#include <stdio.h>
#include "driver/gpio.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Domain used in the key value store for application data.
 *
 * Purged: On factory reset.
 */
#define kAppKeyValueStoreDomain_Configuration ((HAPPlatformKeyValueStoreDomain) 0x00)

/**
 * Key used in the key value store to store the configuration state.
 *
 * Purged: On factory reset.
 */
#define kAppKeyValueStoreKey_Configuration_State ((HAPPlatformKeyValueStoreDomain) 0x00)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Global accessory configuration.
 */
typedef struct {
    struct {
        bool lightBulbOn[3];
        int  lightBulbBrightness;
    } state;
    HAPAccessoryServerRef* server;
    HAPPlatformKeyValueStoreRef keyValueStore;
} AccessoryConfiguration;

static AccessoryConfiguration accessoryConfiguration;

//----------------------------------------------------------------------------------------------------------------------

/**
 * Load the accessory state from persistent memory.
 */
static void LoadAccessoryState(void) {
    HAPPrecondition(accessoryConfiguration.keyValueStore);

    HAPError err;

    // Load persistent state if available
    bool found;
    size_t numBytes;

    err = HAPPlatformKeyValueStoreGet(
            accessoryConfiguration.keyValueStore,
            kAppKeyValueStoreDomain_Configuration,
            kAppKeyValueStoreKey_Configuration_State,
            &accessoryConfiguration.state,
            sizeof accessoryConfiguration.state,
            &numBytes,
            &found);

    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPFatalError();
    }
    if (!found || numBytes != sizeof accessoryConfiguration.state) {
        if (found) {
            HAPLogError(&kHAPLog_Default, "Unexpected app state found in key-value store. Resetting to default.");
        }
        HAPRawBufferZero(&accessoryConfiguration.state, sizeof accessoryConfiguration.state);
    }
}

/**
 * Save the accessory state to persistent memory.
 */
static void SaveAccessoryState(void) {
    HAPPrecondition(accessoryConfiguration.keyValueStore);

    HAPError err;
    err = HAPPlatformKeyValueStoreSet(
            accessoryConfiguration.keyValueStore,
            kAppKeyValueStoreDomain_Configuration,
            kAppKeyValueStoreKey_Configuration_State,
            &accessoryConfiguration.state,
            sizeof accessoryConfiguration.state);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPFatalError();
    }
}

//----------------------------------------------------------------------------------------------------------------------

/**
 * HomeKit accessory that provides the Light Bulb service.
 *
 * Note: Not constant to enable BCT Manual Name Change.
 */
static HAPAccessory bridgeAccessory = { .aid = 1,
                                        .category = kHAPAccessoryCategory_Bridges,
                                        .name = "Acme Bridge",
                                        .manufacturer = "Acme",
                                        .model = "Bridge1,1",
                                        .serialNumber = "ABCDEFGH0001",
                                        .firmwareVersion = "1",
                                        .hardwareVersion = "1",
                                        .services = (const HAPService* const[]) { &bridgeAccessoryInformationService,
                                                                                  &hapProtocolInformationService,
                                                                                  &pairingService,
                                                                                  NULL },
                                        .callbacks = { .identify = IdentifyAccessory } };

static HAPAccessory lightBulbAccessory = { .aid = 2,
                                           .category = kHAPAccessoryCategory_BridgedAccessory,
                                           .name = "Acme Light Bulb",
                                           .manufacturer = "Acme",
                                           .model = "LightBulb1,1",
                                           .serialNumber = "ABCDEFGH0002",
                                           .firmwareVersion = "1",
                                           .hardwareVersion = "1",
                                           .services = (const HAPService* const[]) { &lightBulbAccessoryInformationService,
                                                                                     &lightBulbService,
                                                                                     NULL },
                                           .callbacks = { .identify = IdentifyAccessory } };

static HAPAccessory lightBulbAccessoryB = { .aid = 3,
                                           .category = kHAPAccessoryCategory_BridgedAccessory,
                                           .name = "Acme Light Bulb",
                                           .manufacturer = "Acme",
                                           .model = "LightBulb1,1",
                                           .serialNumber = "ABCDEFGH0002",
                                           .firmwareVersion = "1",
                                           .hardwareVersion = "1",
                                           .services = (const HAPService* const[]) { &lightBulbAccessoryInformationService,
                                                                                     &lightBulbService,
                                                                                     NULL },
                                           .callbacks = { .identify = IdentifyAccessory } };

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
HAPError IdentifyAccessory(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPAccessoryIdentifyRequest* request HAP_UNUSED,
        void* _Nullable context HAP_UNUSED) {
    HAPLogInfo(&kHAPLog_Default, "%s", __func__);
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleLightBulbOnRead(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPBoolCharacteristicReadRequest* request HAP_UNUSED,
        bool* value,
        void* _Nullable context HAP_UNUSED) {
	uint64_t i = request->accessory->aid;
    *value = accessoryConfiguration.state.lightBulbOn[i];
    switch(i)
    {
    case 2:
    	gpio_pad_select_gpio(26);
    	gpio_set_direction(26, GPIO_MODE_OUTPUT);
    	gpio_set_level(26, !accessoryConfiguration.state.lightBulbOn[2]);
    break;

    case 3:
    	gpio_pad_select_gpio(27);
    	gpio_set_direction(27, GPIO_MODE_OUTPUT);
    	gpio_set_level(27, !accessoryConfiguration.state.lightBulbOn[3]);
    	break;
}
    HAPLogInfo(&kHAPLog_Default, "%s: %s", __func__, *value ? "true" : "false");

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleLightBulbOnWrite(
        HAPAccessoryServerRef* server,
        const HAPBoolCharacteristicWriteRequest* request,
        bool value,
        void* _Nullable context HAP_UNUSED) {
    HAPLogInfo(&kHAPLog_Default, "%s: %s", __func__, value ? "true" : "false");
    	printf("Hello my master I'm alive!\n");
    	uint64_t aid = request->accessory->aid;
//    	printf("%llu\n", aid);
        printf("Hello my master I'm alive!\n");
    if (accessoryConfiguration.state.lightBulbOn[aid] != value) {
        accessoryConfiguration.state.lightBulbOn[aid] = value;
        switch(aid)
        {
        case 2:
        	gpio_pad_select_gpio(26);
        	gpio_set_direction(26, GPIO_MODE_OUTPUT);
        	gpio_set_level(26, !value);
        break;

        case 3:
        	gpio_pad_select_gpio(27);
        	gpio_set_direction(27, GPIO_MODE_OUTPUT);
        	gpio_set_level(27, !value);
        	break;
    }

        SaveAccessoryState();

        HAPAccessoryServerRaiseEvent(server, request->characteristic, request->service, request->accessory);
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleLightBulbBrightnessRead(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPIntCharacteristicReadRequest* request HAP_UNUSED,
        int* value,
        void* _Nullable context HAP_UNUSED) {
    *value = accessoryConfiguration.state.lightBulbBrightness;
    HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, *value);

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleLightBulbBrightnessWrite(
        HAPAccessoryServerRef* server,
        const HAPIntCharacteristicWriteRequest* request,
        int value,
        void* _Nullable context HAP_UNUSED) {
    HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, value);
    if (accessoryConfiguration.state.lightBulbBrightness != value) {
        accessoryConfiguration.state.lightBulbBrightness = value;

        SaveAccessoryState();

        HAPAccessoryServerRaiseEvent(server, request->characteristic, request->service, request->accessory);
    }

    return kHAPError_None;
}

//----------------------------------------------------------------------------------------------------------------------

void AccessoryNotification(
        const HAPAccessory* accessory,
        const HAPService* service,
        const HAPCharacteristic* characteristic,
        void* ctx) {
    HAPLogInfo(&kHAPLog_Default, "Accessory Notification");

    HAPAccessoryServerRaiseEvent(accessoryConfiguration.server, characteristic, service, accessory);
}

void AppCreate(HAPAccessoryServerRef* server, HAPPlatformKeyValueStoreRef keyValueStore) {
    HAPPrecondition(server);
    HAPPrecondition(keyValueStore);

    HAPLogInfo(&kHAPLog_Default, "%s", __func__);

    HAPRawBufferZero(&accessoryConfiguration, sizeof accessoryConfiguration);
    accessoryConfiguration.server = server;
    accessoryConfiguration.keyValueStore = keyValueStore;
    LoadAccessoryState();
}

void AppRelease(void) {
}

const HAPAccessory* _Nullable const* _Nullable bridgedAccessories =
    (const HAPAccessory* const[]) { &lightBulbAccessory, &lightBulbAccessoryB,
                                    NULL };

void AppAccessoryServerStart(void) {
    HAPAccessoryServerStartBridge(
        accessoryConfiguration.server, &bridgeAccessory,
        bridgedAccessories, true
    );
}

//----------------------------------------------------------------------------------------------------------------------

void AccessoryServerHandleUpdatedState(HAPAccessoryServerRef* server, void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(!context);

    switch (HAPAccessoryServerGetState(server)) {
        case kHAPAccessoryServerState_Idle: {
            HAPLogInfo(&kHAPLog_Default, "Accessory Server State did update: Idle.");
            return;
        }
        case kHAPAccessoryServerState_Running: {
            HAPLogInfo(&kHAPLog_Default, "Accessory Server State did update: Running.");
            return;
        }
        case kHAPAccessoryServerState_Stopping: {
            HAPLogInfo(&kHAPLog_Default, "Accessory Server State did update: Stopping.");
            return;
        }
    }
    HAPFatalError();
}

const HAPAccessory* AppGetAccessoryInfo() {
    return &bridgeAccessory;
}

void AppInitialize(
        HAPAccessoryServerOptions* hapAccessoryServerOptions,
        HAPPlatform* hapPlatform,
        HAPAccessoryServerCallbacks* hapAccessoryServerCallbacks) {
    /*no-op*/
}

void AppDeinitialize() {
    /*no-op*/
}
