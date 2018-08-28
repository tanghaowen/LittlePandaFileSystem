#include "stdafx.h"  
#include <iostream>  
#include "windows.h"  
#include <string.h>  
#include <Strsafe.h> 
#include <vector>
#include <stdio.h>
#include "LPFS.h"
#include <mysql.h>
#include <atlstr.h>
#include "SQL.h"



#define READ_BUFF 100
int main(void)
{

	TCHAR * LPFS_LIB_PATH = TEXT("D:/LPFS_LIB");
	TCHAR* image_lib_path = TEXT("D:\\flaskSite\\img");
	TCHAR* exact_path = TEXT("D:\\1");
	int blockFileSizeMB = 1024;
	std::vector <SQL_IMAGE> image_not_added;
	int image_ids[] = { 653567 };





	MYSQL* sql_connect = OpenLittlepandaMysql();
	getImagesInfoFromDbByImageIds(image_ids, 1, sql_connect, image_not_added);

	//getAllNotAddedImagesInfoFromDb(sql_connect, image_not_added);
	//getImagesInfoFromDbWithRange(0, 200, sql_connect, image_not_added);
	//SaveFilesToLibFile( &image_not_added, image_lib_path, blockFileSizeMB,sql_connect,0,LPFS_LIB_PATH);
	SaveFilesToLibFileMemoryTmpMode(&image_not_added, image_lib_path, blockFileSizeMB, sql_connect, 0, LPFS_LIB_PATH);

	//ReadFilesIndexFromDatabase(&image_not_added, sql_connect);
	//ExacFilesFromLibfileToDir(image_not_added, exact_path);

	/*
	TCHAR* images_paths[1000];
	std::vector <SQL_IMAGE> images_to_exact;

	images_to_exact.clear();
	for (size_t i = 0; i < 100; i++)
	{
		SQL_IMAGE image;
		image.imageId = i+1;
		images_to_exact.push_back(image);
	}
	ReadFilesIndexFromDatabase(&images_to_exact, sql_connect);
	ExacFilesFromLibfileToDirWithReturnFilespath(images_to_exact, exact_path, images_paths);
	/*
	ReadFilesIndexFromDatabase(&images, sql_connect);
	ExacFilesFromLibfileToDir(images,exact_path);
	*/
	

	/*

	SaveFilesToLibFile(indexs, files);
	SaveIndexStructToFile(indexs);
	ReadIndexFile(newIndexs);
	ReadFileFromLibfile(newIndexs);*/
	mysql_close(sql_connect);


	return 0;
}




