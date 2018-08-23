#pragma once
#include "IDbConnection.h"
#include "MySQLStatement.h"

class CMySQLConnectionException : public CMySQLException {
public:
	enum {
		E_INIT = 1, \
		E_OUT_OF_MEMORY = 2, \
		E_WRONG_QUERY = 3
	};

	CMySQLConnectionException(const int err_code, const Tchar *err_descr);
	CMySQLConnectionException(MYSQL *conn);
	CMySQLConnectionException(MYSQL_STMT *stmt);

	CMySQLConnectionException(const CMySQLConnectionException &obj);
	CMySQLConnectionException(CMySQLConnectionException &&obj) = default;

	~CMySQLConnectionException();
};

class CMySQLConnection : public IDbConnection {
	enum {
		DEF_CONN_TIMEOUT = 5
	};
	MYSQL *conn;
	bool connected;

	inline void CheckConnected() const;
	MYSQL_STMT *InternalPrepareQuery(const char *query_text) const;
public:
	CMySQLConnection();
	CMySQLConnection(const CMySQLConnection &obj) = delete;
	CMySQLConnection(CMySQLConnection &&obj) = default;

	CMySQLConnection &operator=(const CMySQLConnection &obj) = delete;
	CMySQLConnection &operator=(CMySQLConnection &&obj) = default;

	void Connect(const char *location, const unsigned port, \
					const char *login, \
					const char *pwd, \
					const char *schema_name) override;
	void SetSchema(const char *schema_name) override;

	record_t ExecScalarQuery(const char *query_text) override;
	std::shared_ptr<IDbResultSet> ExecQuery(const char *query_text) const override;
	std::shared_ptr<IDbStatement> PrepareQuery(const char *query_text) const override;

	void Disconnect() override;
	virtual ~CMySQLConnection();
};

void CMySQLConnection::CheckConnected() const {

	//if (!connected)	throw XException(0, "No connection to a MySQL database");
}