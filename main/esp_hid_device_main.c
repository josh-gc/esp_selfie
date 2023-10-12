/* This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this software is
   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
// #if CONFIG_BT_BLE_ENABLED
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"
//#endif
#include "esp_bt_main.h"
#include "esp_bt_device.h"

#include "esp_hidd.h"
#include "esp_hid_gap.h"

#include "esp_check.h"		// I added this for ESP_CHECK...

static const char *TAG = "esp_hid_device_main";

typedef struct
{
    xTaskHandle task_hdl;
    esp_hidd_dev_t *hid_dev;
    uint8_t protocol_mode;
    uint8_t *buffer;
} local_param_t;

// #if CONFIG_BT_BLE_ENABLED
static local_param_t s_ble_hid_param = {0};
const unsigned char hidapiReportMap[] = { //8 bytes input, 8 bytes feature
    0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
    0x0A, 0x00, 0x01,  // Usage (0x0100)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x08,        //   Report Count (8)
    0x09, 0x01,        //   Usage (0x01)
    0x82, 0x02, 0x01,  //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Buffered Bytes)
    0x95, 0x08,        //   Report Count (8)
    0x09, 0x02,        //   Usage (0x02)
    0xB2, 0x02, 0x01,  //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile,Buffered Bytes)
    0x95, 0x08,        //   Report Count (8)
    0x09, 0x03,        //   Usage (0x03)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection

    // 38 bytes
};

const unsigned char mediaReportMap[] = {
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x03,        //   Report ID (3)
    0x09, 0x02,        //   Usage (Numeric Key Pad)
    0xA1, 0x02,        //   Collection (Logical)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x0A,        //     Usage Maximum (0x0A)
    0x15, 0x01,        //     Logical Minimum (1)
    0x25, 0x0A,        //     Logical Maximum (10)
    0x75, 0x04,        //     Report Size (4)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x00,        //     Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0x05, 0x0C,        //   Usage Page (Consumer)
    0x09, 0x86,        //   Usage (Channel)
    0x15, 0xFF,        //   Logical Minimum (-1)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x02,        //   Report Size (2)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x46,        //   Input (Data,Var,Rel,No Wrap,Linear,Preferred State,Null State)
    0x09, 0xE9,        //   Usage (Volume Increment)
    0x09, 0xEA,        //   Usage (Volume Decrement)
    0x15, 0x00,        //   Logical Minimum (0)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0xE2,        //   Usage (Mute)
    0x09, 0x30,        //   Usage (Power)
    0x09, 0x83,        //   Usage (Recall Last)
    0x09, 0x81,        //   Usage (Assign Selection)
    0x09, 0xB0,        //   Usage (Play)
    0x09, 0xB1,        //   Usage (Pause)
    0x09, 0xB2,        //   Usage (Record)
    0x09, 0xB3,        //   Usage (Fast Forward)
    0x09, 0xB4,        //   Usage (Rewind)
    0x09, 0xB5,        //   Usage (Scan Next Track)
    0x09, 0xB6,        //   Usage (Scan Previous Track)
    0x09, 0xB7,        //   Usage (Stop)
    0x15, 0x01,        //   Logical Minimum (1)
    0x25, 0x0C,        //   Logical Maximum (12)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x80,        //   Usage (Selection)
    0xA1, 0x02,        //   Collection (Logical)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x03,        //     Usage Maximum (0x03)
    0x15, 0x01,        //     Logical Minimum (1)
    0x25, 0x03,        //     Logical Maximum (3)
    0x75, 0x02,        //     Report Size (2)
    0x81, 0x00,        //     Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
};

static esp_hid_raw_report_map_t ble_report_maps[] = {
    {
        .data = hidapiReportMap,
        .len = sizeof(hidapiReportMap)
    },
    {
        .data = mediaReportMap,
        .len = sizeof(mediaReportMap)
    }
};

static esp_hid_device_config_t ble_hid_config = {
    .vendor_id          = 0x16C0,
    .product_id         = 0x05DF,
    .version            = 0x0100,
    .device_name        = "ESP BLE HID2",
    .manufacturer_name  = "Espressif",
    .serial_number      = "1234567890",
    .report_maps        = ble_report_maps,
    .report_maps_len    = 2
};

// Volume up/down bits
#define HID_CC_RPT_VOLUME_UP            0x40
#define HID_CC_RPT_VOLUME_DOWN          0x80

// HID Consumer Control report bitmasks
#define HID_CC_RPT_VOLUME_BITS          0x3F

// Macros for the HID Consumer Control 2-byte report
#define HID_CC_RPT_SET_VOLUME_UP(s)     (s)[0] &= HID_CC_RPT_VOLUME_BITS;    (s)[0] |= HID_CC_RPT_VOLUME_UP
#define HID_CC_RPT_SET_VOLUME_DOWN(s)   (s)[0] &= HID_CC_RPT_VOLUME_BITS;    (s)[0] |= HID_CC_RPT_VOLUME_DOWN

// HID Consumer Usage IDs (subset of the codes available in the USB HID Usage Tables spec)
#define HID_CONSUMER_VOLUME_UP      233 // Volume Increment
#define HID_CONSUMER_VOLUME_DOWN    234 // Volume Decrement

#define HID_RPT_ID_CC_IN        3   // Consumer Control input report ID
#define HID_CC_IN_RPT_LEN       2   // Consumer Control input report Len

/********************	   Bluetooth defines		********************/
static local_param_t s_bt_hid_param = {0};
const unsigned char mouseReportMap[] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)

    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)

    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)

    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x38,                    //     USAGE (Wheel)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)

    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
};

