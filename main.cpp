#include <windows.h>
#include <commdlg.h>
#include "resource.h"
#include <string>
#include <vector>

// Link with ComCtl32.lib
#pragma comment(lib, "ComCtl32.lib")

// Forward declarations of functions included in this code module
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK PhotoEditorWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK NewFileDlgProc(HWND, UINT, WPARAM, LPARAM);

// Constants
#define ID_BUTTON_NEW_PROJECT 1
#define ID_MENU_FILE_NEW 2

// Global Variables
HINSTANCE g_hInstance;
std::vector<HBITMAP> layers;
HWND hLayerListView;

// Function to add a layer to the list view
void AddLayer(HWND hWnd, HBITMAP hBitmap) {
    layers.push_back(hBitmap);

    LVITEM lvItem = { 0 };
    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = const_cast<LPWSTR>(L"Layer");
    lvItem.iItem = ListView_GetItemCount(hLayerListView);

    ListView_InsertItem(hLayerListView, &lvItem);
}

// The main function for a Windows application
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    g_hInstance = hInstance;

    // Register the main window class
    const wchar_t MAIN_CLASS_NAME[] = L"Main Window Class";

    WNDCLASS wc = { };
    wc.lpfnWndProc   = MainWndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = MAIN_CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

    RegisterClass(&wc);

    // Create the main window
    HWND hWnd = CreateWindowEx(
        0,
        MAIN_CLASS_NAME,
        L"Main Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hWnd == NULL) {
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);

    // Run the message loop
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// Process messages for the main window
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        {
            // Create a button
            HWND hButton = CreateWindow(
                L"BUTTON",  // Predefined class; Unicode assumed
                L"New Project",      // Button text
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles
                50,         // x position
                50,         // y position
                150,        // Button width
                30,         // Button height
                hWnd,       // Parent window
                (HMENU) ID_BUTTON_NEW_PROJECT,       // Control ID
                (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                NULL);      // Pointer not needed
        }
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BUTTON_NEW_PROJECT) {
            // Register the photo editor window class
            const wchar_t PHOTO_EDITOR_CLASS_NAME[] = L"Photo Editor Window Class";

            WNDCLASS wc = { };
            wc.lpfnWndProc   = PhotoEditorWndProc;
            wc.hInstance     = g_hInstance;
            wc.lpszClassName = PHOTO_EDITOR_CLASS_NAME;
            wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);

            RegisterClass(&wc);

            // Create the photo editor window
            HWND hPhotoEditorWnd = CreateWindowEx(
                0,
                PHOTO_EDITOR_CLASS_NAME,
                L"Photo Editor",
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
                NULL,
                NULL,
                g_hInstance,
                NULL
            );

            if (hPhotoEditorWnd != NULL) {
                ShowWindow(hPhotoEditorWnd, SW_SHOW);
            }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Process messages for the photo editor window
LRESULT CALLBACK PhotoEditorWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        {
            HMENU hMenubar = CreateMenu();
            HMENU hMenu = CreateMenu();

            AppendMenu(hMenu, MF_STRING, ID_MENU_FILE_NEW, L"New File");
            AppendMenu(hMenu, MF_STRING, IDM_FILE_IMPORT, L"Import");
            AppendMenu(hMenubar, MF_POPUP, (UINT_PTR)hMenu, L"File");

            SetMenu(hWnd, hMenubar);

            // Create a list view for layers
            hLayerListView = CreateWindow(WC_LISTVIEW, 
                NULL, 
                WS_CHILD | WS_VISIBLE | LVS_REPORT, 
                600, 0, 200, 600, 
                hWnd, 
                NULL, 
                g_hInstance, 
                NULL);

            LVCOLUMN lvColumn;
            lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;
            lvColumn.pszText = const_cast<LPWSTR>(L"Layers");
            lvColumn.cx = 200;
            ListView_InsertColumn(hLayerListView, 0, &lvColumn);
        }
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_MENU_FILE_NEW) {
            DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_NEWFILE), hWnd, NewFileDlgProc);
        }
        else if (LOWORD(wParam) == IDM_FILE_IMPORT) {
            OPENFILENAME ofn;
            wchar_t szFile[260];

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"All\0*.*\0Image Files\0*.BMP;*.JPG;*.GIF;*.PNG\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn) == TRUE) {
                HBITMAP hBitmap = (HBITMAP) LoadImage(NULL, ofn.lpstrFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
                if (hBitmap != NULL) {
                    AddLayer(hWnd, hBitmap);
                }
            }
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // Draw the layers in the correct order
            for (HBITMAP hBitmap : layers) {
                HDC hdcMem = CreateCompatibleDC(hdc);
                HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hBitmap);

                BITMAP bm;
                GetObject(hBitmap, sizeof(bm), &bm);

                BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
                SelectObject(hdcMem, hbmOld);
                DeleteDC(hdcMem);
            }

            EndPaint(hWnd, &ps);
        }
        break;

    case WM_DESTROY:
        // Cleanup
        for (HBITMAP hBitmap : layers) {
            DeleteObject(hBitmap);
        }
        DestroyWindow(hWnd);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Dialog procedure for the New File dialog
INT_PTR CALLBACK NewFileDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            wchar_t projectName[100];
            wchar_t canvasWidth[10];
            wchar_t canvasHeight[10];

            GetDlgItemText(hDlg, IDC_EDIT_PROJECT_NAME, projectName, 100);
            GetDlgItemText(hDlg, IDC_EDIT_CANVAS_WIDTH, canvasWidth, 10);
            GetDlgItemText(hDlg, IDC_EDIT_CANVAS_HEIGHT, canvasHeight, 10);

            // Convert width and height to integers
            int width = _wtoi(canvasWidth);
            int height = _wtoi(canvasHeight);

            // Here you can add code to handle the project name, width, and height

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        } else if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
