#include <Windows.h>
#include <WinUser.h>
#include <stdio.h>
#include "OnkoSurvey.h"
#include "resource.h"


/*************************
	GLOBAL VARIABLES
***************************/


struct tagButton buttons[NUM_OF_BUTTONS];
struct tagCombobox combos[NUM_OF_COMBOS];


static int i = 0; // pre scrollbar




/**********************************
	FUNCTION DECLARATIONS
**********************************/
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); //standardna procedura okna

static WNDPROC OriginalEditCtrlProc = NULL; // sem si ulozim originalnu alebo custom proceduru

LRESULT CALLBACK EditWndProc(HWND, UINT, WPARAM, LPARAM); // subclassing - custom procedura len pre Edit boxy...

LRESULT CALLBACK ResultDlgProc(HWND, UINT, WPARAM, LPARAM); // procedura dialogoveho okna ResultDialog



void CreateControls(const HWND m_hwnd, HWND & m_hwndEditText1, HWND & m_hwndEditText2, HWND & m_hwndButtonEvaluate,
					HDC & m_hdcEditText1, HDC & m_hdcEditText2, HDC & m_hdcButtonEvalute);

void ReleaseControls(const HWND m_hwndEditText1, const HWND m_hwndEditText2, const HWND m_hwndButtonEvaluate,
					 const HDC m_hdcEditText1, const HDC m_hdcEditText2, const HDC m_hdcButtonEvaluate);

BOOL VerifyComboStatus(const HWND m_hwnd);

int CalculateResult(const HWND m_hwnd);

void ShowResult(const HINSTANCE m_hInstance, const HWND m_hwnd, const int m_nResult, const HWND m_hwndButtonEvaluate);

int PrintResult(const int nResult);


/* EVENT HANDLING PROCEDURES */
int OnCreate(const HWND m_hwnd, TEXTMETRIC &m_tm, int &m_cxChar, int &m_cxCaps, int &m_cyChar,
			 int &m_iMaxWidth, int &m_iDeltaPerLine, ULONG &m_ulScrollLines,
			 HWND & m_hwndEditText1, HWND & m_hwndEditText2, HWND & m_hwndButtonEvaluate,
			 HDC & m_hdcEditText1, HDC & m_hdcEditText2, HDC & m_hdcButtonEvalute);

int OnCtlColorStatic(const WPARAM m_wParam);

int OnCtlColorEdit(const WPARAM m_wParam);

int OnSettingChange(ULONG &m_ulScrollLines, int & m_iDeltaPerLine);

int OnSize(const HWND m_hwnd, const LPARAM m_lParam, SCROLLINFO & m_si, int & m_cxClient, int & m_cyClient, const int m_cxChar, const int m_cyChar, const int m_iMaxWidth);

int OnVScroll(const WPARAM m_wParam, const HWND m_hwnd, SCROLLINFO &m_si, int & m_iVertPos, const int m_cyChar);

int OnKeyDown(const HWND m_hwnd, const WPARAM m_wParam);

int OnMouseWheel(const HWND m_hwnd, const WPARAM m_wParam, const int m_iDeltaPerLine, int & m_iAccumDelta);

int OnPaint(const HWND m_hwnd, PAINTSTRUCT & m_ps, SCROLLINFO & m_si, int & m_iVertPos, int & m_iHorzPos);

int OnCommand(const HINSTANCE m_hInstance, const HWND m_hwnd, const WPARAM m_wParam, const LPARAM m_lParam, const HWND m_hwndButtonEvaluate);

int OnDestroy(const HWND m_hwndEditText1, const HWND m_hwndEditText2, const HWND m_hwndButtonEvaluate,
			  const HDC m_hdcEditText1, const HDC m_hdcEditText2, const HDC m_hdcButtonEvaluate);




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{	
	static TCHAR szAppName[] = TEXT("Onkologick˝ geriatrick˝ dotaznÌk");
	MSG msg;
	HWND hwnd;
	WNDCLASS wndclass;

	wndclass.style			= CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc	= WndProc;
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	wndclass.hInstance		= hInstance;
	wndclass.hIcon			= LoadIcon(NULL, szAppName);
	wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground	= (HBRUSH)(COLOR_BACKGROUND + 2);
	wndclass.lpszMenuName	= szAppName;
	wndclass.lpszClassName	= szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBoxW(NULL, TEXT("Chyba pri otv·ranÌ okna!"), szAppName, MB_ICONERROR);
		return 0;
	}


	hwnd = CreateWindow(szAppName, TEXT("Onkologick˝ geriatrick˝ dotaznÌk"),
		WS_OVERLAPPEDWINDOW | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, SW_MAXIMIZE);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!IsDialogMessage(hwnd, &msg)) // IsDialogMessage potrebujem kvoli WS_TABSTOP!
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int  cxChar, cxCaps, cyChar, cxClient, cyClient, iMaxWidth;
	static int  iDeltaPerLine, iAccumDelta;     // for mouse wheel logic
	//HDC         hdc;
	int         iVertPos, iHorzPos;
	static SCROLLINFO  si;
	TEXTMETRIC  tm;
	ULONG       ulScrollLines;                  // for mouse wheel logic
	PAINTSTRUCT	ps;

	HWND hwndEditText1 = NULL;
	HWND hwndEditText2 = NULL;
	HWND hwndButtonEvaluate = NULL;

	HDC hdcEditText1 = NULL;
	HDC hdcEditText2 = NULL;
	HDC hdcButtonEvalute = NULL;

	switch (message)
	{
		case WM_CREATE:			return OnCreate(hwnd, tm, cxChar, cxCaps, cyChar, iMaxWidth, iDeltaPerLine, ulScrollLines, hwndEditText1, hwndEditText2, hwndButtonEvaluate, hdcEditText1, hdcEditText2, hdcButtonEvalute);
		case WM_CTLCOLORSTATIC: return OnCtlColorStatic(wParam); // farba fontu STATIC poli - cize labelov
		case WM_CTLCOLOREDIT:	return OnCtlColorEdit(wParam); //farba fontu EDIT poli - cize textboxov a comboboxov
		case WM_SETTINGCHANGE:	return OnSettingChange(ulScrollLines, iDeltaPerLine);
		case WM_SIZE:			return OnSize(hwnd, lParam, si, cxClient, cyClient, cxChar, cyChar, iMaxWidth);
		case WM_VSCROLL:		return OnVScroll(wParam, hwnd, si, iVertPos, cyChar);
		case WM_KEYDOWN:		return OnKeyDown(hwnd, wParam);
		case WM_MOUSEWHEEL:		
			if (OnMouseWheel(hwnd, wParam, iDeltaPerLine, iAccumDelta) == -1)
				break;
			return 0;
		case WM_PAINT:			return OnPaint(hwnd, ps, si, iVertPos, iHorzPos);
		case WM_COMMAND:		return OnCommand((HINSTANCE)GetWindowLongW(hwnd, GWL_HINSTANCE), hwnd, wParam, lParam, hwndButtonEvaluate);
		case WM_DESTROY:		return OnDestroy(hwndEditText1, hwndEditText2, hwndButtonEvaluate, hdcEditText1, hdcEditText2, hdcButtonEvalute);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}


