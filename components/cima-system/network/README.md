# Network
...
## Wifi Manager

According to ESP IDF 6 [wifi example](https://github.com/espressif/esp-idf/blob/master/examples/common_components/protocol_examples_common/wifi_connect.c#L109)
the order shall be

```cpp
//WIFI start
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    // Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask)
    esp_netif_config.if_desc = EXAMPLE_NETIF_DESC_STA;
    esp_netif_config.route_prio = 128;
    s_example_sta_netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

//WIFI connect
    wifi_config_t wifi_config = {
        .sta = {

            .ssid = CONFIG_EXAMPLE_WIFI_SSID,
            .password = CONFIG_EXAMPLE_WIFI_PASSWORD,

            .scan_method = EXAMPLE_WIFI_SCAN_METHOD,
            .sort_method = EXAMPLE_WIFI_CONNECT_AP_SORT_METHOD,
            .threshold.rssi = CONFIG_EXAMPLE_WIFI_SCAN_RSSI_THRESHOLD,
            .threshold.authmode = EXAMPLE_WIFI_SCAN_AUTH_MODE_THRESHOLD,
        },
    };
// DO connect

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
        WIFI_EVENT_STA_DISCONNECTED, &example_handler_on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, 
        IP_EVENT_STA_GOT_IP, &example_handler_on_sta_got_ip, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,     
        WIFI_EVENT_STA_CONNECTED, &example_handler_on_wifi_connect, s_example_sta_netif));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_err_t ret = esp_wifi_connect();
    if (ret != ESP_OK) {
        return ret;
    }
    return ESP_OK;
```

## Modem

![alt text](https://upload.wikimedia.org/wikipedia/commons/thumb/2/2a/EIA_232_DTE_DCE_DCE_DTE.png/500px-EIA_232_DTE_DCE_DCE_DTE.png "Modem network topology")
_[Data circuit-terminating equipment](https://en.wikipedia.org/wiki/Data_circuit-terminating_equipment)_

- _DCE data circuit equipement_: An actual Modem -- and interface betwen carrier physical language and a computer physical language
- _DTE data teminal equipment_: A computer

**NOTE:** ESP-IDF introduces two common GSM modems (BG96, SIM800L). However I have M590E which needs to be introduced.

### Neoway M590E

(https://cdn.instructables.com/ORIG/FX6/UAHQ/IZ6CYGIU/FX6UAHQIZ6CYGIU.pdf)

### PPPoS

Example at [PPP over Serial (PPPoS) client example](https://github.com/espressif/esp-idf/tree/master/examples/protocols/pppos_client) Introduces PPPoS modem functionality and shows how to utilize it, but references available GSM modems (BG96, SIM800L). Introduciton of new modem is inevitable.

#### Constants

- *CONFIG_EXAMPLE_UART_RX_BUFFER_SIZE* (can be variable)
- *CONFIG_EXAMPLE_UART_TX_BUFFER_SIZE* (can be variable)

- *CONFIG_EXAMPLE_UART_PATTERN_QUEUE_SIZE* (can be variable)

- *CONFIG_EXAMPLE_UART_EVENT_QUEUE_SIZE* (can be variable)
- *CONFIG_EXAMPLE_UART_EVENT_TASK_STACK_SIZE* (can be variable)
- *CONFIG_EXAMPLE_UART_EVENT_TASK_PRIORITY* (can be variable)

- *CONFIG_EXAMPLE_UART_MODEM_TX_PIN* (can be variable)
- *CONFIG_EXAMPLE_UART_MODEM_RX_PIN* (can be variable)
- *CONFIG_EXAMPLE_UART_MODEM_RTS_PIN* (can be variable)
- *CONFIG_EXAMPLE_UART_MODEM_CTS_PIN* (can be variable)

- *CONFIG_EXAMPLE_MODEM_APN* (can be variable)

- *CONFIG_EXAMPLE_MODEM_PPP_AUTH_USERNAME* (can be variable)
- *CONFIG_EXAMPLE_MODEM_PPP_AUTH_PASSWORD* (can be variable)
