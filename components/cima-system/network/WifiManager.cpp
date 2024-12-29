#include <system/network/WifiManager.h>
#include <functional>
#include <cstring>
#include <thread>

#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>

#include <lwip/err.h>
#include <lwip/sys.h>

#define EXAMPLE_ESP_MAXIMUM_RETRY  10

namespace cima::system::network {

    Log WifiManager::LOG("WifiManager");

    WifiManager::WifiManager() 
        : started(false), networkIterator(networks.end()) {
    }

    void WifiManager::addNetwork(const WifiCredentials &credentials){
        this->credentials.push_back(credentials);
        networkIterator = this->credentials.begin();
    }

    void WifiManager::start(){

        if(networkIterator == this->credentials.end()) {
            LOG.error("No networks defined.");
            return;
        }

        LOG.info("Connecting...");

        tcpip_adapter_init();

        ESP_ERROR_CHECK(esp_event_loop_create_default());
        ESP_ERROR_CHECK(esp_wifi_init(&firmwareWifiConfig));

        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiManager::wifiEventHandlerWrapper, this));
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiManager::ipEventHandlerWrapper, this));
        
        //TODO now for try - latermaybe split into individual methods since it cant register for any
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP, &WifiManager::ipEventHandlerWrapper, this));

        initAccesspoint();
    }

    void WifiManager::initAccesspoint(){
        if(networkIterator == this->credentials.end()) {
            LOG.info("End of Network list reached. Starting over.");
            networkIterator = this->credentials.begin();
        }
        initWifiStationConfig(networkIterator->getSsid(), networkIterator->getPassphrase());
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig) );
        ESP_ERROR_CHECK(esp_wifi_start());
        started = true;

        LOG.info("Wifi station started.");
    }

    void WifiManager::wifiEventHandler(int32_t event_id, void* event_data) {
        if (event_id == WIFI_EVENT_STA_START) {
            auto connectresult = esp_wifi_connect();
            LOG.info("Connection attempt %d ended with code: %d", connectionAttempts, connectresult);
        } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            if (connectionAttempts < EXAMPLE_ESP_MAXIMUM_RETRY) {
                auto connectresult = esp_wifi_connect();
                LOG.info("Connection attempt %d ended with code: %d", connectionAttempts, connectresult);

                auto kickReason = (wifi_event_sta_disconnected_t *)event_data;
                if(kickReason) {
                    LOG.info("Kick reason: %d", kickReason->reason);
                    if(kickReason->reason == WIFI_REASON_NO_AP_FOUND){
                        LOG.info("Network %s not found. Trying next one.", networkIterator->getSsid().c_str());
                        tryNextNetwork();
                    }
                }

                connectionAttempts++;
                LOG.info("Retrying to connect to the AP");
            } else {
                LOG.info("Connectin to the AP '%s' has failed.", networkIterator->getSsid().c_str());

                if(networkIterator != networks.end()){
                    LOG.info("Trying another network.");
                    tryNextNetwork();
                } else {
                    LOG.error("No more networks defined in list.");
                }
                
            }

            //Signalize transition to connectivity lost state
            if (connected) {
                auto signalThread = std::thread([&](){networkDownSignal();});
                signalThread.join();
            }
            connected = false;
        }
    }

    void WifiManager::tryNextNetwork() {
        ++networkIterator;
        connectionAttempts = 0;
        ESP_ERROR_CHECK(esp_wifi_stop());
        initAccesspoint();
    }

    void WifiManager::ipEventHandler(int32_t event_id, void* event_data) {
        if (event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            LOG.info("got ip:" IPSTR, IP2STR(&event->ip_info.ip));
            connectionAttempts = 0;
            connected = true;
            auto signalThread = std::thread([&](){networkUpSignal();});
            signalThread.join();

        } else if (event_id == IP_EVENT_STA_LOST_IP) {
            LOG.info("Wi-Fi connectivity lost");
            connectionAttempts = 0;
            connected = false;
            auto signalThread = std::thread([&](){networkDownSignal();});
            signalThread.join();
        } else {
            LOG.info("Unhandled IP event occured: %d", event_id);
        }
    }

    bool WifiManager::isStarted(){
        return started;
    }

    bool WifiManager::isConnected(){
        return connected;
    }

    void WifiManager::wifiEventHandlerWrapper(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
            ((WifiManager *)arg)->wifiEventHandler(event_id, event_data);
    }

    void WifiManager::ipEventHandlerWrapper(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
            ((WifiManager *)arg)->ipEventHandler(event_id, event_data);
    }

    void WifiManager::initWifiStationConfig(const std::string &ssid, const std::string &password) {
        
        wifi_sta_config_t &wifi_config_sta = wifiConfig.sta;
        std::strcpy((char *)wifi_config_sta.ssid,ssid.c_str());
        std::strcpy((char *)wifi_config_sta.password, password.c_str());

        wifi_config_sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
        wifi_config_sta.bssid_set = false;
        wifi_config_sta.channel = 0;
        wifi_config_sta.listen_interval = 0;
        wifi_config_sta.sort_method = WIFI_CONNECT_AP_BY_SECURITY; 

        LOG.info("Setup to connect to AP SSID:%s password:%s", (char *)wifiConfig.sta.ssid, (char *)wifiConfig.sta.password);
    }
}