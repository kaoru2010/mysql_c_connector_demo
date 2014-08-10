#include <mysql.h>

int main()
{
    MYSQL mysql;
    mysql_init(&mysql);


    mysql_close(&mysql);
}