static esp_hid_raw_report_map_t bt_report_maps[] = {
    {
        .data = mouseReportMap,
        .len = sizeof(mouseReportMap)
    },
};

static esp_hid_device_config_t bt_hid_config = {
    .vendor_id          = 0x16C0,
    .product_id         = 0x05DF,
    .version            = 0x0100,
    .device_name        = "ESP BT HID1",
    .manufacturer_name  = "Espressif",
    .serial_number      = "1234567890",
    .report_maps        = bt_report_maps,
    .report_maps_len    = 1
};

// Enum to track the HID connection status
typedef enum {
	HID_IS_DISCONNECTED,
	HID_IS_BLE,
	HID_IS_BT
} hid_connection_status_t;

/********************	  Forward declarations		********************/
// Callbacks for BLE And BT Classic
static void ble_hidd_event_callback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
static void bt_hidd_event_callback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);

// HID Task
hid_connection_status_t	hid_connection_status = HID_IS_DISCONNECTED;	// Flag to track if we are connected (and how)

static void	run_example( void* arguments );			// Task to receive "button" presses
static bool	_run_example_running = false;			// Flag to check if it's still running

// Send commands tasks:
static void send_volume_up();
static void send_volume_down();

// Task to send the HID value
void 		esp_hidd_send_consumer_value(uint8_t key_cmd, bool key_pressed);

/********************	      Main Script			********************/
// TODO: Make this glamcor_bluetooth_hid_init()
void app_main(void)
{
	esp_err_t 			ret = ESP_FAIL;
	static const char* 	TAG = "app_main";

	ESP_LOGI(TAG, "app_main");

	// Init NVS TODO: Remove this when moving to actual code
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

	// Set HID gap init
	ESP_GOTO_ON_ERROR( esp_hid_gap_init(HID_DEV_MODE), error, TAG, "esp_hid_gap_init failed" );

	// Init BLE
	ESP_GOTO_ON_ERROR( esp_hid_ble_gap_adv_init(ESP_HID_APPEARANCE_GENERIC, ble_hid_config.device_name)
						, error, TAG, "esp_hid_ble_gap_adv_init failed" );

	ESP_GOTO_ON_ERROR( esp_ble_gatts_register_callback(esp_hidd_gatts_event_handler)
						, error, TAG, "esp_ble_gatts_register_callback failed" );

	ESP_GOTO_ON_ERROR( esp_hidd_dev_init(&ble_hid_config, ESP_HID_TRANSPORT_BLE, ble_hidd_event_callback, &s_ble_hid_param.hid_dev)
						, error, TAG, "esp_hidd_dev_init failed" );

	// Init BT Classic
	// setting device name...
	esp_bt_dev_set_device_name(bt_hid_config.device_name);

	// setting cod major, peripheral
	esp_bt_cod_t cod;
	cod.major = ESP_BT_COD_MAJOR_DEV_PERIPHERAL;

	ESP_GOTO_ON_ERROR( esp_bt_gap_set_cod(cod, ESP_BT_SET_COD_MAJOR_MINOR), error, TAG, "esp_bt_gap_set_cod failed" );

	vTaskDelay(1000 / portTICK_PERIOD_MS);
	ESP_GOTO_ON_ERROR( esp_hidd_dev_init(&bt_hid_config, ESP_HID_TRANSPORT_BT, bt_hidd_event_callback, &s_bt_hid_param.hid_dev)
						, error, TAG, "esp_hidd_dev_init failed" );

	// Start xTask to testing
	xTaskCreate( run_example, "run_example", 2048, NULL, 10, NULL );

	return;

error:
	// Send error message
	ESP_LOGE( TAG, "app_main failed: ( %d )", ret );
}

