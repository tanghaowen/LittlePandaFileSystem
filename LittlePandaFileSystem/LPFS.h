#pragma once
#include <Windows.h>
#include <vector>
#include <mysql.h>
#include <stdlib.h>
#include "tools.h"

struct LPFS_INDEX
{
	TCHAR * fileName;
	long offset;
	long size;

};
struct SQL_IMAGE
{
	int imageId;
	TCHAR * path;
	int size;
	long offset;
	int libfileId = -1;
	TCHAR *libfile_path;
	TCHAR *libfile_name;
	int libfile_size;

};
void SaveIndexToDatabase(std::vector <SQL_IMAGE> &images, MYSQL* sql_connect,int startPos);

void FiltoutFilenameFromFullHashPath(TCHAR* fullhashpath) {

}


void SaveIndexStructToFile(std::vector<LPFS_INDEX> indexs) {
	FILE *H_lib_index;
	H_lib_index = fopen("datalib.lpfs.idx", "wb");
	int files_number = indexs.size();


	fwrite(&files_number, sizeof(files_number), 1, H_lib_index);
	for (auto index : indexs)
	{

		int fileName_size;
		// 写入offset
		fwrite(&index.offset, sizeof(index.offset), 1, H_lib_index);
		// 写入文件的大小
		fwrite(&index.size, sizeof(index.size), 1, H_lib_index);
		// 计算并写入文件名字符串的长度
		fileName_size = lstrlen(index.fileName) + sizeof(TCHAR);
		fwrite(&fileName_size, sizeof(int), 1, H_lib_index);
		// 写入文件名字符串
		fwrite(index.fileName, sizeof(TCHAR), fileName_size, H_lib_index);
	}
	fclose(H_lib_index);

}
void ReadIndexFile(std::vector<LPFS_INDEX> &indexs) {
	FILE *H_lib_index;
	H_lib_index = fopen("datalib.lpfs.idx", "rb");
	int files_number;

	fseek(H_lib_index, 0, SEEK_SET);
	fread(&files_number, sizeof(files_number), 1, H_lib_index);
	int file_name_length;
	LPFS_INDEX index;
	TCHAR filename[MAX_PATH];
	for (size_t i = 0; i < files_number; i++)
	{

		fread(&index.offset, sizeof(index.offset), 1, H_lib_index);
		fread(&index.size, sizeof(index.size), 1, H_lib_index);
		fread(&file_name_length, sizeof(file_name_length), 1, H_lib_index);

		fread(filename, sizeof(TCHAR), file_name_length, H_lib_index);
		index.fileName = filename;
		indexs.push_back(index);


	}

	fclose(H_lib_index);

}

