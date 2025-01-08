#include <windows.h>
#include <wlanapi.h>
#include <iostream>
#include <string>

#pragma comment(lib, "wlanapi.lib")

void checkWifiConnection() {
    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2; // API-Version
    DWORD dwCurVersion = 0;
    DWORD dwResult = 0;
    PWLAN_INTERFACE_INFO_LIST pIfList = NULL;

    // WLAN-Client initialisieren
    dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        std::cerr << "Fehler: WlanOpenHandle fehlgeschlagen (" << dwResult << ")" << std::endl;
        return;
    }

    // WLAN-Interfaces abrufen
    dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
    if (dwResult != ERROR_SUCCESS) {
        std::cerr << "Fehler: WlanEnumInterfaces fehlgeschlagen (" << dwResult << ")" << std::endl;
        WlanCloseHandle(hClient, NULL);
        return;
    }

    // Informationen zu den Interfaces durchlaufen
    for (unsigned int i = 0; i < pIfList->dwNumberOfItems; i++) {
        PWLAN_INTERFACE_INFO pIfInfo = &pIfList->InterfaceInfo[i];

        std::wcout << L"Interface Name: " << pIfInfo->strInterfaceDescription << std::endl;

        // Verbindungsstatus prüfen
        if (pIfInfo->isState == wlan_interface_state_connected) {
            std::wcout << L"Status: Verbunden mit einem Netzwerk." << std::endl;
        } else {
            std::wcout << L"Status: Nicht verbunden." << std::endl;
        }
    }

    // Speicher freigeben
    if (pIfList != NULL) {
        WlanFreeMemory(pIfList);
        pIfList = NULL;
    }

    // Handle schließen
    WlanCloseHandle(hClient, NULL);
}

int main() {
    checkWifiConnection();
    return 0;
}
