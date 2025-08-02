#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#define HOTKEY_ID 1
#define MODIFIER_KEY MOD_CONTROL
#define HOTKEY_KEY VK_F2
#define LIST_NUM 3
#define NAME_MAX 512
const char *toolFolderName = "tools";
bool isPaused = false;

// 函数声明
bool GetExecutableDirectory(char *path, size_t size);
bool ExecutePssuspend(const char *toolFolder, const char *processName, bool isResume);
void HandleProcessToggle(const char *toolFolder, const char *processName);
void RegisterHotKeyAndWindow();
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

char targetProcessName[NAME_MAX] = {0};

bool GetExecutableDirectory(char *path, size_t size)
{
    char executablePath[NAME_MAX];
    if (GetModuleFileName(NULL, executablePath, NAME_MAX) == 0)
    {
        return false;
    }

    char *lastSlash = strrchr(executablePath, '\\');
    if (lastSlash != NULL)
    {
        *lastSlash = '\0';
    }

    snprintf(path, size, "%s", executablePath);
    return true;
}

bool ExecutePssuspend(const char *toolFolder, const char *processName, bool isResume)
{
    char command[NAME_MAX * 2];
    if (isResume)
    {
        snprintf(command, sizeof(command), "%s\\pssuspend.exe -r %s", toolFolder, processName);
    }
    else
    {
        snprintf(command, sizeof(command), "%s\\pssuspend.exe %s", toolFolder, processName);
    }
    return system(command) == 0;
}

void HandleProcessToggle(const char *toolFolder, const char *processName)
{
    char pssuspendPath[NAME_MAX];
    snprintf(pssuspendPath, sizeof(pssuspendPath), "%s\\pssuspend.exe", toolFolder);

    if (GetFileAttributesA(pssuspendPath) != INVALID_FILE_ATTRIBUTES)
    {
        if (isPaused)
        {
            if (ExecutePssuspend(toolFolder, processName, true))
            {
                printf("[SUCCESS] Process %s resumed\n", processName);
                Beep(500, 600);
                isPaused = false;
            }
            else
            {
                printf("[ERROR] Failed to resume %s\n", processName);
                Beep(250, 600);
            }
        }
        else
        {
            if (ExecutePssuspend(toolFolder, processName, false))
            {
                printf("[SUCCESS] Process %s paused\n", processName);
                Beep(500, 600);
                isPaused = true;
            }
            else
            {
                printf("[ERROR] Failed to pause %s\n", processName);
                Beep(250, 600);
            }
        }
    }
    else
    {
        printf("[ERROR] pssuspend.exe not found\n");
        printf("  => Ensure pssuspend.exe exists in the tools folder\n");
        Beep(250, 600);
    }
}

void RegisterHotKeyAndWindow()
{
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "HotkeyDemo";

    if (!RegisterClassA(&wc))
    {
        printf("[ERROR] Window class registration failed\n");
        return;
    }

    HWND hwnd = CreateWindowA("HotkeyDemo", "HotkeyDemo", 0, 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
    if (!hwnd)
    {
        printf("[ERROR] Window creation failed\n");
        return;
    }

    if (RegisterHotKey(hwnd, HOTKEY_ID, MODIFIER_KEY, HOTKEY_KEY))
    {
        printf("[STATUS] Hotkey Ctrl+F2 registered\n");
    }
    else
    {
        printf("[ERROR] Failed to register Ctrl+F2\n");
        return;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnregisterHotKey(hwnd, HOTKEY_ID);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_HOTKEY && wParam == HOTKEY_ID)
    {
        char toolFolder[NAME_MAX];
        if (GetExecutableDirectory(toolFolder, NAME_MAX))
        {
            snprintf(toolFolder, NAME_MAX, "%s\\%s", toolFolder, toolFolderName);
            if (GetFileAttributesA(toolFolder) == INVALID_FILE_ATTRIBUTES)
            {
                printf("[ERROR] Tools folder not found\n");
                Beep(750, 600);
            }
            else
            {
                HandleProcessToggle(toolFolder, targetProcessName);
            }
        }
        else
        {
            printf("[ERROR] Failed to get executable path\n");
            Beep(250, 600);
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main()
{
    SetConsoleOutputCP(CP_UTF8); // 统一使用UTF-8输出

    char targetProcessNamelist[LIST_NUM][NAME_MAX] = {"MonsterHunterWorld.exe", "BBQ-Win64-Shipping.exe", ""};
    printf("懒得弄中文兼容\n");
    printf("Process Suspender v1.0 (by lil candy)\n");
    printf("Compatible with WIN10/WIN11\n\n");
    printf("Target Process List:\n");
    for (int i = 0; i < LIST_NUM - 1; i++)
        printf("  %d: %s\n", i + 1, targetProcessNamelist[i]);
    printf("  %d: Custom process\n\n", LIST_NUM);

    int choice = 0;
    printf(" Select %d process can select your own exe\r\n", LIST_NUM);
    printf("Select target process [1-%d]: ", LIST_NUM);
    scanf("%d", &choice);
    getchar();

    if (choice >= 1 && choice <= LIST_NUM - 1)
    {
        strncpy(targetProcessName, targetProcessNamelist[choice - 1], NAME_MAX);
    }
    else if (choice == LIST_NUM)
    {
        printf("Enter process name: ");
        fgets(targetProcessName, NAME_MAX, stdin);
        targetProcessName[strcspn(targetProcessName, "\n")] = '\0';
    }
    else
    {
        printf("[WARN] Invalid selection, using default process\n");
        strncpy(targetProcessName, targetProcessNamelist[0], NAME_MAX);
    }

    printf("\nMonitoring: %s\n", targetProcessName);
    printf("Press Ctrl+F2 to toggle pause/resume state\n\n");

    RegisterHotKeyAndWindow();
    getchar(); // 等待回车
    return 0;
}