void SaveFilesToLibFile(std::vector<LPFS_INDEX> & indexs, std::vector <TCHAR *> files) {
	printf("Now saving images to lib...");
	FILE* Htmpfile, *H_lib_file;
	H_lib_file = fopen("datalib.lpfs", "wb");
	fseek(H_lib_file, 0, SEEK_SET);


	int count = 0;
	for (auto image_path : files) {


		long image_file_size;
		LPFS_INDEX index;

		Htmpfile = _wfopen(image_path, L"rb");
		fseek(Htmpfile, 0, SEEK_END);
		image_file_size = ftell(Htmpfile);
		fseek(Htmpfile, 0, SEEK_SET);

		short * file_buffer = (short*)malloc(image_file_size);
		fread(file_buffer, image_file_size, 1, Htmpfile);
		fclose(Htmpfile);


		long offset = ftell(H_lib_file);


		fwrite(file_buffer, image_file_size, 1, H_lib_file);
		free(file_buffer);
		index.fileName = image_path;
		index.offset = offset;
		index.size = image_file_size;
		indexs.push_back(index);



	}
	fclose(H_lib_file);
	printf("Saving over!");
}
void SaveFilesToLibFile(std::vector <SQL_IMAGE>* files, TCHAR * fileDir, int single_lib_file_max_size_MB, 
	MYSQL* sql_connect, int startFilePost , TCHAR* LPFS_LIB_PATH) {
	printf("Now saving images to lib...");
	FILE* Htmpfile, *H_lib_file;
	MYSQL_RES* query_res;
	MYSQL_ROW mysql_row;
	char *libfilename, tmp_libfilename[500], query_string[1000];
	int lib_file_seek_offset;
	int lib_file_id;
	TCHAR libfilefullpath[MAX_PATH];
	int newLibFile = false;

	if (mysql_query(sql_connect, "select * from lpfs_libfiles order by libfile_id DESC limit 1;"))
	{
		printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
	}
	query_res = mysql_store_result(sql_connect);

	mysql_row = mysql_fetch_row(query_res);
	if (mysql_row == NULL)
	{
		libfilename = "datalib1.lpfs";
		lib_file_seek_offset = 0;
		lib_file_id = 1;

		/*
		sprintf(query_string, "insert into lpfs_libfiles (libfile_path,libfile_name,libfile_size) values( '%s' , '%s',0 )", tchar2Char(LPFS_LIB_PATH), libfilename);
		if (mysql_query(sql_connect, query_string))
		{
			printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
		}
		lib_file_id = mysql_insert_id(sql_connect);*/

		newLibFile = true;


	}
	else if (atoi(mysql_row[3]) < single_lib_file_max_size_MB * 1024 * 1024) {
		libfilename = mysql_row[2];
		lib_file_seek_offset = atoi(mysql_row[3]);
		lib_file_id = atoi(mysql_row[0]);
	}
	else {
		sprintf(tmp_libfilename, "datalib%d.lpfs", atoi(mysql_row[0]) + 1);
		libfilename = tmp_libfilename;
		lib_file_seek_offset = 0;
		lib_file_id = atoi(mysql_row[0])+1;

		/*
		sprintf(query_string, "insert into lpfs_libfiles (libfile_path,libfile_name,libfile_size) values( '%s' , '%s',0 )", tchar2Char(LPFS_LIB_PATH), libfilename);
		if (mysql_query(sql_connect, query_string))
		{
			printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
		}
		lib_file_id = mysql_insert_id(sql_connect);*/
		newLibFile = true;

	}


	
	wsprintf(libfilefullpath, L"%s//%s", LPFS_LIB_PATH, char2Tchar(libfilename));
	if (newLibFile) {
		H_lib_file = _wfopen(libfilefullpath, L"wb");
	}
	else {
		H_lib_file = _wfopen(libfilefullpath, L"ab");
	}

	fseek(H_lib_file, lib_file_seek_offset, SEEK_SET);
	int count = 0;
	TCHAR fileFullPath[MAX_PATH];


	for (size_t i = startFilePost; i < files->size(); i++) {

		wsprintf(fileFullPath, L"%s//%s", fileDir, (*files)[i].path);


		Htmpfile = _wfopen(fileFullPath, L"rb");


		short * file_buffer = (short*)malloc((*files)[i].size);
		fread(file_buffer, (*files)[i].size, 1, Htmpfile);
		fclose(Htmpfile);


		long offset = ftell(H_lib_file);


		fwrite(file_buffer, (*files)[i].size, 1, H_lib_file);
		free(file_buffer);
		(*files)[i].offset = offset;
		wprintf(L"\nimage_id:%d. %d/%d  : %s  -   %d - %d" , (*files)[i].imageId,i, files->size(), (*files)[i].path, (*files)[i].size, ftell(H_lib_file));
		(*files)[i].libfileId = lib_file_id;
		
		
		if (ftell(H_lib_file) > single_lib_file_max_size_MB * 1024 * 1024) {
			sprintf(query_string, "update  lpfs_libfiles set libfile_size=%d where libfile_id = %d", ftell(H_lib_file), lib_file_id);
			if (mysql_query(sql_connect, query_string))
			{
				printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
			}
			if (newLibFile) {

				sprintf(query_string, "insert into lpfs_libfiles (libfile_id,libfile_path,libfile_name,libfile_size) values(%d, '%s' , '%s',%d )"
					, lib_file_id, tchar2Char(LPFS_LIB_PATH), libfilename, ftell(H_lib_file));
				if (mysql_query(sql_connect, query_string))
				{
					printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
				}
				lib_file_id = mysql_insert_id(sql_connect);
			}
			
			fclose(H_lib_file);
			H_lib_file = NULL;
			SaveIndexToDatabase(*files, sql_connect, startFilePost);
			SaveFilesToLibFile(files, fileDir, single_lib_file_max_size_MB, sql_connect, i + 1,LPFS_LIB_PATH);
			break;
		}



	}


	if (H_lib_file != NULL) {
		if (newLibFile) {
			sprintf(query_string, "insert into lpfs_libfiles (libfile_id,libfile_path,libfile_name,libfile_size) values(%d, '%s' , '%s',%d )"
				, lib_file_id, tchar2Char(LPFS_LIB_PATH), libfilename, ftell(H_lib_file));
			if (mysql_query(sql_connect, query_string))
			{
				printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
			}
			lib_file_id = mysql_insert_id(sql_connect);
		}else{
			sprintf(query_string, "update  lpfs_libfiles set libfile_size=%d where libfile_id = %d", ftell(H_lib_file), lib_file_id);
			if (mysql_query(sql_connect, query_string))
			{
				printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
			}
		}
		fclose(H_lib_file);



		SaveIndexToDatabase(*files, sql_connect, startFilePost);
	}


	printf("\nSaving over!");
}
void SaveFilesToLibFileMemoryTmpMode(std::vector <SQL_IMAGE>* files, TCHAR * fileDir, int single_lib_file_max_size_MB,

	MYSQL* sql_connect, int startFilePost, TCHAR* LPFS_LIB_PATH) {
	printf("Now saving images to lib...");
	FILE* Htmpfile, *H_lib_file;
	MYSQL_RES* query_res;
	MYSQL_ROW mysql_row;
	char *libfilename, tmp_libfilename[500], query_string[1000];
	int lib_file_seek_offset , lib_file_start_seek_offset;
	int lib_file_id;
	TCHAR libfilefullpath[MAX_PATH];
	int newLibFile = false;

	if (mysql_query(sql_connect, "select * from lpfs_libfiles order by libfile_id DESC limit 1;"))
	{
		printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
	}
	query_res = mysql_store_result(sql_connect);

	mysql_row = mysql_fetch_row(query_res);
	if (mysql_row == NULL)
	{
		libfilename = "datalib1.lpfs";
		lib_file_seek_offset = 0;
		lib_file_id = 1;

		/*
		sprintf(query_string, "insert into lpfs_libfiles (libfile_path,libfile_name,libfile_size) values( '%s' , '%s',0 )", tchar2Char(LPFS_LIB_PATH), libfilename);
		if (mysql_query(sql_connect, query_string))
		{
		printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
		}
		lib_file_id = mysql_insert_id(sql_connect);*/

		newLibFile = true;


	}
	else if (atoi(mysql_row[3]) < single_lib_file_max_size_MB * 1024 * 1024) {
		libfilename = mysql_row[2];
		lib_file_seek_offset = atoi(mysql_row[3]);
		lib_file_id = atoi(mysql_row[0]);
	}
	else {
		sprintf(tmp_libfilename, "datalib%d.lpfs", atoi(mysql_row[0]) + 1);
		libfilename = tmp_libfilename;
		lib_file_seek_offset = 0;
		lib_file_id = atoi(mysql_row[0]) + 1;

		/*
		sprintf(query_string, "insert into lpfs_libfiles (libfile_path,libfile_name,libfile_size) values( '%s' , '%s',0 )", tchar2Char(LPFS_LIB_PATH), libfilename);
		if (mysql_query(sql_connect, query_string))
		{
		printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
		}
		lib_file_id = mysql_insert_id(sql_connect);*/
		newLibFile = true;

	}


	
	INT64 blocksz = ((INT64)single_lib_file_max_size_MB + 200) * 1024 * 1024;

	INT8 * lib_file_buffer = (INT8*)malloc(blocksz);

	lib_file_start_seek_offset = lib_file_seek_offset;

	wsprintf(libfilefullpath, L"%s//%s", LPFS_LIB_PATH, char2Tchar(libfilename));
	if (newLibFile) {
		H_lib_file = _wfopen(libfilefullpath, L"wb");
	}
	else {
		H_lib_file = _wfopen(libfilefullpath, L"ab");
	}

	fseek(H_lib_file, lib_file_seek_offset, SEEK_SET);
	int count = 0;
	TCHAR fileFullPath[MAX_PATH];


	for (size_t i = startFilePost; i < files->size(); i++) {

		wsprintf(fileFullPath, L"%s//%s", fileDir, (*files)[i].path);


		Htmpfile = _wfopen(fileFullPath, L"rb");

		fseek(Htmpfile, 0, SEEK_END);
		int endpos = ftell(Htmpfile);
		fseek(Htmpfile, 0, SEEK_SET);
		printf("Write into memory!");
		fread(lib_file_buffer+lib_file_seek_offset, (*files)[i].size, 1, Htmpfile);
		(*files)[i].offset = lib_file_seek_offset;
		lib_file_seek_offset += (*files)[i].size;
		fclose(Htmpfile);
		
		wprintf(L"\nimage_id:%d. %d/%d  : %s  -   %d - %d", (*files)[i].imageId, i, files->size(), (*files)[i].path, (*files)[i].size, lib_file_seek_offset);
		(*files)[i].libfileId = lib_file_id;


		if (lib_file_seek_offset > single_lib_file_max_size_MB * 1024 * 1024) {
			sprintf(query_string, "update  lpfs_libfiles set libfile_size=%d where libfile_id = %d", lib_file_seek_offset, lib_file_id);
			if (mysql_query(sql_connect, query_string))
			{
				printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
			}
			if (newLibFile) {

				sprintf(query_string, "insert into lpfs_libfiles (libfile_id,libfile_path,libfile_name,libfile_size) values(%d, '%s' , '%s',%d )"
					, lib_file_id, tchar2Char(LPFS_LIB_PATH), libfilename, lib_file_seek_offset);
				if (mysql_query(sql_connect, query_string))
				{
					printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
				}
				lib_file_id = mysql_insert_id(sql_connect);
			}
			fwrite(&lib_file_buffer[lib_file_start_seek_offset], lib_file_seek_offset- lib_file_start_seek_offset, 1, H_lib_file);
			free(lib_file_buffer);
			fclose(H_lib_file);
			H_lib_file = NULL;
			SaveIndexToDatabase(*files, sql_connect, startFilePost);
			SaveFilesToLibFileMemoryTmpMode(files, fileDir, single_lib_file_max_size_MB, sql_connect, i + 1, LPFS_LIB_PATH);
			break;
		}



	}


	if (H_lib_file != NULL) {
		if (newLibFile) {
			sprintf(query_string, "insert into lpfs_libfiles (libfile_id,libfile_path,libfile_name,libfile_size) values(%d, '%s' , '%s',%d )"
				, lib_file_id, tchar2Char(LPFS_LIB_PATH), libfilename, lib_file_seek_offset);
			if (mysql_query(sql_connect, query_string))
			{
				printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
			}
			lib_file_id = mysql_insert_id(sql_connect);
		}
		else {
			sprintf(query_string, "update  lpfs_libfiles set libfile_size=%d where libfile_id = %d", lib_file_seek_offset, lib_file_id);
			if (mysql_query(sql_connect, query_string))
			{
				printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
			}
		}
		fwrite(&lib_file_buffer[lib_file_start_seek_offset], lib_file_seek_offset - lib_file_start_seek_offset, 1, H_lib_file);
		free(lib_file_buffer);
		fclose(H_lib_file);



		SaveIndexToDatabase(*files, sql_connect, startFilePost);
	}


	printf("\nSaving over!");
}
void ExacFilesFromLibfileToDir(std::vector<SQL_IMAGE> images , TCHAR* exactPath) {
	std::vector<int> opened_libfiles_id;
	std::vector<FILE*> opened_libfiles_point;


	FILE * tmpfile , *saved_file;
	TCHAR save_file_name[MAX_PATH], open_lib_file_fullpath[MAX_PATH];
	for (auto image : images) {
		std::vector<int>::iterator itr = std::find(opened_libfiles_id.begin(), opened_libfiles_id.end(), image.libfileId);

		if (itr == opened_libfiles_id.end()) {
			// 图片的libfile_id还没打开过
			// 打开libfile
			wsprintf(open_lib_file_fullpath, L"%s\\%s", image.libfile_path, image.libfile_name);
			tmpfile = _wfopen(open_lib_file_fullpath, L"rb");
			opened_libfiles_point.push_back(tmpfile);
			opened_libfiles_id.push_back(image.libfileId);
		}
		else {
			int index = itr - opened_libfiles_id.begin();
			tmpfile = opened_libfiles_point[index];
		}

		printf("\nNow exacting image id: %d", image.imageId);
		wsprintf(save_file_name, L"%s//%d.jpg",exactPath ,image.imageId);
		saved_file = _wfopen(save_file_name, L"wb");
		fseek(saved_file, 0, SEEK_SET);
		fseek(tmpfile, image.offset, SEEK_SET);
		short * file_buffer = (short*)malloc(image.size);
		fread(file_buffer, image.size, 1, tmpfile);
		fwrite(file_buffer, image.size, 1, saved_file);
		free(file_buffer);
		fclose(saved_file);
	}


	for (auto file_point : opened_libfiles_point)
		fclose(file_point);


}


