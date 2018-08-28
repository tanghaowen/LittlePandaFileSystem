#pragma once
#include <windows.h>
TCHAR* char2Tchar(char* char_string) {
	TCHAR * tmp_path;
	int lenth_of_charstring = strlen(char_string) + sizeof(char);

	tmp_path = (TCHAR*)malloc(sizeof(TCHAR) * lenth_of_charstring);
	MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, char_string, lenth_of_charstring, tmp_path, sizeof(TCHAR) * lenth_of_charstring);
	return tmp_path;
}

char* tchar2Char(TCHAR* char_string) {
	char * tmp_path;
	int lenth_of_charstring = lstrlen(char_string) + 1;
	tmp_path = (char*)malloc(sizeof(char) * lenth_of_charstring);
	wcstombs(tmp_path, char_string, lenth_of_charstring);
	return tmp_path;
}