void CreateControls(const HWND m_hwnd, HWND & m_hwndEditText1, HWND & m_hwndEditText2, HWND & m_hwndButtonEvaluate,
					HDC & m_hdcEditText1, HDC & m_hdcEditText2, HDC & m_hdcButtonEvalute)
{
	// label for Question #1
	CreateWindow(TEXT("Static"), TEXT("1. Uh·dol, ak˝ je dnes d·tum?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 10, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #1
	buttons[0].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 10, 60, 25,
		m_hwnd, (HMENU)IDC_BUTTON1,
		NULL, NULL); 

	CreateWindow(TEXT("Static"), TEXT("2. Uh·dol, ak˝ je dnes deÚ v t˝ûdni?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 40, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #2
	buttons[1].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 40, 60, 25,
		m_hwnd, (HMENU)IDC_BUTTON2,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("3. Uh·dol, ak˝ je teraz rok?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 70, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #3
	buttons[2].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 70, 60, 25,
		m_hwnd, (HMENU)IDC_BUTTON3,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("4. Uh·dol, akÈ je teraz roËnÈ obdobie?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 100, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #4
	buttons[3].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 100, 60, 25,
		m_hwnd, (HMENU)IDC_BUTTON4,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("5. Uh·dol, na ktorom sme poschodÌ?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 130, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #5
	buttons[4].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 130, 60, 25,
		m_hwnd, (HMENU)IDC_BUTTON5,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("6. Poviem V·m tri predmety. Budem chcieù, aby ste ich zopakovali. O chvÌæu sa V·s na tieto slov· op‰ù op˝tam: \"LOPTA, ä¡L, V¡ZA\"."),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 160, 900, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	CreateWindow(TEXT("Static"), TEXT("  a) Uh·dol slovo \"LOPTA\"?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 190, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #6a - button6
	buttons[5].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 190, 60, 25,
		m_hwnd, (HMENU)IDC_BUTTON6,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("  b) Uh·dol slovo \"ä¡L\"?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 220, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #6b - button7
	buttons[6].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 220, 60, 25,
		m_hwnd, (HMENU)IDC_BUTTON7,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("  c) Uh·dol slovo \"V¡ZA\"?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 250, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #6c - button8
	buttons[7].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 250, 60, 25,
		m_hwnd, (HMENU)IDC_BUTTON8,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("7. Vezmite do ruky papier, ktor˝ m·te pred sebou, preloûte ho na polovicu a poloûte na podlahu"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 280, 900, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	CreateWindow(TEXT("Static"), TEXT("  a) Uchopil papier?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 310, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #7a - button9
	buttons[8].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 310, 60, 25,
		m_hwnd, (HMENU)IDC_BUTTON9,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("  b) Preloûil papier na polovicu?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 340, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #7b - button10
	buttons[9].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 340, 60, 25,
		m_hwnd, (HMENU)IDC_BUTTON10,
		NULL, NULL);

	CreateWindow(TEXT("Static"), TEXT("  c) Poloûil papier na podlahu?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 370, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #7c - button11
	buttons[10].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 370, 60, 25,
		m_hwnd, (HMENU)IDC_BUTTON11,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("8. Teraz mi povedzte 3 slov·, ktorÈ ste mali zopakovaù."),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 400, 900, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	CreateWindow(TEXT("Static"), TEXT("  a) Povedal slovo \"LOPTA?\""),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 430, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #8a - button12
	buttons[11].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 430, 60, 25,
		m_hwnd, (HMENU)IDC_BUTTON12,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("  b) Povedal slovo \"ä¡L?\""),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 460, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #8b - button13
	buttons[12].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 460, 60, 25,
		m_hwnd, (HMENU)IDC_BUTTON13,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("  c) Povedal slovo \"V¡ZA?\""),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 490, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #8c - button14
	buttons[13].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 490, 60, 25,
		m_hwnd, (HMENU)IDC_BUTTON14,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("9. Ako sa stravujete?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 520, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// combobox #1 pre otazku 9
	combos[0].hwndComboBox = CreateWindow(TEXT("combobox"), NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
		330, 520, 330, 250,
		m_hwnd, (HMENU)IDC_COMBOBOX1,
		NULL, NULL);
	SendMessage(combos[0].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("<Vyber odpoveÔ>"));
	SendMessage(combos[0].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Neschopn˝ jesù s·m bez pomoci"));
	SendMessage(combos[0].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("S·m s ùaûkosùami"));
	SendMessage(combos[0].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("S·m bez ùaûkostÌ"));
	SendMessage(combos[0].hwndComboBox, CB_SETCURSEL, 0, 0);

	CreateWindow(TEXT("Static"), TEXT("10. Koæko tekutÌn prijmete za deÚ?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 550, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// combobox #2 pre otazku 10
	combos[1].hwndComboBox = CreateWindow(TEXT("combobox"), NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
		330, 550, 330, 250,
		m_hwnd, (HMENU)IDC_COMBOBOX2,
		NULL, NULL);
	SendMessage(combos[1].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("<Vyber odpoveÔ>"));
	SendMessage(combos[1].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Menej ako 900ml"));
	SendMessage(combos[1].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Medzi 900ml a 1500ml"));
	SendMessage(combos[1].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Viac ako 1500ml"));
	SendMessage(combos[1].hwndComboBox, CB_SETCURSEL, 0, 0);

	CreateWindow(TEXT("Static"), TEXT("11. UdrûÌte moË?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 580, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// combobox #3 pre otazku 11
	combos[2].hwndComboBox = CreateWindow(TEXT("combobox"), NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
		330, 580, 330, 250,
		m_hwnd, (HMENU)IDC_COMBOBOX3,
		NULL, NULL);
	SendMessage(combos[2].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("<Vyber odpoveÔ>"));
	SendMessage(combos[2].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("PomoËÌ sa viac ako 1x denne alebo m· PK"));
	SendMessage(combos[2].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("ObËas nie (max 1x denne neudrûÌ)"));
	SendMessage(combos[2].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("¡no"));
	SendMessage(combos[2].hwndComboBox, CB_SETCURSEL, 0, 0);


	CreateWindow(TEXT("Static"), TEXT("12. Pohyblivosù pacienta?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 610, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// combobox #4 pre otazku 12
	combos[3].hwndComboBox = CreateWindow(TEXT("combobox"), NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
		330, 610, 330, 250,
		m_hwnd, (HMENU)IDC_COMBOBOX4,
		NULL, NULL);
	SendMessage(combos[3].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("<Vyber odpoveÔ>"));
	SendMessage(combos[3].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Prip˙tan˝ na lÙûko"));
	SendMessage(combos[3].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("ChodÌ po byte"));
	SendMessage(combos[3].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Vych·dza z bytu"));
	SendMessage(combos[3].hwndComboBox, CB_SETCURSEL, 0, 0);


	CreateWindow(TEXT("Static"), TEXT("13. Osobn· hygiena pacienta?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 640, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// combobox #5 pre otazku 13
	combos[4].hwndComboBox = CreateWindow(TEXT("combobox"), NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
		330, 640, 330, 250,
		m_hwnd, (HMENU)IDC_COMBOBOX5,
		NULL, NULL);
	SendMessage(combos[4].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("<Vyber odpoveÔ>"));
	SendMessage(combos[4].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Potrebuje pomoc"));
	SendMessage(combos[4].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("SebestaËn˝ pri um˝vanÌ, holenÌ, ËesanÌ..."));
	SendMessage(combos[4].hwndComboBox, CB_SETCURSEL, 0, 0);

	CreateWindow(TEXT("Static"), TEXT("14. Obliekanie?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 670, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// combobox #6 pre otazku 14
	combos[5].hwndComboBox = CreateWindow(TEXT("combobox"), NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
		330, 670, 330, 250,
		m_hwnd, (HMENU)IDC_COMBOBOX6,
		NULL, NULL);
	SendMessage(combos[5].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("<Vyber odpoveÔ>"));
	SendMessage(combos[5].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Nedok·ûe sa obliecù"));
	SendMessage(combos[5].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Oblieka sa s pomocou (asi polovicu zvl·dne s·m)"));
	SendMessage(combos[5].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Oblieka sa s·m"));
	SendMessage(combos[5].hwndComboBox, CB_SETCURSEL, 0, 0);

	CreateWindow(L"Static", TEXT("15. UûÌvate viac ako 3 lieky?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 700, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// button for Question 14 - button15
	buttons[14].hwndButton = CreateWindow(L"Button", TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 700, 60, 25,
		m_hwnd, (HMENU)IDC_BUTTON15,
		NULL, NULL);
	
	CreateWindowEx(WS_EX_TRANSPARENT, L"Static", TEXT("16. ⁄bytok hmotnosti za posledn˝ mesiac?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 730, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	// combobox #7 pre otazku 16
	combos[6].hwndComboBox = CreateWindow(L"combobox", NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
		330, 730, 330, 250,
		m_hwnd, (HMENU)IDC_COMBOBOX7, NULL, NULL);
	SendMessage(combos[6].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)L"<Vyber odpoveÔ>");
	SendMessage(combos[6].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)L"Viac ako 3 kg");
	SendMessage(combos[6].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)L"Nevie udaù");
	SendMessage(combos[6].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)L"1 - 2 kg");
	SendMessage(combos[6].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)L"Bez ˙bytku");
	SendMessage(combos[6].hwndComboBox, CB_SETCURSEL, 0, 0);

	CreateWindow(L"Static", TEXT("V˝öka (cm)"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 760, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	m_hwndEditText1 = CreateWindow(L"Edit", TEXT("150"),
		WS_CHILD | WS_VISIBLE | ES_CENTER | WS_BORDER | ES_NUMBER | WS_TABSTOP,
		330, 760, 60, 25,
		m_hwnd, (HMENU)IDC_EDITTEXT1,
		NULL, NULL);
	SendMessage(m_hwndEditText1, EM_SETLIMITTEXT, 3, NULL); //nastav max pocet znakov v EDIT na 3
	if (m_hwndEditText1 != NULL)
	{
		WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(m_hwndEditText1, GWLP_WNDPROC, (LONG_PTR)EditWndProc);
		if (OriginalEditCtrlProc == NULL)
		{
			OriginalEditCtrlProc = oldProc;
		}
	}

	CreateWindow(L"Static", TEXT("cm"),
		WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER,
		400, 760, 30, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	CreateWindow(L"Static", TEXT("V·ha (kg)"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 790, 300, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	m_hwndEditText2 = CreateWindow(L"Edit", TEXT("75"),
		WS_CHILD | WS_VISIBLE | ES_CENTER | WS_BORDER | ES_NUMBER | WS_TABSTOP,
		330, 790, 60, 25,
		m_hwnd, (HMENU)IDC_EDITTEXT2,
		NULL, NULL);
	SendMessage(m_hwndEditText2, EM_SETLIMITTEXT, 3, NULL); //nastav max pocet znakov v EDIT na 3
	if (m_hwndEditText2 != NULL)
	{
		WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(m_hwndEditText2, GWLP_WNDPROC, (LONG_PTR)EditWndProc);
		if (OriginalEditCtrlProc == NULL)
		{
			OriginalEditCtrlProc = oldProc;
		}
	}

	CreateWindow(L"Static", TEXT("kg"),
		WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER,
		400, 790, 30, 25,
		m_hwnd, (HMENU)1,
		NULL, NULL);

	m_hwndButtonEvaluate = CreateWindow(L"Button", TEXT("VYHODNOTIç"),
		WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		10, 820, 900, 100,
		m_hwnd, (HMENU)IDC_BUTTON_EVALUATE,
		NULL, NULL);
	m_hdcButtonEvalute = GetDC(m_hwndButtonEvaluate);

	for (int i = 0; i < NUM_OF_BUTTONS; i++)
	{
		buttons[i].bButton = FALSE;
		buttons[i].hdcButton = GetDC(buttons[i].hwndButton);
	}
	SetFocus(buttons[0].hwndButton);
	for (int i = 0; i < NUM_OF_COMBOS; i++)
	{
		combos[i].bInitialRowComboBox = TRUE;
		combos[i].hdcComboBox = GetDC(combos[i].hwndComboBox);
	}
}

void ReleaseControls(const HWND m_hwndEditText1, const HWND m_hwndEditText2, const HWND m_hwndButtonEvaluate,
					 const HDC m_hdcEditText1, const HDC m_hdcEditText2, const HDC m_hdcButtonEvaluate)
{
	for (int i = 0; i < NUM_OF_BUTTONS; i++)
	{
		ReleaseDC(buttons[i].hwndButton, buttons[i].hdcButton);
	}
	for (int i = 0; i < NUM_OF_COMBOS; i++)
	{
		ReleaseDC(combos[i].hwndComboBox, combos[i].hdcComboBox);
	}
	ReleaseDC(m_hwndEditText1, m_hdcEditText1);
	ReleaseDC(m_hwndEditText2, m_hdcEditText2);
	ReleaseDC(m_hwndButtonEvaluate, m_hdcButtonEvaluate);
}

LRESULT CALLBACK EditWndProc(const HWND m_hwnd, const UINT m_uMsg, const WPARAM m_wParam, const LPARAM m_lParam)
{
	// sice som umoznil zadavat do EDIT len ciselne hodnoty (ES_NUMBER), 
	// uzivatel vsak stale moze urobit ctrl+v, preto to odchytavam a zahadzujem
	// zachytava aj "paste" cez kontextove menu ...
	if (m_uMsg == WM_PASTE)
		return 0;

	return CallWindowProc(OriginalEditCtrlProc, m_hwnd, m_uMsg, m_wParam, m_lParam);
}

BOOL VerifyComboStatus(const HWND m_hwnd)
{
	wchar_t *strText = new wchar_t[256]; // potrebujem wchar_t* ako ekvivalent LPWSTR
	for (int i = 0; i < NUM_OF_COMBOS; i++)
	{
		if (combos[i].bInitialRowComboBox == TRUE)
		{
			GetWindowText(combos[i].hwndComboBox, strText, 256);
			if (wcscmp(strText, L"<Vyber odpoveÔ>") == 0)
			{
				MessageBox(m_hwnd, TEXT("Vyberte poloûku!"), TEXT("Chyba!"), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
				SetFocus(combos[i].hwndComboBox);
				return FALSE;
			}
		}
	}
	delete[] strText;
	return TRUE;
}

int CalculateResult(const HWND m_hwnd)
{
	
	int nResult = 0;

	//spocitaj buttony okrem posledneho, lebo ten ma inu logiku
	for (int i = 0; i < NUM_OF_BUTTONS - 1; i++)
	{
		nResult += buttons[i].bButton ? 1 : 0;
	}
	nResult += buttons[NUM_OF_BUTTONS - 1].bButton ? 0 : 1;

	// vypocitaj BMI z 2 editov
	double dBMI = 0.0;
	int nVyska = 0;
	int nVaha = 0;

	nVyska = GetDlgItemInt(m_hwnd,IDC_EDITTEXT1, NULL, FALSE);
	nVaha = GetDlgItemInt(m_hwnd, IDC_EDITTEXT2, NULL, FALSE);


	dBMI = (double)nVaha / (((double)nVyska / 100.0) * ((double)nVyska / 100.0));
	if (dBMI > (double)22.9)
		nResult += 3;
	else if (dBMI > (double)20.9)
		nResult += 2;
	else if (dBMI >= (double)18.9)
		nResult += 1;
	// else - znamena ze je mensie ako 18.9 a dostava 0 bodov takze "do nothing"


	// vypocitaj z comboboxov tak, ze poziadas combobox o index aktualne vybranej polozky
	// a kedze polozky su usporiadane v comboboxe od polozky s hodnotou 0 az po polozku s najvyssou
	// hodnotou, body koresponduju s indexom polozky v comboboxe, preto CB_GETCURSEL
	for (int i = 0; i < NUM_OF_COMBOS; i++)
	{
		nResult += SendMessage(combos[i].hwndComboBox, CB_GETCURSEL, 0, 0);
	}
	return nResult;
}

void ShowResult(const HINSTANCE m_hInstance, const HWND m_hwnd, const int m_nResult, const HWND m_hwndButtonEvaluate)
{
	LPWSTR wcAnswer = new wchar_t[1000];
	LPWSTR wcGroup = new wchar_t[20];

	wsprintfW(wcAnswer, TEXT("Pacient dosiahol %d bodov.\nPacient patrÌ do %lS SKUPINY.\n\n"
		"Prim·rna skupina: od %d bodov.\nIntermedi·rna skupina: od %d do %d bodov.\nSekund·rna skupina: od %d do %d bodov.\n"
		"Terci·rna skupina: od %d do %d bodov."),
		m_nResult,
		(m_nResult >= ONKO_LOWER_LIMIT_PRIMARY) ? TEXT("PRIM¡RNEJ") : \
				(m_nResult >= ONKO_LOWER_LIMIT_INTERMEDIARY && m_nResult <= ONKO_UPPER_LIMIT_INTERMEDIARY) ? TEXT("INTERMEDI¡RNEJ") : \
				(m_nResult >= ONKO_LOWER_LIMIT_SECONDARY && m_nResult <= ONKO_UPPER_LIMIT_SECONDARY) ? TEXT("SEKUND¡RNEJ") : TEXT("TERCI¡RNEJ"),
		ONKO_LOWER_LIMIT_PRIMARY,
		ONKO_LOWER_LIMIT_INTERMEDIARY,
		ONKO_UPPER_LIMIT_INTERMEDIARY,
		ONKO_LOWER_LIMIT_SECONDARY,
		ONKO_UPPER_LIMIT_SECONDARY,
		ONKO_LOWER_LIMIT_TERTIARY,
		ONKO_UPPER_LIMIT_TERTIARY
	);
	
	//MessageBoxW(m_hwnd, wcAnswer, L"VYHODNOTENIE", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
	//DialogBoxW(m_hInstance, MAKEINTRESOURCE(IDD_DIALOG_RESULT), m_hwnd, reinterpret_cast<DLGPROC>(ResultDlgProc));
	HWND hwndResultDialog;
	hwndResultDialog = CreateDialogW(m_hInstance, MAKEINTRESOURCE(IDD_DIALOG_RESULT), m_hwnd, 
									 reinterpret_cast<DLGPROC>(ResultDlgProc)
									);
	ShowWindow(hwndResultDialog, SW_SHOW);
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;
	hdc = BeginPaint(hwndResultDialog, &ps);

	GetClientRect(hwndResultDialog, &rect);
	SetBkMode(hdc, TRANSPARENT);
	rect.left += 15;
	rect.top += 15;
	DrawText(hdc, wcAnswer, -1, &rect, DT_VCENTER);

	EndPaint(hwndResultDialog, &ps);
	
	
	SetFocus(m_hwndButtonEvaluate);

	delete[] wcAnswer;
	delete[] wcGroup;
}

int PrintResult(const int nResult)
{
	PRINTDLG prDlg;
	DOCINFO document_Information;
	HDC hPrinter; 
	TEXTMETRIC tm;
	int iCharsPerLine;

	memset(&prDlg, 0, sizeof(prDlg));
	prDlg.Flags = PD_RETURNDC; // get rid of PD_RETURNDEFAULT on the line below if you'd like to see the "Printer Settings" dialog!
	prDlg.lStructSize = sizeof(prDlg);
	// try to retrieve the printer DC
	if (!PrintDlg(&prDlg))
	{
		MessageBoxW(NULL, TEXT("TlaË zlyhala!"), TEXT("Chyba"), MB_OK | MB_ICONERROR);
		return -1;
	}
	
	hPrinter = prDlg.hDC;
	
	
	// initialization of the printing!
	memset(&document_Information, 0, sizeof(document_Information));
	document_Information.cbSize = sizeof(document_Information);
	document_Information.lpszDocName = TEXT("Geriatrick˝ dotaznÌk");
	StartDoc(hPrinter, &document_Information);
	// start the first and only page
	StartPage(hPrinter);

	// uncomment the following line to print in colour! :)
	// SetTextColor( hPrinter, 0x0000FF );
	
	LPWSTR wcCurrentDateTime = new wchar_t[25];
	SYSTEMTIME st;
	GetLocalTime(&st);
	wsprintfW(wcCurrentDateTime, TEXT("%d.%d.%d   %d:%d:%d"), st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond);
	TextOut(hPrinter, 100, 200, wcCurrentDateTime, wcslen(wcCurrentDateTime));
	delete[] wcCurrentDateTime;

	LPCWSTR wcTitle = TEXT("V › S L E D O K   G E R I A T R I C K … H O   D O T A Z N Õ K A:");
	TextOut(hPrinter, 1100, 600, wcTitle, wcslen(wcTitle));


	GetTextMetrics(hPrinter, &tm);
	iCharsPerLine = GetDeviceCaps(hPrinter, HORZRES) / tm.tmAveCharWidth;
	wchar_t* wcUnderLine = new wchar_t[iCharsPerLine + 1];
	wchar_t *wcSrc = TEXT("=");
	for (int i = 0; i < iCharsPerLine; i++)
	{
		wcUnderLine[i] = wcSrc[0];
	}
	TextOut(hPrinter, 0, 700, wcUnderLine, wcslen(wcUnderLine));
	delete[] wcUnderLine;


	LPWSTR wcAnswer1 = new wchar_t[50];
	wsprintfW(wcAnswer1, TEXT("Pacient dosiahol %d bodov."), nResult);
	TextOut(hPrinter, 100, 1000, wcAnswer1, wcslen(wcAnswer1));
	delete[] wcAnswer1;

	LPWSTR wcGroup = new wchar_t[50];
	wsprintfW(wcGroup, TEXT("Pacient patrÌ do %lS SKUPINY."),(nResult >= ONKO_LOWER_LIMIT_PRIMARY) ? TEXT("PRIM¡RNEJ") : \
		(nResult >= ONKO_LOWER_LIMIT_INTERMEDIARY && nResult <= ONKO_UPPER_LIMIT_INTERMEDIARY) ? TEXT("INTERMEDI¡RNEJ") : \
		(nResult >= ONKO_LOWER_LIMIT_SECONDARY && nResult <= ONKO_UPPER_LIMIT_SECONDARY) ? TEXT("SEKUND¡RNEJ") : TEXT("TERCI¡RNEJ"));
	TextOut(hPrinter, 100, 1100, wcGroup, wcslen(wcGroup));
	delete[] wcGroup;

	LPWSTR wcBuffer = new wchar_t[50];
	wsprintfW(wcBuffer, TEXT("Prim·rna skupina:      od %d bodov."), ONKO_LOWER_LIMIT_PRIMARY);
	TextOut(hPrinter, 100, 1500, wcBuffer, wcslen(wcBuffer));
	memset(wcBuffer, 0, sizeof(wcBuffer));
	
	wsprintfW(wcBuffer, TEXT("Intermedi·rna skupina: od %d do %d bodov."), ONKO_LOWER_LIMIT_INTERMEDIARY, ONKO_UPPER_LIMIT_INTERMEDIARY);
	TextOut(hPrinter, 100, 1600, wcBuffer, wcslen(wcBuffer));
	memset(wcBuffer, 0, sizeof(wcBuffer));

	wsprintfW(wcBuffer, TEXT("Sekund·rna skupina:    od %d do %d bodov."), ONKO_LOWER_LIMIT_SECONDARY, ONKO_UPPER_LIMIT_SECONDARY);
	TextOut(hPrinter, 100, 1700, wcBuffer, wcslen(wcBuffer));
	memset(wcBuffer, 0, sizeof(wcBuffer));

	wsprintfW(wcBuffer, TEXT("Terci·rna skupina:     do %d bodov."), ONKO_UPPER_LIMIT_TERTIARY);
	TextOut(hPrinter, 100, 1800, wcBuffer, wcslen(wcBuffer));
	
	delete[] wcBuffer;

	
	// finish the first page
	EndPage(hPrinter);

	// end this document and release the printer's DC
	EndDoc(hPrinter);
	DeleteDC(hPrinter);
	
	return 0;
}

int OnCreate(const HWND m_hwnd, TEXTMETRIC &m_tm, int &m_cxChar, int &m_cxCaps, int &m_cyChar, 
			 int &m_iMaxWidth, int &m_iDeltaPerLine, ULONG &m_ulScrollLines, 
			 HWND & m_hwndEditText1, HWND & m_hwndEditText2, HWND & m_hwndButtonEvaluate,
			 HDC & m_hdcEditText1, HDC & m_hdcEditText2, HDC & m_hdcButtonEvalute )
{

	HDC hdc = GetDC(m_hwnd);
	GetTextMetrics(hdc, &m_tm);
	ReleaseDC(m_hwnd, hdc);

	m_cxChar = m_tm.tmAveCharWidth;
	m_cxCaps = (m_tm.tmPitchAndFamily & 1 ? 3 : 2) * m_cxChar / 2;
	m_cyChar = m_tm.tmHeight + m_tm.tmExternalLeading;
	// Save the width of the three columns
	m_iMaxWidth = 40 * m_cxChar + 22 * m_cxCaps;

	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &m_ulScrollLines, 0);

	// ulScrollLines usually equals 3 or 0 (for no scrolling)
	// WHEEL_DELTA equals 120, so iDeltaPerLine will be 40

	if (m_ulScrollLines)
		m_iDeltaPerLine = WHEEL_DELTA / m_ulScrollLines;
	else
		m_iDeltaPerLine = 0;

	CreateControls(m_hwnd, m_hwndEditText1, m_hwndEditText2, m_hwndButtonEvaluate, m_hdcEditText1, m_hdcEditText2, m_hdcButtonEvalute);
	return 0;
}

int OnCtlColorStatic(const WPARAM m_wParam)
{
	SetTextColor((HDC)m_wParam, RGB(50, 50, 100));
	return (LPARAM)GetStockObject(DC_BRUSH);
}

int OnCtlColorEdit(const WPARAM m_wParam)
{
	SetTextColor((HDC)m_wParam, RGB(0, 0, 255));
	return (LPARAM)GetStockObject(DC_BRUSH);
}

int OnSettingChange(ULONG &m_ulScrollLines, int & m_iDeltaPerLine)
{
	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &m_ulScrollLines, 0);

	// ulScrollLines usually equals 3 or 0 (for no scrolling)
	// WHEEL_DELTA equals 120, so iDeltaPerLine will be 40

	if (m_ulScrollLines)
		m_iDeltaPerLine = WHEEL_DELTA / m_ulScrollLines;
	else
		m_iDeltaPerLine = 0;

	return 0;
}

int OnSize(const HWND m_hwnd, const LPARAM m_lParam, SCROLLINFO & m_si, int & m_cxClient, int & m_cyClient, const int m_cxChar, const int m_cyChar, const int m_iMaxWidth)
{
	m_cxClient = LOWORD(m_lParam);
	m_cyClient = HIWORD(m_lParam);

	// Set vertical scroll bar range and page size

	m_si.cbSize = sizeof(m_si);
	m_si.fMask = SIF_RANGE | SIF_PAGE;
	m_si.nMin = 0;
	m_si.nMax = NUMLINES - 1;
	m_si.nPage = m_cyClient / m_cyChar;
	SetScrollInfo(m_hwnd, SB_VERT, &m_si, TRUE);

	// Set horizontal scroll bar range and page size

	m_si.cbSize = sizeof(m_si);
	m_si.fMask = SIF_RANGE | SIF_PAGE;
	m_si.nMin = 0;
	m_si.nMax = 2 + m_iMaxWidth / m_cxChar;
	m_si.nPage = m_cxClient / m_cxChar;
	SetScrollInfo(m_hwnd, SB_HORZ, &m_si, TRUE);
	
	return 0;
}

int OnVScroll(const WPARAM m_wParam, const HWND m_hwnd, SCROLLINFO &m_si, int & m_iVertPos, const int m_cyChar)
{
	// Get all the vertical scroll bar information

	m_si.cbSize = sizeof(m_si);
	m_si.fMask = SIF_ALL;
	GetScrollInfo(m_hwnd, SB_VERT, &m_si);

	// Save the position for comparison later on

	m_iVertPos = m_si.nPos;

	switch (LOWORD(m_wParam))
	{
	case SB_TOP:
		m_si.nPos = m_si.nMin;
		break;

	case SB_BOTTOM:
		m_si.nPos = m_si.nMax;
		break;

	case SB_LINEUP:
		m_si.nPos -= 1;
		break;

	case SB_LINEDOWN:
		m_si.nPos += 1;
		break;

	case SB_PAGEUP:
		m_si.nPos -= m_si.nPage;
		break;

	case SB_PAGEDOWN:
		m_si.nPos += m_si.nPage;
		break;

	case SB_THUMBTRACK:
		m_si.nPos = m_si.nTrackPos;
		break;

	default:
		break;
	}
	m_si.fMask = SIF_POS;
	SetScrollInfo(m_hwnd, SB_VERT, &m_si, TRUE);
	GetScrollInfo(m_hwnd, SB_VERT, &m_si);

	// If the position has changed, scroll the window and update it

	if (m_si.nPos != m_iVertPos)
	{
		ScrollWindow(m_hwnd, 0, m_cyChar * (m_iVertPos - m_si.nPos), NULL, NULL);
		UpdateWindow(m_hwnd);
	}
	return 0;
}

int OnKeyDown(const HWND m_hwnd, const WPARAM m_wParam)
{
	switch (m_wParam)
	{
		case VK_HOME:	return SendMessage(m_hwnd, WM_VSCROLL, SB_TOP, 0); break;
		case VK_END:	return SendMessage(m_hwnd, WM_VSCROLL, SB_BOTTOM, 0); break;
		case VK_PRIOR:	return SendMessage(m_hwnd, WM_VSCROLL, SB_PAGEUP, 0); break;
		case VK_NEXT:	return SendMessage(m_hwnd, WM_VSCROLL, SB_PAGEDOWN, 0); break;
		case VK_UP:		return SendMessage(m_hwnd, WM_VSCROLL, SB_LINEUP, 0); break;
		case VK_DOWN:	return SendMessage(m_hwnd, WM_VSCROLL, SB_LINEDOWN, 0); break;
		case VK_LEFT:	return SendMessage(m_hwnd, WM_HSCROLL, SB_PAGEUP, 0); break;
		case VK_RIGHT:	return SendMessage(m_hwnd, WM_HSCROLL, SB_PAGEDOWN, 0); break;
		case VK_RETURN: return SendMessage(m_hwnd, WM_COMMAND, BN_CLICKED, 0); break;
	}
	return 0;
}

int OnMouseWheel(const HWND m_hwnd, const WPARAM m_wParam, const int m_iDeltaPerLine, int & m_iAccumDelta)
{
	if (m_iDeltaPerLine == 0)
		return -1;
	
	m_iAccumDelta += (short)HIWORD(m_wParam);     // 120 or -120

	while (m_iAccumDelta >= m_iDeltaPerLine)
	{
		SendMessage(m_hwnd, WM_VSCROLL, SB_LINEUP, 0);
		m_iAccumDelta -= m_iDeltaPerLine;
	}

	while (m_iAccumDelta <= -m_iDeltaPerLine)
	{
		SendMessage(m_hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
		m_iAccumDelta += m_iDeltaPerLine;
	}
	
	return 0;
}

int OnPaint(const HWND m_hwnd, PAINTSTRUCT & m_ps, SCROLLINFO & m_si, int & m_iVertPos, int & m_iHorzPos)
{
	//static int i = 0;
	BeginPaint(m_hwnd, &m_ps);
	EndPaint(m_hwnd, &m_ps);
	// Get vertical scroll bar position

	m_si.cbSize = sizeof(m_si);
	m_si.fMask = SIF_POS;
	if (i++ == 0)
		GetScrollInfo(m_hwnd, SB_VERT, &m_si);
	m_iVertPos = m_si.nPos;

	// Get horizontal scroll bar position

	//GetScrollInfo(hwnd, SB_HORZ, &si);
	m_iHorzPos = m_si.nPos;

	return 0;
}

int OnCommand(const HINSTANCE m_hInstance, const HWND m_hwnd, const WPARAM m_wParam, const LPARAM m_lParam, const HWND m_hwndButtonEvaluate)
{
	int  nLength;
	int  nIdx;
	UINT uiContent;
	

	// udalost akehokolvek controlu v okne 
	// odchytavam udalost pre combobox v HIWORD(wParam). 
	// v lParam handle toho comboboxu, ktory poslal udalost, 
	// preto nemusim rozlisovat explicitne cez switch CASE
	// ale odkazem sa na handle, ktory dostanem
	switch (HIWORD(m_wParam))
	{
		case CBN_SELCHANGE: //odchytavam comboboxy ci uz ked rozbali combobox kliknutim alebo klavesou
		case CBN_DROPDOWN:
			nIdx = SendMessage((HWND)m_lParam, CB_FINDSTRINGEXACT, -1, LPARAM(L"<Vyber odpoveÔ>"));
			if (nIdx != CB_ERR)
			{
				SendMessage((HWND)m_lParam, CB_DELETESTRING, nIdx, 0);
				SendMessage((HWND)m_lParam, CB_SETCURSEL, 0, 0);
			}
			break;
		
		// Ak pole EditText1 alebo EditText2 strati focus, kontrola ci:
		// a) nie je pole prazdne, ak je, tak upozorni a vrat focus na pole
		// b) nie je zadana 0, ak je, tak upozorni, oznac text a vrat focus na pole
		// parametre spravy EN_KILLFOCUS
		// lParam - handle na pole, ktoreho sa to tyka
		// LOWORD(wParam) id pola, ktore sa to tyka
		case EN_KILLFOCUS:
			nLength = 0;
			uiContent = -1;
			nLength = GetWindowTextLength((HWND)m_lParam);
			if (nLength == 0)
			{
				MessageBox(m_hwnd, TEXT("MusÌte zadaù ËÌseln˙ hodnotu. Pole nemÙûe ostaù pr·zdne!"), TEXT("Chyba!"), MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
				SendMessage((HWND)m_lParam, EM_SETSEL, 0, nLength);
				SetFocus((HWND)m_lParam);
			}
			else
			{
				uiContent = GetDlgItemInt(m_hwnd, LOWORD(m_wParam), NULL, FALSE);
				if (uiContent <= 0)
				{
					MessageBox(m_hwnd, TEXT("Neplatn· hodnota! MusÌte zadaù ËÌseln˙ hodnotu v intervale 1 - 999."), TEXT("Chyba!"), MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
					SendMessage((HWND)m_lParam, EM_SETSEL, 0, nLength);
					SetFocus((HWND)m_lParam);
				}
			}
			break;

		case BN_CLICKED:
			if (LOWORD(m_wParam) == IDC_BUTTON_EVALUATE)
			{
				if (VerifyComboStatus(m_hwnd))
				{
					int nRes = CalculateResult(m_hwnd);
					ShowResult(m_hInstance, m_hwnd, nRes, m_hwndButtonEvaluate);
					/*if (MessageBox(m_hwnd, TEXT("éel·te si vytlaËiù v˝sledky?"), TEXT("TLA»"), MB_YESNO) == IDYES)
					{
						PrintResult(nRes);
					}*/
				}
			}
			else if (LOWORD(m_wParam) <= IDC_BUTTON15 && LOWORD(m_wParam) >= IDC_BUTTON1)
			{
				int nBtnIdx = LOWORD(m_wParam) - IDC_BUTTON1;
				SetWindowText(buttons[nBtnIdx].hwndButton, buttons[nBtnIdx].bButton ? TEXT("NIE") : TEXT("¡NO"));
				buttons[nBtnIdx].bButton = buttons[nBtnIdx].bButton ? FALSE : TRUE;
			}
			break;
		
		}
	return 0;
}

int OnDestroy(const HWND m_hwndEditText1, const HWND m_hwndEditText2, const HWND m_hwndButtonEvaluate,
			  const HDC m_hdcEditText1, const HDC m_hdcEditText2, const HDC m_hdcButtonEvaluate)
{
	ReleaseControls(m_hwndEditText1, m_hwndEditText2, m_hwndButtonEvaluate, m_hdcEditText1, m_hdcEditText2, m_hdcButtonEvaluate); // uvolnit vsetky handle, ktore som alokoval v CreateControls()
	PostQuitMessage(0);
	return 0;
}


LRESULT CALLBACK ResultDlgProc(HWND m_hWndResDlg, UINT m_uMsg, WPARAM m_wParam, LPARAM m_lParam)
{
	switch (m_uMsg)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_KEYDOWN:
		if (m_wParam == VK_ESCAPE)
			EndDialog(m_hWndResDlg, 0);
			return TRUE;
	case WM_COMMAND:
		switch (m_wParam)
		{
		case IDOK:
			EndDialog(m_hWndResDlg, 0);
			return TRUE;
		case IDPRINT:
			PrintResult(CalculateResult(GetParent(m_hWndResDlg)));
			EndDialog(m_hWndResDlg, 0);
			return TRUE;
		}
		break;
	}

	return FALSE;
}
