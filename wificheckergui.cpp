#define UNICODE

#include <windows.h>
#include <wlanapi.h>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

#pragma comment(lib, "wlanapi.lib")

// Globale Variable für die Fensterfarbe
COLORREF windowColor = RGB(0, 255, 0); // Standard: Grün
HWND hwndGlobal = NULL; // Globales Handle für das Fenster

// Funktion, um den WLAN-Status zu überprüfen
void checkWifiConnection() {
    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2;
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

    // Standardfarbe: Grün (nicht verbunden)
    COLORREF newColor = RGB(0, 255, 0);

    for (unsigned int i = 0; i < pIfList->dwNumberOfItems; i++) {
        PWLAN_INTERFACE_INFO pIfInfo = &pIfList->InterfaceInfo[i];

        // Wenn das Interface verbunden ist, setze die Farbe auf Rot
        if (pIfInfo->isState == wlan_interface_state_connected) {
            newColor = RGB(255, 0, 0); // Rot
            break;
        }
    }

    // Fensterfarbe nur ändern, wenn sie sich ändert
    if (newColor != windowColor) {
        windowColor = newColor;

        // Fenster neu zeichnen
        if (hwndGlobal != NULL) {
            InvalidateRect(hwndGlobal, NULL, TRUE);
        }
    }

    // Speicher freigeben
    if (pIfList != NULL) {
        WlanFreeMemory(pIfList);
    }
    WlanCloseHandle(hClient, NULL);
}

// Fensterprozedur
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Hintergrundfarbe setzen
            HBRUSH brush = CreateSolidBrush(windowColor);
            FillRect(hdc, &ps.rcPaint, brush);
            DeleteObject(brush);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Hintergrund-Thread, um die Verbindung alle 5 Sekunden zu prüfen
void connectionCheckThread() {
    while (true) {
        checkWifiConnection();
        std::this_thread::sleep_for(std::chrono::seconds(5)); // 5 Sekunden warten
    }
}

int main() {
    // Fensterklasse registrieren
    const wchar_t CLASS_NAME[] = L"WLAN Status Fenster";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Fenster erstellen
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"WLAN Verbindungsstatus",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 100,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    // Globales Handle setzen
    hwndGlobal = hwnd;

    // Fenster immer im Vordergrund halten
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

    ShowWindow(hwnd, SW_SHOW);

    // Verbindung in einem separaten Thread prüfen
    std::thread checkerThread(connectionCheckThread);
    checkerThread.detach(); // Hintergrund-Thread unabhängig laufen lassen

    // Nachrichten-Schleife
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
