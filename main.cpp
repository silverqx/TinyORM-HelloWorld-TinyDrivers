#include <QCoreApplication>
#include <QDebug>
#include <QLibraryInfo>

#ifdef _WIN32
#  include <qt_windows.h>
#endif

#include <iostream>

#include <orm/db.hpp>
#include <orm/libraryinfo.hpp>
#include <orm/tiny/model.hpp>

using Orm::Constants::EMPTY;
using Orm::Constants::H127001;
using Orm::Constants::ID;
using Orm::Constants::InnoDB;
using Orm::Constants::LT;
using Orm::Constants::NAME;
using Orm::Constants::NEWLINE;
using Orm::Constants::P3306;
using Orm::Constants::QMYSQL;
using Orm::Constants::QUOTE;
using Orm::Constants::SPACE;
using Orm::Constants::ROOT;
using Orm::Constants::TZ00;
using Orm::Constants::UTF8MB4;
using Orm::Constants::UTF8MB40900aici;
using Orm::Constants::Version;
using Orm::Constants::database_;
using Orm::Constants::driver_;
using Orm::Constants::charset_;
using Orm::Constants::collation_;
using Orm::Constants::engine_;
using Orm::Constants::host_;
using Orm::Constants::isolation_level;
using Orm::Constants::options_;
using Orm::Constants::password_;
using Orm::Constants::port_;
using Orm::Constants::prefix_;
using Orm::Constants::prefix_indexes;
using Orm::Constants::qt_timezone;
using Orm::Constants::strict_;
using Orm::Constants::timezone_;
using Orm::Constants::username_;

using Orm::DB;
using Orm::LibraryInfo;
using Orm::QtTimeZoneConfig;
using Orm::Tiny::Model;
using Orm::Utils::Helpers;

using ConfigUtils = Orm::Utils::Configuration;

class User final : public Model<User>
{
    friend Model;
    using Model::Model;
};

std::ostream &operator<<(std::ostream &os, const QString &value)
{
    return os << value.toUtf8().constData();
}

void run()
{
    // Create the MySQL database connection
    // Ownership of a unique_ptr()
    auto manager = DB::create({
        {driver_,         QMYSQL},
        {host_,           qEnvironmentVariable("DB_MYSQL_HOST",      H127001)},
        {port_,           qEnvironmentVariable("DB_MYSQL_PORT",      P3306)},
        {database_,       qEnvironmentVariable("DB_MYSQL_DATABASE",  EMPTY)},
        {username_,       qEnvironmentVariable("DB_MYSQL_USERNAME",  ROOT)},
        {password_,       qEnvironmentVariable("DB_MYSQL_PASSWORD",  EMPTY)},
        {charset_,        qEnvironmentVariable("DB_MYSQL_CHARSET",   UTF8MB4)},
        {collation_,      qEnvironmentVariable("DB_MYSQL_COLLATION", UTF8MB40900aici)},
        // Very important for tests
        {timezone_,       TZ00},
        /* Specifies what time zone all QDateTime-s will have, the overridden default
           is the QTimeZone::UTC, set to the QTimeZone::LocalTime or QtTimeZoneType::DontConvert to
           use the system local time. */
        {qt_timezone,     QVariant::fromValue(QtTimeZoneConfig::utc())},
        {prefix_,         EMPTY},
        {prefix_indexes,  false},
        {strict_,         true},
        {isolation_level, QStringLiteral("REPEATABLE READ")}, // MySQL default is REPEATABLE READ for InnoDB
        {engine_,         InnoDB},
        {Version,         {}}, // Autodetect
        {options_,        ConfigUtils::mysqlSslOptions()},
        // {options_,        ConfigUtils::mariaSslOptions()},
    });

    // Print Qt and TinyORM builds information
    std::cout << QLibraryInfo::build() << NEWLINE
              << LibraryInfo::build() << NEWLINE << NEWLINE;

    // Select using QueryBuilder - normal queries
    {
        auto users = DB::unprepared("select id, name from users where id < 3");

        while(users.next())
            std::cout << users.value(ID).toULongLong() << SPACE << QUOTE
                      << users.value(NAME).toString() << QUOTE << NEWLINE;
    }
    // Select using QueryBuilder - prepared queries
    {
        auto users = DB::select("select id, name from users where id < ?", {3});

        while(users.next())
            std::cout << users.value(ID).toULongLong() << SPACE << QUOTE
                      << users.value(NAME).toString() << QUOTE << NEWLINE;
    }
    // Select using ORM (User model)
    {
        const auto users = User::where(ID, LT, 3)->get({ID, NAME});

        for (const auto &user : users)
            std::cout << user.getAttribute<quint64>(ID) << SPACE << QUOTE
                      << user.getAttribute<QString>(NAME) << QUOTE << NEWLINE;
    }
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
//    SetConsoleOutputCP(1250);
#endif

    try {
        /* Needed from Qt v6.5.3 to avoid:
           qt.core.qobject.connect: QObject::connect(QObject, Unknown): invalid nullptr parameter */
        const QCoreApplication app(argc, argv);

        run();
    } catch (const std::exception &e) {

        Helpers::logException(e);

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/*
   Expected output:

   Qt 6.7.0 (x86_64-little_endian-llp64 shared (dynamic) debug build; by MSVC 2019)
   TinyORM 0.36.5 (x86_64-little_endian-llp64 shared debug build; by MSVC 2022 (1939))

   Executed unprepared query (20ms, 2 results, 2 affected, tinyorm_default) : select id, name from users where id < 3
   1 "andrej"
   2 "silver"
   Executed prepared query (0ms, 2 results, 2 affected, tinyorm_default) : select id, name from users where id < 3
   1 "andrej"
   2 "silver"
   Executed prepared query (0ms, 2 results, 2 affected, tinyorm_default) : select `id`, `name` from `users` where `id` < 3
   1 "andrej"
   2 "silver"
*/