/********************	     Private Functions 		********************/
// BLE Event callback
static void ble_hidd_event_callback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    esp_hidd_event_t 		event 	= (esp_hidd_event_t)id;
    esp_hidd_event_data_t 	*param 	= (esp_hidd_event_data_t *)event_data;
    static const char 		*TAG 	= "ble_hidd_event_callback";

	switch (event) {
		case ESP_HIDD_START_EVENT: {
			ESP_LOGI(TAG, "Setting to connectable, discoverable");
			esp_hid_ble_gap_adv_start();
			break;
		}
		case ESP_HIDD_CONNECT_EVENT: {
			ESP_LOGI(TAG, "CONNECTED");
			hid_connection_status = HID_IS_BLE;
			break;
		}
		case ESP_HIDD_PROTOCOL_MODE_EVENT: {
			ESP_LOGI(TAG, "PROTOCOL MODE[%u]: %s", param->protocol_mode.map_index, param->protocol_mode.protocol_mode ? "REPORT" : "BOOT");
			break;
		}
		case ESP_HIDD_CONTROL_EVENT: {
			ESP_LOGI(TAG, "CONTROL[%u]: %sSUSPEND", param->control.map_index, param->control.control ? "EXIT_" : "");
			break;
		}
		case ESP_HIDD_OUTPUT_EVENT: {
			ESP_LOGI(TAG, "OUTPUT[%u]: %8s ID: %2u, Len: %d, Data:", param->output.map_index, esp_hid_usage_str(param->output.usage), param->output.report_id, param->output.length);
			ESP_LOG_BUFFER_HEX(TAG, param->output.data, param->output.length);
			break;
		}
		case ESP_HIDD_FEATURE_EVENT: {
			ESP_LOGI(TAG, "FEATURE[%u]: %8s ID: %2u, Len: %d, Data:", param->feature.map_index, esp_hid_usage_str(param->feature.usage), param->feature.report_id, param->feature.length);
			ESP_LOG_BUFFER_HEX(TAG, param->feature.data, param->feature.length);
			break;
		}
		case ESP_HIDD_DISCONNECT_EVENT: {
			ESP_LOGI(TAG, "DISCONNECT: %s", esp_hid_disconnect_reason_str(esp_hidd_dev_transport_get(param->disconnect.dev), param->disconnect.reason));
			hid_connection_status = HID_IS_DISCONNECTED;
			ESP_LOGI(TAG, "Setting to connectable, discoverable again");
			esp_hid_ble_gap_adv_start();	// Start broadcasting again
			break;
		}
		case ESP_HIDD_STOP_EVENT: {
			ESP_LOGI(TAG, "STOP");
			break;
		}
		default:
			break;
    }
    return;
}

// BT Classic Event callback
static void bt_hidd_event_callback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
	esp_hidd_event_t event = (esp_hidd_event_t)id;
	esp_hidd_event_data_t *param = (esp_hidd_event_data_t *)event_data;
	static const char *TAG = "bt_hidd_event_callback";

	switch (event) {
	case ESP_HIDD_START_EVENT: {
		if (param->start.status == ESP_OK) {
			ESP_LOGI(TAG, "Setting to connectable, discoverable");
			esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
		} else {
			ESP_LOGE(TAG, "START failed!");
		}
		break;
	}
	case ESP_HIDD_CONNECT_EVENT: {
		if (param->connect.status == ESP_OK) {
			ESP_LOGI(TAG, "CONNECTED");
			esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
			hid_connection_status = HID_IS_BT;
		} else {
			ESP_LOGE(TAG, "CONNECT failed!");
		}
		break;
	}
	case ESP_HIDD_PROTOCOL_MODE_EVENT: {
		ESP_LOGI(TAG, "PROTOCOL MODE[%u]: %s", param->protocol_mode.map_index, param->protocol_mode.protocol_mode ? "REPORT" : "BOOT");
		break;
	}
	case ESP_HIDD_OUTPUT_EVENT: {
		ESP_LOGI(TAG, "OUTPUT[%u]: %8s ID: %2u, Len: %d, Data:", param->output.map_index, esp_hid_usage_str(param->output.usage), param->output.report_id, param->output.length);
		ESP_LOG_BUFFER_HEX(TAG, param->output.data, param->output.length);
		break;
	}
	case ESP_HIDD_FEATURE_EVENT: {
		ESP_LOGI(TAG, "FEATURE[%u]: %8s ID: %2u, Len: %d, Data:", param->feature.map_index, esp_hid_usage_str(param->feature.usage), param->feature.report_id, param->feature.length);
		ESP_LOG_BUFFER_HEX(TAG, param->feature.data, param->feature.length);
		break;
	}
	case ESP_HIDD_DISCONNECT_EVENT: {
		if (param->disconnect.status == ESP_OK) {
			ESP_LOGI(TAG, "DISCONNECTED");
			hid_connection_status = HID_IS_DISCONNECTED;
			ESP_LOGI(TAG, "Setting to connectable, discoverable again");
			esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
		} else {
			ESP_LOGE(TAG, "DISCONNECT failed!");
		}
		break;
	}
	case ESP_HIDD_STOP_EVENT: {
		ESP_LOGI(TAG, "STOP");
		break;
	}
	default:
		break;
	}
	return;
}

