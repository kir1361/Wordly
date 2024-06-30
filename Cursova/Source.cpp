#include <windows.h>
#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>
#include <Lmcons.h>
#include <ctime>
#include "resource1.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace std;


// Глобальні змінні:
HINSTANCE hInst; 	//Дескриптор програми	
LPCTSTR szWindowClass = L"QWERTY";
LPCTSTR szTitle = L"Crossword";
vector<wstring> EnteredWord;
wstring SecretWord;
bool allCellsFilled = true;
COLORREF color = RGB(255, 255, 255);
HWND cells[6][5];
int currentRow = 0;
int currentCell = 0;
int mistakesCount = 0;
int enteredWordCount = 0;

HFONT CreateCustomFont(int size)
{
	return CreateFont
	(
		size, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, UNICODE,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_SWISS, L"TimesNewRoman"
	);
}
HFONT hFont = CreateCustomFont(40);

void ApplyCustomFontToAllEditBoxes(HFONT hFont, HWND cells[6][5]) {
	for (int row = 0; row < 6; row++) {
		for (int col = 0; col < 5; col++) {
			HWND hEditBox = cells[row][col];
			SendMessage(hEditBox, WM_SETFONT, (WPARAM)hFont, TRUE);
		}
	}
}

void InitializeEditBoxes(HWND hwndParent) {
	for (int row = 0; row < 6; row++) {
		for (int col = 0; col < 5; col++) {
			int id = 1061 + row * 5 + col;

			cells[row][col] = GetDlgItem(hwndParent, id);


		}
	}
}


vector<wstring> loadWordsFromFile(const string& filename) {
	vector<wstring> words;
	wifstream file(filename);

	file.imbue(locale(file.getloc(), new codecvt_utf8<wchar_t, 0x10ffff, codecvt_mode::little_endian>));


	if (file.is_open()) {
		wstring word;	
		while (getline(file, word)) {
			words.push_back(word);
		}
		file.close();
	}
	else {
		cerr << "Error" << filename << endl;
	}

	return words;
}

wstring chooseRandomWord(const vector<wstring>& words) {
	if (words.empty()) {
		cerr << "No words available to choose from." << endl;
		return wstring();
	}

	mt19937 rng(time(nullptr));
	uniform_int_distribution<size_t> dist(0, words.size() - 1);

	return words[dist(rng)];
}


void startNewGame()
{
	string filePath = "words.txt";

	vector<wstring> words = loadWordsFromFile(filePath);

	SecretWord.clear();
	SecretWord = chooseRandomWord(words);
	currentRow = 0;
	currentCell = 0;
}

wstring GetUserName() 
{
	wchar_t username[UNLEN + 1];
	DWORD username_len = UNLEN + 1;

	if (GetUserName(username, &username_len)) 
	{
		return username;
	}
	else 
	{
		return L"UnknownUser";
	}

}

void WriteResultToFile(const wstring& secretWord, int attempts, bool success) 
{
	string ResultfilePath = "result.txt";
	wofstream resultFile(ResultfilePath, ios::app);
	if (!resultFile.is_open()) {
		cerr << "Error opening results file." << endl;
		return;
	}

	resultFile.imbue(locale(resultFile.getloc(), new codecvt_utf8<wchar_t, 0x10ffff, codecvt_mode::little_endian>));


	time_t t = time(nullptr);
	tm localTime;
	localtime_s(&localTime, &t);

	wchar_t dateTime[100];
	wcsftime(dateTime, sizeof(dateTime), L"%d.%m.%Y", &localTime);

	wstring username = GetUserName();

	resultFile << secretWord << L": "
		<< (success ? L"вгадано" : L"не вгадано")
		<< L", спроб: " << attempts
		<< L", дата: " << dateTime
		<< L", користувач: " << username
		<< endl;

	resultFile.close();
}
vector<wstring> LoadResultsFromFile() 
{
	string ResultfilePath = "result.txt";
	wifstream resultFile(ResultfilePath);

	vector<wstring> Resultwords;

	resultFile.imbue(locale(resultFile.getloc(), new codecvt_utf8<wchar_t, 0x10ffff, codecvt_mode::little_endian>));


	if (resultFile.is_open()) {
		wstring ResultWord;
		while (getline(resultFile, ResultWord)) {
			Resultwords.push_back(ResultWord);
		}
		resultFile.close();
	}
	else {
		cerr << "Error" << ResultfilePath << endl;
	}

	return Resultwords;
}