/*传入数据库中的图片的id(vector<SQL_IMAGE>)
修改传入的vector<SQL_IMAGE> *images，包含着读出的图片的名字，路径，libfile和offset信息*/
void ReadFilesIndexFromDatabase(std::vector<SQL_IMAGE> *images, std::vector<int> images_ids, MYSQL* sql_connect) {
	MYSQL_RES* query_res;
	MYSQL_ROW mysql_row;
	char query_string[500];
	int count = 0;
	TCHAR *tmp_path;
	for (auto image_id : images_ids)
	{
		SQL_IMAGE image;
		sprintf(query_string, "select images.image_id,images.hash,images.path,images.size,lpfs_idx.libfile_id,lpfs_idx.`offset`,lpfs_libfiles.libfile_name,lpfs_libfiles.libfile_path,lpfs_libfiles.libfile_size from images,lpfs_idx,lpfs_libfiles where images.image_id = %d and images.image_id = lpfs_idx.image_id and lpfs_idx.libfile_id = lpfs_libfiles.libfile_id;", image_id);
		if (mysql_query(sql_connect, query_string))
		{
			printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
		}
		query_res = mysql_store_result(sql_connect);

		mysql_row = mysql_fetch_row(query_res);

		image.imageId = image_id;
		image.libfileId = atoi(mysql_row[4]);
		image.offset = atoi(mysql_row[5]);
		image.path = char2Tchar(mysql_row[2]);
		image.size = atoi(mysql_row[3]);
		image.libfile_name = char2Tchar(mysql_row[6]);
		image.libfile_path = char2Tchar(mysql_row[7]);
		image.libfile_size = atoi(mysql_row[8]);


		(*images).push_back(image);



		count++;
	}


}

