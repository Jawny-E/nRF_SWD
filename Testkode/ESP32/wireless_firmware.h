#pragma once

class WL_FIR{
  public:
    WL_FIR(const char* ssid, const char* password, const char* url);
    void wireless_connect();
    bool download_firmware();
  private:
    const char* _ssid;
    const char* _password;
    const char* _url;
};