void RestartGame() 
{
	for (int row = 0; row < 6; row++) {
		for (int col = 0; col < 5; col++) {	

			SetWindowText(cells[row][col], L""); 
		}
	}
	EnteredWord.clear();
	mistakesCount = 0;
	enteredWordCount = 0;
	startNewGame();
	SetFocus(cells[0][0]);
}
// Попередній опис функцій

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR  CALLBACK DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR  CALLBACK ResultsDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


// Основна програма 
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
	int nCmdShow)
{
	MSG msg;

	// Реєстрація класу вікна 
	MyRegisterClass(hInstance);

	DialogBox(hInstance, MAKEINTRESOURCE(101), NULL, (DLGPROC)DialogProc);
	// Створення вікна програми
	if (!	(hInstance, nCmdShow))
	{
		return FALSE;
	}
	// Цикл обробки повідомлень
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW ; 		//стиль вікна
	wcex.lpfnWndProc = (WNDPROC)WndProc; 		//віконна процедура
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance; 			//дескриптор програми
	wcex.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_ICON1));	//визначення іконки
	//wcex.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS));
	wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOW); //установка фону
	wcex.lpszMenuName = NULL; 				//визначення меню
	wcex.lpszClassName = szWindowClass; 		//ім’я класу
	wcex.hIconSm = LoadIcon(NULL, MAKEINTRESOURCE(IDI_ICON1));
	

	return RegisterClassEx(&wcex); 			//реєстрація класу вікна
}

// FUNCTION: InitInstance (HANDLE, int)
// Створює вікно програми і зберігає дескриптор програми в змінній hInst

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; //зберігає дескриптор додатка в змінній hInst
	HWND hWnd = CreateWindow(szWindowClass, 	// ім’я класу вікна
		szTitle, 				// назва програми
		WS_OVERLAPPEDWINDOW,			// стиль вікна
		CW_USEDEFAULT, 			// положення по Х	
		CW_USEDEFAULT,			// положення по Y	
		CW_USEDEFAULT, 			// розмір по Х
		CW_USEDEFAULT, 			// розмір по Y
		NULL, 					// дескриптор батьківського вікна	
		NULL, 					// дескриптор меню вікна
		hInstance, 				// дескриптор програми
		NULL); 				// параметри створення.

	if (!hWnd) 	//Якщо вікно не творилось, функція повертає FALSE
	{
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow); 		//Показати вікно
	UpdateWindow(hWnd); 				//Оновити вікно
	return TRUE;
}

// FUNCTION: WndProc (HWND, unsigned, WORD, LONG)
// Віконна процедура. Приймає і обробляє всі повідомлення, що приходять в додаток


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DialogBox(hInst, MAKEINTRESOURCE(101), NULL, DialogProc);
}
INT_PTR CALLBACK ResultsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) 
	{
		case WM_INITDIALOG:
		{
			vector<wstring> results = LoadResultsFromFile();
			wstring resultText;
			for (const wstring& line : results) {
				resultText += line + L"\r\n";
			}
			SetDlgItemText(hDlg, 1093, resultText.c_str());
			return TRUE;
			
		}
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK )
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;	

		}

	}
	return FALSE;

}

