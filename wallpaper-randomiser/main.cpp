#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_int.hpp>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <windows.h>
#include <ctime>
#include <dirent.h>
#include <sstream>
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' \
                      version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' \
                        language='*'\"")
#include <commctrl.h>
#include "resource.h"
#include <ShlObj.h>

//CONTROL DEFINES

typedef std::vector<std::string> fileList;
fileList files;

typedef boost::random::mt19937 Generator;
Generator gen((unsigned)time(0));

//OPTIONS VARIABLES
std::string directory;

// Forward Declarations
bool getFolder(std::string& folderpath, const char* szCaption = NULL, HWND hOwner = NULL);
void setDirectoryToFile();

// Scan directory and push all files into a global vector
void get_files_in_dir(const std::string& directory)
{
	if (files.size() > 1)
	{
		files.erase(files.begin(), files.end());
	}
	DIR* dir;
	struct dirent* ent;
	dir = opendir(directory.c_str());
	if (dir != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			if (ent->d_namlen < 3)
				continue;
			std::stringstream full_directory_file_name;
			full_directory_file_name << directory << "\\" << ent->d_name;
			files.push_back(full_directory_file_name.str());
		}
		closedir(dir);
	}
}

//Get a random file from the vector
std::string get_random_file()
{
	boost::random::uniform_int_distribution<> range(0, files.size() - 1);
	return files[range(gen)];
}

bool has_traversed_files(std::ifstream& file)
{
	// Find the length of the file
	size_t length = 0;
	std::string* null_buffer = new std::string;
	while (!file.eof())
	{
		//get each line of the file and store it in an unused string buffer
		std::getline(file, *null_buffer); 
		//increment the length each iteration of the file
		length++;
	}
	// Free the memory of the string
	delete null_buffer;
	// The length must be decremented as the last line of the file is blank
	length--;
	//If the length of the file equals the length of the vector return true
	if (length == files.size())
		return true;
	else
		return false;
}

// Check if the file has been generated before
bool has_file_gen(std::ifstream& file, std::string ltc)
{
	// Declare a vector to hold all of the file's lines
	std::vector<std::string> previous_files;

	std::string buffer;
	// Set the stream back to the beginning of the file
	file.clear();
	file.seekg(0, std::ios::beg);
	// Get each line of the file and push it into a vector
	while (file.eof() == false)
	{
		std::getline(file, buffer);
		previous_files.push_back(buffer);
	}
	// Pop the last line back, it's blank
	previous_files.pop_back();

	for (UINT i = 0; i < previous_files.size(); ++i)
	{
		if (previous_files[i] == ltc)
		{
			return true;
		}
	}
	return false;
}

void oldMain()
{	
	if (directory.length() < 3)
		return;
	get_files_in_dir(directory);
	// Open the txt file containing all the previously generated files
	std::ifstream previous_files("previous_files.txt");
	// If the file doesn't exist, create it
	if (previous_files.fail() == true)
	{
		previous_files.close();
		std::ofstream create("previous_files.txt");
		previous_files.open("previous_files.txt");
		create.close();
	}

	/*
	This requires some explanation:

	if the text file contains as many lines as the vector when we check
	if the file generated has already been generated before, it will
	always return true, causing the app to hang.

	Therefore we check if the file has as many lines as the vector.
	If it does, all the files in the folder have been generated at least once
	and we open the file normally and this will clear the file.
	Else we open the file in append mode and will just write to the
	bottom of the file
	*/
	std::ofstream generated_file;
	if (has_traversed_files(previous_files) == true)
	{
		 generated_file.open("previous_files.txt" );
	}
	else
	{
		generated_file.open("previous_files.txt", std::ios_base::app );
	}
	
	//Generate a random file, check if it has already been generated, if not write it to the file
	std::string random_file = get_random_file();
	
	while (has_file_gen(previous_files, random_file) == true)
	{
		random_file = get_random_file();
	}
	generated_file << random_file << std::endl;
	
	std::cout << random_file << std::endl;

	// Finally set the wallpaper to the random file generated
	SystemParametersInfoA( SPI_SETDESKWALLPAPER, 0, (PVOID)random_file.c_str(), SPIF_UPDATEINIFILE );

	MessageBox(NULL, random_file.c_str(), "Wallpaper Set To" , MB_OK | MB_TASKMODAL);
}