void ReadFilesIndexFromDatabase(std::vector<SQL_IMAGE> *images, MYSQL* sql_connect) {
	MYSQL_RES* query_res;
	MYSQL_ROW mysql_row;
	char query_string[500];
	int count = 0;
	TCHAR *tmp_path;
	for (auto &image : *images)
	{

		sprintf(query_string, "select images.image_id,images.hash,images.path,images.size,lpfs_idx.libfile_id,lpfs_idx.`offset`,lpfs_libfiles.libfile_name,lpfs_libfiles.libfile_path,lpfs_libfiles.libfile_size from images,lpfs_idx,lpfs_libfiles where images.image_id = %d and images.image_id = lpfs_idx.image_id and lpfs_idx.libfile_id = lpfs_libfiles.libfile_id;"
			, image.imageId);
		if (mysql_query(sql_connect, query_string))
		{
			printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
		}
		query_res = mysql_store_result(sql_connect);

		mysql_row = mysql_fetch_row(query_res);

		image.libfileId = atoi(mysql_row[4]);
		image.offset = atoi(mysql_row[5]);
		image.path = char2Tchar(mysql_row[2]);
		image.size = atoi(mysql_row[3]);
		image.libfile_name = char2Tchar(mysql_row[6]);
		image.libfile_path = char2Tchar(mysql_row[7]);
		image.libfile_size = atoi(mysql_row[8]);




		count++;
	}


}


