#define ST(A) #A
#define STR(A) ST(A)

#ifdef WIFI_SSID
    #pragma message STR(WIFI_SSID)
#else
    #warning "WIFI_SSID NOT defined"
#endif

#ifdef WIFI_PASSWORD
    #pragma message STR(WIFI_PASSWORD)
#else
    #warning "WIFI_PASSWORD NOT defined"
#endif