// Task to take user inputs to test the code.
static void run_example( void* arguments )
{
	ESP_LOGI( TAG, "run_example" );
	_run_example_running = true;
	char c;

	static const char* help_string = \
	"##############################\n"\
    "ESP HID DEVICE DEMO COMMANDS:\n"\
    "You can input these values\n"\
    "s -- Start pairing\n"\
	"e -- End pairing/disconnect\n"\
    "u -- Send volume up\n"\
    "d -- Send volume down\n"\
    "##############################\n";
	printf("%s\n", help_string);

	while(_run_example_running)
	{
		c = fgetc(stdin);
		if(c != 0xff)	// Catch for no input
		{
			switch (c) 
			{
				case 's':
					printf("TODO: Start Pairing\n");
					break;

				case 'e':
					printf("TODO: Stop Pairing/Disconnect\n");
					break;

				case 'u':
					send_volume_up();
					break;

				case 'd':
					send_volume_down();					
					break;

				case 'h':
					printf("%s\n", help_string);
					break;

				default:
					printf("Unknown character: %c\n", c);
					break;
			}
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	vTaskDelete( NULL );
}

// Function for sending the volume up command
static void send_volume_up()
{
	ESP_LOGI(TAG, "send_volume_up");

	if(hid_connection_status == HID_IS_DISCONNECTED)	// Confirm a device is connected
	{
		ESP_LOGE(TAG, "HID not connected!");
	} 
	else 
	{
		esp_hidd_send_consumer_value(HID_CONSUMER_VOLUME_UP, true);
		vTaskDelay(100 / portTICK_PERIOD_MS);
		esp_hidd_send_consumer_value(HID_CONSUMER_VOLUME_UP, false);
	}
}

// Function for sending the volume down command
static void send_volume_down()
{
	ESP_LOGI(TAG, "send_volume_down");

	if(hid_connection_status == HID_IS_DISCONNECTED)	// Confirm a device is connected
	{
		ESP_LOGE(TAG, "HID not connected!");
	} 
	else 
	{
		esp_hidd_send_consumer_value(HID_CONSUMER_VOLUME_DOWN, true);
		vTaskDelay(100 / portTICK_PERIOD_MS);
		esp_hidd_send_consumer_value(HID_CONSUMER_VOLUME_DOWN, false);
	}
}

// Send the keyboard value
void esp_hidd_send_consumer_value(uint8_t key_cmd, bool key_pressed)
{
    static const char* 	TAG = "esp_hidd_send_consumer_value";
	
	uint8_t buffer[HID_CC_IN_RPT_LEN] = {0, 0};
    if (key_pressed) {
        switch (key_cmd) {
			case HID_CONSUMER_VOLUME_UP:
				HID_CC_RPT_SET_VOLUME_UP(buffer);
				break;

			case HID_CONSUMER_VOLUME_DOWN:
				HID_CC_RPT_SET_VOLUME_DOWN(buffer);
				break;

			default:
				break;
        }
    }
	// Send the command
	if(hid_connection_status == HID_IS_BLE)
	{
		esp_hidd_dev_input_set(s_ble_hid_param.hid_dev, 1, HID_RPT_ID_CC_IN, buffer, HID_CC_IN_RPT_LEN);
	}
	else if(hid_connection_status == HID_IS_BT)
	{
		esp_hidd_dev_input_set(s_bt_hid_param.hid_dev, 1, HID_RPT_ID_CC_IN, buffer, HID_CC_IN_RPT_LEN);
	}
	else
	{
		ESP_LOGE(TAG, "HID not connected!");
	}
    
    return;
}