void SaveIndexToDatabase(std::vector <SQL_IMAGE> &images , MYSQL* sql_connect) {

	char  query[100];
	for (auto image : images)
	{
		sprintf(query, "REPLACE INTO lpfs_idx ( image_id , libfile_id , offset) values ( %d , %d , %d);", image.imageId, image.libfileId, image.offset);
		if (mysql_query(sql_connect, query))
		{
			printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
		}
	}
}

void SaveIndexToDatabase(std::vector <SQL_IMAGE> &images, MYSQL* sql_connect, int startPos) {

	char  query[100];
	for (size_t i = startPos; i < images.size(); i++) {
		if (images[i].libfileId == -1) break;
		sprintf(query, "REPLACE INTO lpfs_idx ( image_id , libfile_id , offset) values ( %d , %d , %d);"
			, images[i].imageId, images[i].libfileId, images[i].offset);
		if (mysql_query(sql_connect, query))
		{
			printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
		}
	}

}
void SaveIndexToDatabase2(std::vector <SQL_IMAGE> &images, MYSQL* sql_connect, int startPos) {

	char  query[100];
	char * queries = (char*)malloc(500 * images.size());
	for (size_t i = startPos; i < images.size(); i++) {
		if (images[i].libfileId == -1) break;
		sprintf(query, "REPLACE INTO lpfs_idx ( image_id , libfile_id , offset) values ( %d , %d , %d);"
			, images[i].imageId, images[i].libfileId, images[i].offset);
		if (mysql_query(sql_connect, query))
		{
			printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
		}
	}

	free(queries);
}



