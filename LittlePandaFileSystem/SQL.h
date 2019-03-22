#pragma once
#include <mysql.h>
#include <stdio.h>
MYSQL* OpenLittlepandaMysql() {
	MYSQL* sql_connect = mysql_init(NULL);
	mysql_options(sql_connect, MYSQL_SET_CHARSET_DIR, "utf8");
	mysql_options(sql_connect, MYSQL_INIT_COMMAND, "SET NAMES utf8");

	char *host = "localhost", *user = "littlepanda", *password = "shinonomehana";
	if (sql_connect == NULL)
	{
		printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
		return NULL;
	}
	if (mysql_real_connect(sql_connect, host,user,password, "littlepanda", 0, NULL, 0) == NULL)
	{
		printf("Error %u: %s\n", mysql_errno(sql_connect), mysql_error(sql_connect));
		return NULL;
	}

	return sql_connect;
}
