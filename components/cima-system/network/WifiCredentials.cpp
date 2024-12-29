#include <system/network/WifiCredentials.h>

namespace cima::system::network {

WifiCredentials::WifiCredentials(const std::string &ssid, const std::string &passphrase)
    : ssid(ssid), passphrase(passphrase) {
}

const std::string &WifiCredentials::getSsid(){
    return ssid;
}

const std::string &WifiCredentials::getPassphrase(){
    return passphrase;
}

}