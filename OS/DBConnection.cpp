#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

using namespace std;

const string server = "tcp://127.0.0.1";
const string username = "root";
const string password = "1234";

class DBConnection{
public:
	sql::Connection* getConn(){
		sql::Driver *driver;
		sql::Connection *dbConn;

		try
		{
			driver = get_driver_instance();
		}
		catch (sql::SQLException e)
		{
			cout << "Couldn't get database driver : " << e.what() << endl;
			system("pause");
			exit(1);
		}
		try {
			dbConn = driver->connect(server, username, password);
		}

		catch (sql::SQLException e)
		{
			cout << "Couldn't connect to database : " << e.what() << endl;
			system("pause");
			exit(1);
		}

		return dbConn;
	}
};