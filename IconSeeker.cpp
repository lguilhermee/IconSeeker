#include <Windows.h>
#include <iostream>
#include <vector>

// Predefined hash values for specific applications
#define IDA_64_7_UP 0xe41dc94415badbe9
#define IDA_64_8_UP 0x84a07ad329870f80
#define IDA_64_8_UP_2 0xf5d165d4fb84aecc
#define CHEAT_ENGINE 0x49ff13d535334afd

// Struct to store window data
struct WindowData
{
    uint32_t ProcessId;
    uint64_t Hash;
    std::wstring Title;
};

// Function to calculate the hash of a buffer
uint64_t Hash(const void* buffer, size_t length)
{
    const auto data = static_cast<const uint8_t*>(buffer);
    uint64_t hash = 0;
    for (size_t i = 0; i < length; ++i)
    {
        hash = hash * 1099511628211ULL;
        hash = hash ^ static_cast<uint64_t>(data[i]);
    }
    return hash;
}

// Function to get the icon hash from a window handle
uint64_t GetIconHashFromWindow(HWND hwnd)
{
    // Get the small icon associated with the window
    auto hIcon = reinterpret_cast<HICON>(SendMessage(hwnd, WM_GETICON, ICON_SMALL, 0));
    if (hIcon == nullptr)
        hIcon = reinterpret_cast<HICON>(GetClassLongPtr(hwnd, GCLP_HICON));

    if (hIcon != nullptr)
    {
        ICONINFO iconInfo;
        if (GetIconInfo(hIcon, &iconInfo))
        {
            BITMAP bitmap;
            if (GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bitmap))
            {
                const int size = bitmap.bmWidth * bitmap.bmHeight * 4; // Assuming 32-bit color depth
                std::vector<BYTE> iconData(size);
                if (GetBitmapBits(iconInfo.hbmColor, size, iconData.data()))
                {
                    const auto hashed = Hash(iconData.data(), size);
                    DeleteObject(iconInfo.hbmColor);
                    DeleteObject(iconInfo.hbmMask);
                    return hashed;
                }
            }
            DeleteObject(iconInfo.hbmColor);
            DeleteObject(iconInfo.hbmMask);
        }
    }
    return 0;
}

// Callback function for EnumWindows to collect window data
BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam)
{
    const auto result = reinterpret_cast<std::vector<WindowData>*>(lParam);
    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    const auto hashed = GetIconHashFromWindow(hwnd);
    wchar_t titleBuffer[MAX_PATH]{ 0 };
    GetWindowTextW(hwnd, (LPWSTR)&titleBuffer, sizeof titleBuffer);
    result->push_back({ processId, hashed, titleBuffer });
    return TRUE;
}

// Function to find a process by its icon hash
bool FindProcessByIcon(uint64_t hashToFind, WindowData* outResult)
{
    std::vector<WindowData> windowsData{};
    EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&windowsData));
    for (const auto& window : windowsData)
    {
        if (window.Hash == hashToFind)
        {
            std::wcout << L"Process ID: " << window.ProcessId << L" Title: " << window.Title << std::endl;
            *outResult = window;
            return true;
        }
    }
    return false;
}

// Function to find all processes with a specific icon hash
bool FindProcessesByIcon(uint64_t hashToFind, std::vector<WindowData>* outResult)
{
    std::vector<WindowData> windowsData{};
    EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&windowsData));
    for (const auto& window : windowsData)
    {
        if (window.Hash == hashToFind)
        {
            std::wcout << L"Process ID: " << window.ProcessId << L" Title: " << window.Title << std::endl;
            outResult->push_back(window);
        }
    }
    return !outResult->empty();
}

// Function to get window data for windows with a specific title
std::vector<WindowData> GetIconHashFromWindow(const wchar_t* windowTitle)
{
    std::vector<WindowData> windowsData{};
    EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&windowsData));
    std::vector<WindowData> result;
    for (const auto& window : windowsData)
    {
        if (wcsstr(window.Title.c_str(), windowTitle))
        {
            result.push_back({ window });
        }
    }
    return result;
}

int main()
{
    // Get icon hash for all windows with the title "Cheat Engine"
    auto hashList = GetIconHashFromWindow(L"Cheat Engine");

    std::vector<WindowData> outResult;

    // Find processes with a specific icon hash
    FindProcessesByIcon(0xe41dc94415badbe9, &outResult);

    return 0;
}