# dbtool

Tool to update the Database.

## Usage

~~~plain
SYNOPSIS
    dbtool [-h] -a <action> [-r] [-v] [-dbdriver <dbdriver>] [-dbhost <dbhost>] [-dbport <dbport>] [-dbname <dbname>] [-dbuser <dbuser>] [-dbpass <dbpass>] [-d <schemadir>]

OPTIONS
    [-h, -help, -?]
        Show help
    -a, --action <string>
        What to do, possible value(s): update, versions
    [-r, --read-only]
        Do not write to Database
    [-v, --verbose]
        Write out stuff
    [-dbdriver, --database-driver <string>]
        Database driver, possible value(s): pgsql
    [-dbhost, --database-host <string>]
        Host name of database server
    [-dbport, --database-port <integer>]
        Port to connect to
    [-dbname, --database-name <string>]
        Name of database
    [-dbuser, --database-user <string>]
        User name for database
    [-dbpass, --database-password <string>]
        Password for database
    [-d, --schema-dir <string>]
        Directory with .sql files to import

ACTIONS
    update: Update the database
    versions: Show database and table versions
~~~
