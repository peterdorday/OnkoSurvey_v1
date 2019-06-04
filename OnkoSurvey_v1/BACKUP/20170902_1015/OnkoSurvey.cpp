#include <Windows.h>
#include <WinUser.h>
#include <stdio.h>
#include "OnkoSurvey.h"
#include "resource.h"


/*************************
	GLOBAL VARIABLES
***************************/
HWND hwndEditText1;
HDC hdcEditText1;

HWND hwndEditText2;
HDC hdcEditText2;

HWND hwndButtonEvaluate;
HDC hdcButtonEvalute;

struct tagButton buttons[NUM_OF_BUTTONS];
struct tagCombobox combos[NUM_OF_COMBOS];


static int i = 0; // pre scrollbar




/**********************************
	FUNCTION DECLARATIONS
**********************************/
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); //standardna procedura okna
static WNDPROC OriginalEditCtrlProc = NULL; // sem si ulozim originalnu alebo custom proceduru
LRESULT CALLBACK EditWndProc(HWND, UINT, WPARAM, LPARAM); // subclassing - custom procedura len pre Edit boxy...

void CreateControls(HWND hwnd);
void ReleaseControls();
BOOL VerifyComboStatus(HWND hwnd);
int CalculateResult(HWND hwnd);
void ShowResult(HWND hwnd, int nResult);
int PrintResult(HWND hwnd, int nResult);

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
	HDC         hdc;
	int         iVertPos, iHorzPos;
	static SCROLLINFO  si;
	TEXTMETRIC  tm;
	ULONG       ulScrollLines;                  // for mouse wheel logic
	PAINTSTRUCT	ps;

	switch (message)
	{
	case WM_CREATE:
		hdc = GetDC(hwnd);
		GetTextMetrics(hdc, &tm);
		ReleaseDC(hwnd, hdc);

		cxChar = tm.tmAveCharWidth;
		cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		// Save the width of the three columns
		iMaxWidth = 40 * cxChar + 22 * cxCaps;

		SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ulScrollLines, 0);

		// ulScrollLines usually equals 3 or 0 (for no scrolling)
		// WHEEL_DELTA equals 120, so iDeltaPerLine will be 40

		if (ulScrollLines)
			iDeltaPerLine = WHEEL_DELTA / ulScrollLines;
		else
			iDeltaPerLine = 0;

		CreateControls(hwnd);
		return 0;

	case WM_CTLCOLORSTATIC: // farba fontu STATIC poli - cize labelov
		SetTextColor((HDC)wParam, RGB(50, 50, 100));
		return (LPARAM)GetStockObject(DC_BRUSH);

	case WM_CTLCOLOREDIT: //farba fontu EDIT poli - cize textboxov a comboboxov
		SetTextColor((HDC)wParam, RGB(0, 0, 255));
		return (LPARAM)GetStockObject(DC_BRUSH);

	case WM_SETTINGCHANGE:
		SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ulScrollLines, 0);

		// ulScrollLines usually equals 3 or 0 (for no scrolling)
		// WHEEL_DELTA equals 120, so iDeltaPerLine will be 40

		if (ulScrollLines)
			iDeltaPerLine = WHEEL_DELTA / ulScrollLines;
		else
			iDeltaPerLine = 0;

		return 0;

	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);

		// Set vertical scroll bar range and page size

		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = NUMLINES - 1;
		si.nPage = cyClient / cyChar;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

		// Set horizontal scroll bar range and page size

		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = 2 + iMaxWidth / cxChar;
		si.nPage = cxClient / cxChar;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
		return 0;

	case WM_VSCROLL:
		// Get all the vertical scroll bar information

		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd, SB_VERT, &si);

		// Save the position for comparison later on

		iVertPos = si.nPos;

		switch (LOWORD(wParam))
		{
		case SB_TOP:
			si.nPos = si.nMin;
			break;

		case SB_BOTTOM:
			si.nPos = si.nMax;
			break;

		case SB_LINEUP:
			si.nPos -= 1;
			break;

		case SB_LINEDOWN:
			si.nPos += 1;
			break;

		case SB_PAGEUP:
			si.nPos -= si.nPage;
			break;

		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;

		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;

		default:
			break;
		}
		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hwnd, SB_VERT, &si);

		// If the position has changed, scroll the window and update it

		if (si.nPos != iVertPos)
		{
			ScrollWindow(hwnd, 0, cyChar * (iVertPos - si.nPos), NULL, NULL);
			UpdateWindow(hwnd);
		}
		return 0;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_HOME:
			SendMessage(hwnd, WM_VSCROLL, SB_TOP, 0);
			break;

		case VK_END:
			SendMessage(hwnd, WM_VSCROLL, SB_BOTTOM, 0);
			break;

		case VK_PRIOR:
			SendMessage(hwnd, WM_VSCROLL, SB_PAGEUP, 0);
			break;

		case VK_NEXT:
			SendMessage(hwnd, WM_VSCROLL, SB_PAGEDOWN, 0);
			break;

		case VK_UP:
			SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0);
			break;

		case VK_DOWN:
			SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
			break;

		case VK_LEFT:
			SendMessage(hwnd, WM_HSCROLL, SB_PAGEUP, 0);
			break;

		case VK_RIGHT:
			SendMessage(hwnd, WM_HSCROLL, SB_PAGEDOWN, 0);
			break;
		case VK_RETURN:
			SendMessage(hwnd, WM_COMMAND, BN_CLICKED, 0);
			break;
		}
		return 0;

	case WM_MOUSEWHEEL:
		if (iDeltaPerLine == 0)
			break;

		iAccumDelta += (short)HIWORD(wParam);     // 120 or -120

		while (iAccumDelta >= iDeltaPerLine)
		{
			SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0);
			iAccumDelta -= iDeltaPerLine;
		}

		while (iAccumDelta <= -iDeltaPerLine)
		{
			SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
			iAccumDelta += iDeltaPerLine;
		}

		return 0;

	case WM_PAINT:
		BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		// Get vertical scroll bar position

		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		if (i++ == 0)
			GetScrollInfo(hwnd, SB_VERT, &si);
		iVertPos = si.nPos;

		// Get horizontal scroll bar position

		//GetScrollInfo(hwnd, SB_HORZ, &si);
		iHorzPos = si.nPos;

		return 0;

	case WM_COMMAND: // udalost akehokolvek controlu v okne
					 // odchytavam udalost pre combobox v HIWORD(wParam). 
					 // v lParam handle toho comboboxu, ktory poslal udalost, 
					 // preto nemusim rozlisovat explicitne cez switch CASE
					 // ale odkazem sa na handle, ktory dostanem
		switch (HIWORD(wParam))
		{
			case CBN_SELCHANGE: //odchytavam comboboxy ci uz ked rozbali combobox kliknutim alebo klavesou
			case CBN_DROPDOWN:
			{
				int nIdx = SendMessage((HWND)lParam, CB_FINDSTRINGEXACT, -1, LPARAM(L"<Vyber odpoveÔ>"));
				if (nIdx != CB_ERR)
				{
					SendMessage((HWND)lParam, CB_DELETESTRING, nIdx, 0);
					SendMessage((HWND)lParam, CB_SETCURSEL, 0, 0);
				}
				break;
			}
			// Ak pole EditText1 alebo EditText2 strati focus, kontrola ci:
			// a) nie je pole prazdne, ak je, tak upozorni a vrat focus na pole
			// b) nie je zadana 0, ak je, tak upozorni, oznac text a vrat focus na pole
			// parametre spravy EN_KILLFOCUS
			// lParam - handle na pole, ktoreho sa to tyka
			// LOWORD(wParam) id pola, ktore sa to tyka
			case EN_KILLFOCUS:
			{
				int length = 0;
				UINT content = -1;
				length = GetWindowTextLength((HWND)lParam);
				if (length == 0)
				{
					MessageBox(hwnd, TEXT("MusÌte zadaù ËÌseln˙ hodnotu. Pole nemÙûe ostaù pr·zdne!"), TEXT("Chyba!"), MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
					SendMessage((HWND)lParam, EM_SETSEL, 0, length);
					SetFocus((HWND)lParam);
				}
				else
				{
					content = GetDlgItemInt(hwnd, LOWORD(wParam), NULL, FALSE);
					if (content <= 0)
					{
						MessageBox(hwnd, TEXT("Neplatn· hodnota! MusÌte zadaù ËÌseln˙ hodnotu v intervale 1 - 999."), TEXT("Chyba!"), MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
						SendMessage((HWND)lParam, EM_SETSEL, 0, length);
						SetFocus((HWND)lParam);
					}
				}

				//break;
			}
		
			case BN_CLICKED:
			{
				if (LOWORD(wParam) == IDC_BUTTON_EVALUATE)
				{
					if (VerifyComboStatus(hwnd))
					{ 
						int nRes = CalculateResult(hwnd);
						ShowResult(hwnd, nRes);
						if (MessageBox(hwnd, TEXT("éel·te si vytlaËiù v˝sledky?"), TEXT("TLA»"), MB_YESNO) == IDYES) 
						{
							PrintResult(hwnd, nRes);
						}
					}
				}
				else if (LOWORD(wParam) <= IDC_BUTTON15 && LOWORD(wParam) >= IDC_BUTTON1)
				{ 
					int nBtnIdx = LOWORD(wParam) - IDC_BUTTON1;
					SetWindowText(buttons[nBtnIdx].hwndButton, buttons[nBtnIdx].bButton ? TEXT("NIE") : TEXT("¡NO"));
					buttons[nBtnIdx].bButton = buttons[nBtnIdx].bButton ? FALSE : TRUE;
				}
				break;
			}
		}

		return 0;

		case WM_DESTROY:
		{
			ReleaseControls(); // uvolnit vsetky handle, ktore som alokoval
			PostQuitMessage(0);
			return 0;
		}
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}


void CreateControls(HWND hwnd)
{
	// label for Question #1
	CreateWindow(TEXT("Static"), TEXT("1. Uh·dol, ak˝ je dnes d·tum?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 10, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #1
	buttons[0].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 10, 60, 25,
		hwnd, (HMENU)IDC_BUTTON1,
		NULL, NULL); 

	CreateWindow(TEXT("Static"), TEXT("2. Uh·dol, ak˝ je dnes deÚ v t˝ûdni?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 40, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #2
	buttons[1].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 40, 60, 25,
		hwnd, (HMENU)IDC_BUTTON2,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("3. Uh·dol, ak˝ je teraz rok?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 70, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #3
	buttons[2].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 70, 60, 25,
		hwnd, (HMENU)IDC_BUTTON3,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("4. Uh·dol, akÈ je teraz roËnÈ obdobie?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 100, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #4
	buttons[3].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 100, 60, 25,
		hwnd, (HMENU)IDC_BUTTON4,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("5. Uh·dol, na ktorom sme poschodÌ?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 130, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #5
	buttons[4].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 130, 60, 25,
		hwnd, (HMENU)IDC_BUTTON5,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("6. Poviem V·m tri predmety. Budem chcieù, aby ste ich zopakovali. O chvÌæu sa V·s na tieto slov· op‰ù op˝tam: \"LOPTA, ä¡L, V¡ZA\"."),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 160, 900, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	CreateWindow(TEXT("Static"), TEXT("  a) Uh·dol slovo \"LOPTA\"?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 190, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #6a - button6
	buttons[5].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 190, 60, 25,
		hwnd, (HMENU)IDC_BUTTON6,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("  b) Uh·dol slovo \"ä¡L\"?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 220, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #6b - button7
	buttons[6].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 220, 60, 25,
		hwnd, (HMENU)IDC_BUTTON7,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("  c) Uh·dol slovo \"V¡ZA\"?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 250, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #6c - button8
	buttons[7].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 250, 60, 25,
		hwnd, (HMENU)IDC_BUTTON8,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("7. Vezmite do ruky papier, ktor˝ m·te pred sebou, preloûte ho na polovicu a poloûte na podlahu"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 280, 900, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	CreateWindow(TEXT("Static"), TEXT("  a) Uchopil papier?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 310, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #7a - button9
	buttons[8].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 310, 60, 25,
		hwnd, (HMENU)IDC_BUTTON9,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("  b) Preloûil papier na polovicu?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 340, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #7b - button10
	buttons[9].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 340, 60, 25,
		hwnd, (HMENU)IDC_BUTTON10,
		NULL, NULL);

	CreateWindow(TEXT("Static"), TEXT("  c) Poloûil papier na podlahu?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 370, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #7c - button11
	buttons[10].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 370, 60, 25,
		hwnd, (HMENU)IDC_BUTTON11,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("8. Teraz mi povedzte 3 slov·, ktorÈ ste mali zopakovaù."),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 400, 900, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	CreateWindow(TEXT("Static"), TEXT("  a) Povedal slovo \"LOPTA?\""),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 430, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #8a - button12
	buttons[11].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 430, 60, 25,
		hwnd, (HMENU)IDC_BUTTON12,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("  b) Povedal slovo \"ä¡L?\""),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 460, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #8b - button13
	buttons[12].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 460, 60, 25,
		hwnd, (HMENU)IDC_BUTTON13,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("  c) Povedal slovo \"V¡ZA?\""),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 490, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// button for Answer #8c - button14
	buttons[13].hwndButton = CreateWindow(TEXT("Button"), TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 490, 60, 25,
		hwnd, (HMENU)IDC_BUTTON14,
		NULL, NULL);
	
	CreateWindow(TEXT("Static"), TEXT("9. Ako sa stravujete?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 520, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// combobox #1 pre otazku 9
	combos[0].hwndComboBox = CreateWindow(TEXT("combobox"), NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
		330, 520, 330, 250,
		hwnd, (HMENU)IDC_COMBOBOX1,
		NULL, NULL);
	SendMessage(combos[0].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("<Vyber odpoveÔ>"));
	SendMessage(combos[0].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Neschopn˝ jesù s·m bez pomoci"));
	SendMessage(combos[0].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("S·m s ùaûkosùami"));
	SendMessage(combos[0].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("S·m bez ùaûkostÌ"));
	SendMessage(combos[0].hwndComboBox, CB_SETCURSEL, 0, 0);

	CreateWindow(TEXT("Static"), TEXT("10. Koæko tekutÌn prijmete za deÚ?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 550, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// combobox #2 pre otazku 10
	combos[1].hwndComboBox = CreateWindow(TEXT("combobox"), NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
		330, 550, 330, 250,
		hwnd, (HMENU)IDC_COMBOBOX2,
		NULL, NULL);
	SendMessage(combos[1].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("<Vyber odpoveÔ>"));
	SendMessage(combos[1].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Menej ako 900ml"));
	SendMessage(combos[1].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Medzi 900ml a 1500ml"));
	SendMessage(combos[1].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Viac ako 1500ml"));
	SendMessage(combos[1].hwndComboBox, CB_SETCURSEL, 0, 0);

	CreateWindow(TEXT("Static"), TEXT("11. UdrûÌte moË?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 580, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// combobox #3 pre otazku 11
	combos[2].hwndComboBox = CreateWindow(TEXT("combobox"), NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
		330, 580, 330, 250,
		hwnd, (HMENU)IDC_COMBOBOX3,
		NULL, NULL);
	SendMessage(combos[2].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("<Vyber odpoveÔ>"));
	SendMessage(combos[2].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("PomoËÌ sa viac ako 1x denne alebo m· PK"));
	SendMessage(combos[2].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("ObËas nie (max 1x denne neudrûÌ)"));
	SendMessage(combos[2].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("¡no"));
	SendMessage(combos[2].hwndComboBox, CB_SETCURSEL, 0, 0);


	CreateWindow(TEXT("Static"), TEXT("12. Pohyblivosù pacienta?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 610, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// combobox #4 pre otazku 12
	combos[3].hwndComboBox = CreateWindow(TEXT("combobox"), NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
		330, 610, 330, 250,
		hwnd, (HMENU)IDC_COMBOBOX4,
		NULL, NULL);
	SendMessage(combos[3].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("<Vyber odpoveÔ>"));
	SendMessage(combos[3].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Prip˙tan˝ na lÙûko"));
	SendMessage(combos[3].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("ChodÌ po byte"));
	SendMessage(combos[3].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Vych·dza z bytu"));
	SendMessage(combos[3].hwndComboBox, CB_SETCURSEL, 0, 0);


	CreateWindow(TEXT("Static"), TEXT("13. Osobn· hygiena pacienta?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 640, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// combobox #5 pre otazku 13
	combos[4].hwndComboBox = CreateWindow(TEXT("combobox"), NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
		330, 640, 330, 250,
		hwnd, (HMENU)IDC_COMBOBOX5,
		NULL, NULL);
	SendMessage(combos[4].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("<Vyber odpoveÔ>"));
	SendMessage(combos[4].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Potrebuje pomoc"));
	SendMessage(combos[4].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("SebestaËn˝ pri um˝vanÌ, holenÌ, ËesanÌ..."));
	SendMessage(combos[4].hwndComboBox, CB_SETCURSEL, 0, 0);

	CreateWindow(TEXT("Static"), TEXT("14. Obliekanie?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 670, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// combobox #6 pre otazku 14
	combos[5].hwndComboBox = CreateWindow(TEXT("combobox"), NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
		330, 670, 330, 250,
		hwnd, (HMENU)IDC_COMBOBOX6,
		NULL, NULL);
	SendMessage(combos[5].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("<Vyber odpoveÔ>"));
	SendMessage(combos[5].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Nedok·ûe sa obliecù"));
	SendMessage(combos[5].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Oblieka sa s pomocou (asi polovicu zvl·dne s·m)"));
	SendMessage(combos[5].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TEXT("Oblieka sa s·m"));
	SendMessage(combos[5].hwndComboBox, CB_SETCURSEL, 0, 0);

	CreateWindow(L"Static", TEXT("15. UûÌvate viac ako 3 lieky?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 700, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// button for Question 14 - button15
	buttons[14].hwndButton = CreateWindow(L"Button", TEXT("NIE"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE | WS_TABSTOP,
		330, 700, 60, 25,
		hwnd, (HMENU)IDC_BUTTON15,
		NULL, NULL);
	
	CreateWindowEx(WS_EX_TRANSPARENT, L"Static", TEXT("16. ⁄bytok hmotnosti za posledn˝ mesiac?"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 730, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	// combobox #7 pre otazku 16
	combos[6].hwndComboBox = CreateWindow(L"combobox", NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
		330, 730, 330, 250,
		hwnd, (HMENU)IDC_COMBOBOX7, NULL, NULL);
	SendMessage(combos[6].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)L"<Vyber odpoveÔ>");
	SendMessage(combos[6].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)L"Viac ako 3 kg");
	SendMessage(combos[6].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)L"Nevie udaù");
	SendMessage(combos[6].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)L"1 - 2 kg");
	SendMessage(combos[6].hwndComboBox, CB_ADDSTRING, 0, (LPARAM)L"Bez ˙bytku");
	SendMessage(combos[6].hwndComboBox, CB_SETCURSEL, 0, 0);

	CreateWindow(L"Static", TEXT("V˝öka (cm)"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 760, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	hwndEditText1 = CreateWindow(L"Edit", TEXT("150"),
		WS_CHILD | WS_VISIBLE | ES_CENTER | WS_BORDER | ES_NUMBER | WS_TABSTOP,
		330, 760, 60, 25,
		hwnd, (HMENU)IDC_EDITTEXT1,
		NULL, NULL);
	SendMessage(hwndEditText1, EM_SETLIMITTEXT, 3, NULL); //nastav max pocet znakov v EDIT na 3
	if (hwndEditText1 != NULL)
	{
		WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(hwndEditText1, GWLP_WNDPROC, (LONG_PTR)EditWndProc);
		if (OriginalEditCtrlProc == NULL)
		{
			OriginalEditCtrlProc = oldProc;
		}
	}

	CreateWindow(L"Static", TEXT("cm"),
		WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER,
		400, 760, 30, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	CreateWindow(L"Static", TEXT("V·ha (kg)"),
		WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
		10, 790, 300, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	hwndEditText2 = CreateWindow(L"Edit", TEXT("75"),
		WS_CHILD | WS_VISIBLE | ES_CENTER | WS_BORDER | ES_NUMBER | WS_TABSTOP,
		330, 790, 60, 25,
		hwnd, (HMENU)IDC_EDITTEXT2,
		NULL, NULL);
	SendMessage(hwndEditText2, EM_SETLIMITTEXT, 3, NULL); //nastav max pocet znakov v EDIT na 3
	if (hwndEditText2 != NULL)
	{
		WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(hwndEditText2, GWLP_WNDPROC, (LONG_PTR)EditWndProc);
		if (OriginalEditCtrlProc == NULL)
		{
			OriginalEditCtrlProc = oldProc;
		}
	}

	CreateWindow(L"Static", TEXT("kg"),
		WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER,
		400, 790, 30, 25,
		hwnd, (HMENU)1,
		NULL, NULL);

	hwndButtonEvaluate = CreateWindow(L"Button", TEXT("VYHODNOTIç"),
		WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		10, 820, 900, 100,
		hwnd, (HMENU)IDC_BUTTON_EVALUATE,
		NULL, NULL);
	hdcButtonEvalute = GetDC(hwndButtonEvaluate);

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

void ReleaseControls()
{
	for (int i = 0; i < NUM_OF_BUTTONS; i++)
	{
		ReleaseDC(buttons[i].hwndButton, buttons[i].hdcButton);
	}
	for (int i = 0; i < NUM_OF_COMBOS; i++)
	{
		ReleaseDC(combos[i].hwndComboBox, combos[i].hdcComboBox);
	}
	ReleaseDC(hwndEditText1, hdcEditText1);
	ReleaseDC(hwndEditText2, hdcEditText2);
	ReleaseDC(hwndButtonEvaluate, hdcButtonEvalute);
}

LRESULT CALLBACK EditWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// sice som umoznil zadavat do EDIT len ciselne hodnoty (ES_NUMBER), 
	// uzivatel vsak stale moze urobit ctrl+v, preto to odchytavam a zahadzujem
	// zachytava aj "paste" cez kontextove menu ...
	if (uMsg == WM_PASTE)
		return 0;

	return CallWindowProc(OriginalEditCtrlProc, hwnd, uMsg, wParam, lParam);
}

BOOL VerifyComboStatus(HWND hwnd)
{
	wchar_t *strText = new wchar_t[256]; // potrebujem wchar_t* ako ekvivalent LPWSTR
	for (int i = 0; i < NUM_OF_COMBOS; i++)
	{
		if (combos[i].bInitialRowComboBox == TRUE)
		{
			GetWindowText(combos[i].hwndComboBox, strText, 256);
			if (wcscmp(strText, L"<Vyber odpoveÔ>") == 0)
			{
				MessageBox(hwnd, TEXT("Vyberte poloûku!"), TEXT("Chyba!"), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
				SetFocus(combos[i].hwndComboBox);
				return FALSE;
			}
		}
	}
	delete[] strText;
	return TRUE;
}

int CalculateResult(HWND hwnd)
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

	nVyska = GetDlgItemInt(hwnd,IDC_EDITTEXT1, NULL, FALSE);
	nVaha = GetDlgItemInt(hwnd, IDC_EDITTEXT2, NULL, FALSE);


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

void ShowResult(HWND hwnd, int nResult)
{
	LPWSTR wcAnswer = new wchar_t[1000];
	LPWSTR wcGroup = new wchar_t[20];

	wsprintfW(wcAnswer, TEXT("Pacient dosiahol %d bodov.\nPacient patrÌ do %lS SKUPINY.\n\n"
		"Prim·rna skupina: od %d bodov.\nIntermedi·rna skupina: od %d do %d bodov.\nSekund·rna skupina: od %d do %d bodov.\n"
		"Terci·rna skupina: od %d do %d bodov."),
		nResult,
		(nResult >= ONKO_LOWER_LIMIT_PRIMARY) ? TEXT("PRIM¡RNEJ") : \
				(nResult >= ONKO_LOWER_LIMIT_INTERMEDIARY && nResult <= ONKO_UPPER_LIMIT_INTERMEDIARY) ? TEXT("INTERMEDI¡RNEJ") : \
				(nResult >= ONKO_LOWER_LIMIT_SECONDARY && nResult <= ONKO_UPPER_LIMIT_SECONDARY) ? TEXT("SEKUND¡RNEJ") : TEXT("TERCI¡RNEJ"),
		ONKO_LOWER_LIMIT_PRIMARY,
		ONKO_LOWER_LIMIT_INTERMEDIARY,
		ONKO_UPPER_LIMIT_INTERMEDIARY,
		ONKO_LOWER_LIMIT_SECONDARY,
		ONKO_UPPER_LIMIT_SECONDARY,
		ONKO_LOWER_LIMIT_TERTIARY,
		ONKO_UPPER_LIMIT_TERTIARY
	);
	
	MessageBoxW(hwnd, wcAnswer, L"VYHODNOTENIE", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
	SetFocus(hwndButtonEvaluate);

	delete[] wcAnswer;
	delete[] wcGroup;
}

int PrintResult(HWND hwnd, int nResult)
{
	PRINTDLG prDlg;
	memset(&prDlg, 0, sizeof(prDlg));
	prDlg.lStructSize = sizeof(prDlg);

	/*
	get rid of PD_RETURNDEFAULT on the line below if you'd like to
	see the "Printer Settings" dialog!
	*/
	//prDlg.Flags = PD_RETURNDEFAULT | PD_RETURNDC;
	prDlg.Flags = PD_RETURNDC;
	// try to retrieve the printer DC
	if (!PrintDlg(&prDlg))
	{
		MessageBoxW(NULL, TEXT("TlaË zlyhala!"), TEXT("Chyba"), MB_OK | MB_ICONERROR);
		return -1;
	}

	DOCINFO  document_Information;
	HDC hPrinter = prDlg.hDC;

	// initialization of the printing!
	memset(&document_Information, 0, sizeof(document_Information));
	document_Information.cbSize = sizeof(document_Information);
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
	LPCWSTR wcUnderLine = TEXT("===================================================================");
	TextOut(hPrinter, 1100, 600, wcTitle, wcslen(wcTitle));
	TextOut(hPrinter, 900, 700, wcUnderLine, wcslen(wcUnderLine));


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