void getAllNotAddedImagesInfoFromDb(MYSQL* sql_connect, std::vector <SQL_IMAGE> &images) {

	MYSQL_RES* query_res;
	MYSQL_ROW  mysql_row;

	char query_string[] = "select * from images where images.image_id not in (select images.image_id from images,lpfs_idx IGNORE INDEX(PRIMARY) where images.image_id = lpfs_idx.image_id)";
	if (mysql_query(sql_connect, query_string))
	{
		printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
	}
	query_res = mysql_store_result(sql_connect);
	while ((mysql_row = mysql_fetch_row(query_res))) {
		SQL_IMAGE image;
		image.imageId = atoi(mysql_row[0]);
		image.path = char2Tchar(mysql_row[2]);
		image.size = atoi(mysql_row[5]);
		images.push_back(image);
	}
}

void getImagesInfoFromDbWithRange(int start, int end, MYSQL* sql_connect, std::vector <SQL_IMAGE> &images) {

	MYSQL_RES* query_res;
	MYSQL_ROW  mysql_row;

	char query_string[500];
	sprintf(query_string, "select * from images limit %d , %d", start, end);
	if (mysql_query(sql_connect, query_string))
	{
		printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
	}
	query_res = mysql_store_result(sql_connect);
	while ((mysql_row = mysql_fetch_row(query_res))) {
		SQL_IMAGE image;
		image.imageId = atoi(mysql_row[0]);
		image.path = char2Tchar(mysql_row[2]);
		image.size = atoi(mysql_row[5]);
		images.push_back(image);
	}
}
void getImagesInfoFromDbByImageIds(int* image_ids,int lenght, MYSQL* sql_connect, std::vector <SQL_IMAGE> &images) {

	MYSQL_RES* query_res;
	MYSQL_ROW  mysql_row;

	char query_string[500];
	for (size_t i = 0; i < lenght; i++)
	{

		sprintf(query_string, "select * from images where image_id =%d", image_ids[i]);
		if (mysql_query(sql_connect, query_string))
		{
			printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
		}
		query_res = mysql_store_result(sql_connect);
		while ((mysql_row = mysql_fetch_row(query_res))) {
			SQL_IMAGE image;
			image.imageId = atoi(mysql_row[0]);
			char* a =mysql_row[2];
			image.path = char2Tchar(mysql_row[2]);
			image.size = atoi(mysql_row[5]);
			images.push_back(image);
		}
	}

}

void ExacFilesFromLibfileToDirWithReturnFilespath(std::vector<SQL_IMAGE> images, TCHAR* exactPath, TCHAR **images_paths) {
	std::vector<int> opened_libfiles_id;
	std::vector<FILE*> opened_libfiles_point;


	FILE * tmpfile, *saved_file;
	TCHAR save_file_name[MAX_PATH], open_lib_file_fullpath[MAX_PATH];
	int count = 0;
	for (auto image : images) {
		std::vector<int>::iterator itr = std::find(opened_libfiles_id.begin(), opened_libfiles_id.end(), image.libfileId);

		if (itr == opened_libfiles_id.end()) {
			// 图片的libfile_id还没打开过
			// 打开libfile
			wsprintf(open_lib_file_fullpath, L"%s\\%s", image.libfile_path, image.libfile_name);
			tmpfile = _wfopen(open_lib_file_fullpath, L"rb");
			opened_libfiles_point.push_back(tmpfile);
			opened_libfiles_id.push_back(image.libfileId);
		}
		else {
			int index = itr - opened_libfiles_id.begin();
			tmpfile = opened_libfiles_point[index];
		}

		printf("\nNow exacting image id: %d", image.imageId);
		wsprintf(save_file_name, L"%s//%d.jpg", exactPath, image.imageId);
		saved_file = _wfopen(save_file_name, L"wb");
		fseek(saved_file, 0, SEEK_SET);
		fseek(tmpfile, image.offset, SEEK_SET);
		short * file_buffer = (short*)malloc(image.size);
		fread(file_buffer, image.size, 1, tmpfile);
		fwrite(file_buffer, image.size, 1, saved_file);
		free(file_buffer);
		fclose(saved_file);

		images_paths[count] = (TCHAR*)malloc(MAX_PATH);
		wsprintf(images_paths[count], L"%s", save_file_name);
		count++;

	}


	for (auto file_point : opened_libfiles_point)
		fclose(file_point);


}