INT_PTR CALLBACK DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

	case WM_INITDIALOG:
	{
		HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(104));

		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)));

		if (hMenu)
		{
			SetMenu(hwnd, hMenu);
		}

		InitializeEditBoxes(hwnd);
		ApplyCustomFontToAllEditBoxes(hFont, cells);
		startNewGame();
		return TRUE;
	}

	case WM_SETCURSOR:
	{
		SetCursor((HCURSOR)LoadCursor(hInst, MAKEINTRESOURCE(IDC_CURSOR2)));
		return TRUE;
	}
	case WM_COMMAND:
	{
		int Id = LOWORD(wParam);

		if (Id >= 1001 && Id <= 1033)
		{
			wchar_t buttonText[2];
			GetWindowText(GetDlgItem(hwnd, Id), buttonText, 2);

			if (currentRow  < 6 && currentCell < 5)
			{
				SetWindowText(cells[currentRow][currentCell], buttonText);

				currentCell++;
				if (currentCell == 5) 
				{
					if (Id == 1035) {
						currentCell = 0;
						currentRow++;

						if (currentRow < 6) {
							SetFocus(cells[currentRow][currentCell]);
						}
					}
				}
			}
		}
		else if (Id == 1034)
		{
			if (currentCell > 0)
			{
				currentCell--;
				SetWindowText(cells[currentRow][currentCell], L"");
				SetFocus(cells[currentRow][currentCell]);

			}
		}


		else if (Id == 1035)
		{
			bool allCellsCorrect = true;
			wchar_t enteredWord[5] = { 0 };

			for (int col = 0; col < 5; col++)
			{
				wchar_t cellText[2] = { 0 };

				GetWindowText(cells[currentRow][col], cellText, 2);

				enteredWord[col] = cellText[0];
				if (wcslen(cellText) == 0)
				{
					allCellsFilled = false;
					MessageBox(hwnd, L"Занадто коротко", L"Коротко!", MB_OK);
					break;
				}
				else 

					allCellsFilled = true;

			}

			if (allCellsFilled)
			{

				wstring entWrd;
				entWrd.assign(enteredWord, enteredWord + sizeof(enteredWord) / sizeof(wchar_t));
				EnteredWord.push_back(entWrd);
				for (int i = 0; i < 6; i++) 
				{
					for (int j = 0; j < 5; j++) 
					{

						if (entWrd[j] != SecretWord[j]) 
						{
							mistakesCount++;
							allCellsCorrect = false;
							break;
							
						}


					}


				}
				enteredWordCount++;

				if (enteredWordCount==6 && mistakesCount >=1 )
				{
					InvalidateRect(hwnd, NULL, TRUE);
					MessageBox(hwnd, L"Ви програли:(", L"Програш:(", MB_OK);
					WriteResultToFile(SecretWord, enteredWordCount, false);
					RestartGame();
					SetFocus(cells[0][0]);
					return TRUE;
				}
				if (allCellsCorrect)
				{
					InvalidateRect(hwnd, NULL, TRUE);
					MessageBox(hwnd, L"Вітаю! Ви перемогли!", L"Перемога!", MB_OK);
					WriteResultToFile(SecretWord,enteredWordCount, true);
					RestartGame();
					SetFocus(cells[0][0]);
					return TRUE;

				}
				
				InvalidateRect(hwnd, NULL, TRUE);

				currentRow++;
				currentCell = 0;
				if (currentRow < 6)
				{
					SetFocus(cells[currentRow][currentCell]);
				}
			}

		}
		else if (Id == 40001)
		{
			MessageBox(hwnd, L"КІУКІ-22-7, Довбня К.О.", L"О нас", MB_OK);
		}
		else if (Id == 40002)
		{
			MessageBox(hwnd,
				L"Задача гри максимально проста,потрібно вгадати загадане слово. У користувача є шість спроб для того щоб вгадати слово, після шести спроб гра буде завершена.\nВ грі існує три кольори підсвітки літер,ось пояснення до кожного кольору:\nЗелений - літера вгадана правильно і стоїть в правильній позиції\nПомаранчевий - літера вгадана правильна, але стоїть не в правильній позиції\nСірий - літера взагалі не вгадана.", L"Інструкція користувача", 
				MB_OK);
		}
		else if (Id == 40003)
		{
			DialogBox(hInst, MAKEINTRESOURCE(105), hwnd, ResultsDialogProc);

		}

		
		break;
	}

	case WM_CTLCOLORSTATIC:
{
	HDC hdc = (HDC)wParam;
	HWND hwndCtl = (HWND)lParam;

	for (int i = 0; i < 6; i++)
	{
		if (i >= EnteredWord.size())
			continue;
		wstring myEntWrd = EnteredWord.at(i);
		for (int j = 0; j < 5; j++)
		{
			if (cells[i][j] == hwndCtl)
			{
				if (myEntWrd.at(j) == SecretWord.at(j))
				{
					color = RGB(0, 255, 0); //зелений
				}
				else
				{
					auto pos = SecretWord.find(myEntWrd.at(j));
					if (pos != string::npos)
					{
						color = RGB(255, 165, 0);//помаранчевий
					}
					else
						color = RGB(128, 128, 128);//сірий
				}
					

				return (LRESULT)CreateSolidBrush(color);

			}

		}

	}
	break;
	}
	
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	}
	return FALSE;
}