BOOL CALLBACK AboutDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG: return TRUE;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDOK:
				EndDialog(hWnd, IDOK); break;

			case IDCANCEL:
				EndDialog(hWnd, IDCANCEL); break;
			}
			break;
		}
	default: return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
    {

	case WM_CREATE:
		{
			HWND button = CreateWindowEx(NULL, "BUTTON", "...",
					WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 300, 75, 50, 20,
					hWnd, (HMENU)IDC_BROWSE_BUTTON, NULL, NULL);

			HWND directoryControl = CreateWindow("EDIT", directory.c_str(), 
					WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY |
					ES_AUTOVSCROLL, 30, 75, 260, 20, hWnd, (HMENU)IDC_BROWSE_EDIT,
					NULL, NULL);

			HWND grpBox = CreateWindow("BUTTON", NULL, WS_VISIBLE | 
					WS_CHILD | BS_GROUPBOX, 5, 5, 370, 220, 
					hWnd, NULL, NULL, NULL);

			HWND staticText = CreateWindow("Static", "Select Wallpaper Directory: ", 
					WS_CHILD | WS_VISIBLE | SS_LEFT, 30, 50, 300, 20, 
					hWnd, NULL, NULL, NULL);

			HWND generateButton = CreateWindow("Button", "Generate Wallpaper", 
					WS_VISIBLE | WS_CHILD, 30, 150, 320, 50, 
					hWnd, (HMENU)IDC_GENERATE, NULL, NULL);
		}
		break;

	case WM_COMMAND:
		{
			if (HIWORD(wParam) == BN_CLICKED)
			{
				switch (LOWORD(wParam))
				{
				case IDC_BROWSE_BUTTON:
					{
						getFolder(directory, "Select Wallpaper Directory", hWnd);
						SetDlgItemText(hWnd, IDC_BROWSE_EDIT, directory.c_str());
						break;
					}
				case IDC_GENERATE:
					{
						if (directory.size() < 3)
						{
							MessageBox(hWnd, "Please Select a Directory", "Directory Not Set", MB_OK | MB_ICONEXCLAMATION);
							break;
						}
						oldMain();
						break;
					}
				}
			}

			switch(LOWORD(wParam))
			{

			case ID_FILE_EXIT:
				PostMessage(hWnd, WM_CLOSE, 0, 0);
				break;
			case ID_FILE_ABOUT:
				{
					int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), hWnd, AboutDlgProc);
					break;
				}

			case ID_STUFF_SOMESTUFF:
				oldMain();
				break;
			}

			break;
		}

    case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

    case WM_DESTROY:
		setDirectoryToFile();
		PostQuitMessage(0);
		break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);

    }
    return 0;
}

bool getFolder(std::string& folderpath, 
               const char* szCaption, 
               HWND hOwner)
{
   bool retVal = false;

   // The BROWSEINFO struct tells the shell 
   // how it should display the dialog.
   BROWSEINFO bi;
   memset(&bi, 0, sizeof(bi));

   bi.ulFlags   = BIF_USENEWUI;
   bi.hwndOwner = hOwner;
   bi.lpszTitle = szCaption;

   // must call this if using BIF_USENEWUI
   ::OleInitialize(NULL);

   // Show the dialog and get the itemIDList for the 
   // selected folder.
   LPITEMIDLIST pIDL = ::SHBrowseForFolder(&bi);

   if(pIDL != NULL)
   {
      // Create a buffer to store the path, then 
      // get the path.
      char buffer[_MAX_PATH] = {'\0'};
      if(::SHGetPathFromIDList(pIDL, buffer) != 0)
      {
         // Set the string value.
         folderpath = buffer;
         retVal = true;
      }

      // free the item id list
      CoTaskMemFree(pIDL);
   }

   ::OleUninitialize();

   return retVal;
}

std::string getDirectoryFromFile()
{
	std::string directoryFromFile;
	std::ifstream stream("directory.txt");
	if (stream.fail())
	{
		return directoryFromFile;
	}
	std::getline(stream, directoryFromFile);
	return directoryFromFile;
}

void setDirectoryToFile()
{
	std::ofstream output("directory.txt");
	output << directory;
}

// WinMain -- entry point for windows applications
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
    HWND hWnd;
    MSG Msg;

	directory = getDirectoryFromFile();
	if (directory.size() > 3)
		get_files_in_dir(directory);

	const char wndClassName[] = "Wallpaper Randomiser";

	// Resgister Window
	wc.cbSize		= sizeof(WNDCLASSEX);
	wc.style		= 0;
	wc.lpfnWndProc	= WndProc;
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	= hInstance;
	wc.hIcon		= LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground= (HBRUSH)(COLOR_WINDOW);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wc.lpszClassName= wndClassName;
	wc.hIconSm      = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

	INITCOMMONCONTROLSEX icx;
	icx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icx.dwICC = ICC_BAR_CLASSES;
	InitCommonControlsEx(&icx);

	// Create Window
	hWnd = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			wndClassName,
			"Wallpaper Randomiser",
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			CW_USEDEFAULT, CW_USEDEFAULT,
			400, 300, NULL, 
			NULL, hInstance, NULL );

	if (hWnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}