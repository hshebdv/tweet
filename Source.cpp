#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib,"wininet")

#include <windows.h>
#include <wininet.h>
#include "json.hpp"

using json = nlohmann::json;

TCHAR szClassName[] = TEXT("Window");

BOOL Tweet(LPCWSTR lpszBearerToken, LPCWSTR lpszMessage)
{
	BOOL bRet = FALSE;
	const HINTERNET hSession = InternetOpen(TEXT("Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/38.0.2125.111 Safari/537.36"), INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, INTERNET_FLAG_NO_COOKIES);
	if (hSession)
	{
		const HINTERNET hConnection = InternetConnect(hSession, L"api.twitter.com", INTERNET_DEFAULT_HTTPS_PORT, 0, 0, INTERNET_SERVICE_HTTP, 0, 0);
		if (hConnection)
		{
			const HINTERNET hRequest = HttpOpenRequest(hConnection, L"POST", L"/2/tweets", 0, 0, 0, INTERNET_FLAG_SECURE, 0);
			if (hRequest)
			{
				WCHAR szHeader[1024];
				lstrcpy(szHeader, L"Authorization: Bearer ");
				lstrcat(szHeader, lpszBearerToken);
				HttpAddRequestHeaders(hRequest, szHeader, lstrlen(szHeader), HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);

				lstrcpy(szHeader, L"Content-type: application/json");
				HttpAddRequestHeaders(hRequest, szHeader, lstrlen(szHeader), HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);

				json j;
				j["text"] = "Tweeting with media!";				

				HttpSendRequest(hRequest, 0, 0, (LPVOID)j.dump().c_str(), (DWORD)j.dump().size());
				LPBYTE lpszReturn = (LPBYTE)GlobalAlloc(GMEM_FIXED, 1);
				DWORD dwRead;
				static BYTE szBuf[1024 * 4];
				LPBYTE lpTmp;
				DWORD dwSize = 0;
				for (;;)
				{
					if (!InternetReadFile(hRequest, szBuf, (DWORD)sizeof(szBuf), &dwRead) || !dwRead) break;
					lpTmp = (LPBYTE)GlobalReAlloc(lpszReturn, (SIZE_T)(dwSize + dwRead + 1), GMEM_MOVEABLE);
					if (lpTmp == NULL) break;
					lpszReturn = lpTmp;
					CopyMemory(lpszReturn + dwSize, szBuf, dwRead);
					dwSize += dwRead;
				}
				lpszReturn[dwSize] = 0;

				json j2 = json::parse(lpszReturn);
				if (j2["status"] != 403)
				{
					bRet = TRUE;
				}

				InternetCloseHandle(hRequest);
			}
			InternetCloseHandle(hConnection);
		}
		InternetCloseHandle(hSession);
	}
	return bRet;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit1;
	static HWND hEdit5;
	static HWND hButton;
	switch (msg)
	{
	case WM_CREATE:
		hEdit1 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), 0, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hEdit5 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), 0, WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton = CreateWindow(TEXT("BUTTON"), TEXT("ツイート"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		SendMessage(hWnd, WM_DPICHANGED, 0, 0);
		break;
	case WM_SIZE:
		MoveWindow(hEdit1, 10, 10,256, 32, TRUE);
		MoveWindow(hEdit5, 10, 50,256, 256, TRUE);
		MoveWindow(hButton, 276, 50, 128, 32, TRUE);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			WCHAR szBearerToken[1024];
			GetWindowText(hEdit1, szBearerToken, _countof(szBearerToken));
			WCHAR szMessage[1024];
			GetWindowText(hEdit5, szMessage, _countof(szMessage));
			if (Tweet(szBearerToken, szMessage)) {
				MessageBox(hWnd, L"ツイートしました", L"ツイート", MB_OK);
			}
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("Window"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
