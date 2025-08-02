#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#define HOTKEY_ID 1
#define MODIFIER_KEY (MOD_CONTROL | MOD_ALT) // Ctrl+Alt组合
#define HOTKEY_KEY 'X'                       // 副键为X
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
        return false;

    char *lastSlash = strrchr(executablePath, '\\');
    if (lastSlash)
        *lastSlash = '\0';

    snprintf(path, size, "%s", executablePath);
    return true;
}

bool ExecutePssuspend(const char *toolFolder, const char *processName, bool isResume)
{
    char command[NAME_MAX * 2];
    snprintf(command, sizeof(command), "%s\\pssuspend.exe %s %s", toolFolder, isResume ? "-r" : "", processName);
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
                printf("[成功] 进程 %s 已恢复\n", processName);
                Beep(500, 600);
                isPaused = false;
            }
            else
            {
                printf("[错误] 恢复 %s 失败\n", processName);
                Beep(250, 600);
            }
        }
        else
        {
            if (ExecutePssuspend(toolFolder, processName, false))
            {
                printf("[成功] 进程 %s 已暂停\n", processName);
                Beep(500, 600);
                isPaused = true;
            }
            else
            {
                printf("[错误] 暂停 %s 失败\n", processName);
                Beep(250, 600);
            }
        }
    }
    else
    {
        printf("[错误] 未找到 pssuspend.exe\n");
        printf("  => 请确保 tools 文件夹中存在 pssuspend.exe\n");
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
        printf("[错误] 窗口类注册失败\n");
        return;
    }

    HWND hwnd = CreateWindowA("HotkeyDemo", "HotkeyDemo", 0, 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
    if (!hwnd)
    {
        printf("[错误] 窗口创建失败\n");
        return;
    }

    // 注册 Ctrl+Alt+X 热键
    if (RegisterHotKey(hwnd, HOTKEY_ID, MODIFIER_KEY, HOTKEY_KEY))
    {
        printf("[状态] 热键 Ctrl+Alt+X 已注册\n");
    }
    else
    {
        printf("[错误] 注册 Ctrl+Alt+X 失败\n");
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
                printf("[错误] 未找到工具文件夹\n");
                Beep(750, 600);
            }
            else
            {
                HandleProcessToggle(toolFolder, targetProcessName);
            }
        }
        else
        {
            printf("[错误] 获取可执行路径失败\n");
            Beep(250, 600);
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    char targetProcessNamelist[LIST_NUM][NAME_MAX] = 
    {
        "MonsterHunterWorld.exe",
        "BBQ-Win64-Shipping.exe", 
        ""
    };
    printf("进程挂起器 v1.0--20250802 --made by lil candy\n");
    printf("目标进程列表:\n");
    for (int i = 0; i < LIST_NUM - 1; i++)
        printf("  %d: %s\n", i + 1, targetProcessNamelist[i]);
    printf("  %d: 自定义进程\n\n", LIST_NUM);

    int choice = 0;
    printf("选择目标进程 [1-%d]: ", LIST_NUM);
    scanf("%d", &choice);
    getchar();

    if (choice >= 1 && choice <= LIST_NUM - 1)
    {
        strncpy(targetProcessName, targetProcessNamelist[choice - 1], NAME_MAX);
    }
    else if (choice == LIST_NUM)
    {
        printf("输入进程名称: ");
        fgets(targetProcessName, NAME_MAX, stdin);
        targetProcessName[strcspn(targetProcessName, "\n")] = '\0';
    }
    else
    {
        printf("[警告] 选择无效，使用默认进程\n");
        strncpy(targetProcessName, targetProcessNamelist[0], NAME_MAX);
    }

    printf("\n监控中: %s\n", targetProcessName);
    printf("按 Ctrl+Alt+X 切换暂停/恢复状态\n\n"); // 更新热键提示

    RegisterHotKeyAndWindow();
    getchar();
    return 0;
}