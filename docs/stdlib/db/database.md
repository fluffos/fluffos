---
layout: default
title: stdlib / database
---

## Introduction

The database query builder provides a convenient, fluent interface to creating and running database queries. It can be used to perform most database operations in your mud and works perfectly with all of fluffos supported database systems.

## Config

You can inherit `/std/database.c` and set config with `setConnection()`, for example:

```c
inherit "/std/database";

void test()
{
    mixed res;
    // use SQLITE3
    mapping db = ([
        "host":"",
        "database":"/data/db.sqlite",
        "user":"",
        "type":__USE_SQLITE3__
    ]);
    database::setConnection(db);
    // database queries
    printf("%O\n", database::table("users")->get());
}
```

The database type can be `__USE_MYSQL__`、`__USE_SQLITE3__`、`__USE_POSTGRE__` or `__DEFAULT_DB__`。

Or, clone database object like this:

```c
void test()
{
    object db = new("/std/database", "", "/sqlite.db", "", __USE_SQLITE3__);
    printf("%O\n", db->table("users")->get());
}
```

## Running Database Queries

### Retrieving All Rows From A Table

You may use the `table` method provided by the `/std/database.c` to begin a query. The `table` method returns a fluent query builder instance for the given table, allowing you to chain more constraints onto the query and then finally retrieve the results of the query using the `get` method:

```c
void test()
{
    object db = new("/std/database", "", "/sqlite.db", "", __USE_SQLITE3__);
    // Retrieving all rows from users table
    printf("%O\n", db->table("users")->get());
}
```

The `get` method return a 2D array.

### Retrieving A Single Row / Column From A Table

If you just need to retrieve a single row from a database table, you may use the `first` method. This method will return an array:

```c
    printf("%O\n", db->table("users")->first());
```

If you don't need an entire row, you may extract a single value from a record using the `value` method. This method will return the value of the column directly:

```c
    printf("%s\n", db->table("users")->where("name", "mudren")->value("email"));
```

To retrieve a single row by its id column value, use the `find` method:


```c
    printf("%O\n", db->table("users")->find(3));
```

### Retrieving A List Of Column Values

If you would like to retrieve result containing the values of a single column, you may use the `pluck` method. In this example, we'll retrieve an array of user names:

```c
    printf("%O\n", db->table("users")->pluck("name"));
```

### Aggregates

The query builder also provides a variety of methods for retrieving aggregate values like `count`, `max`, `min`, `avg`, and `sum`. You may call any of these methods after constructing your query:

```c
    printf("user_count = %d\n", db->table("users")->count());
    printf("max_age = %d\n", db->table("users")->max("age"));
```

Of course, you may combine these methods with other clauses to fine-tune how your aggregate value is calculated:

```c
    printf("count = %d\n", db->table("users")->where("age", 14)->count());
```

## Raw Expressions

Sometimes you may need to insert an arbitrary string into a query. To create a raw string expression, you may use the `sql` and `exec` method：

```c
    object db = new("/std/database", "", "/sqlite.db", "", __USE_SQLITE3__);
    mixed res;
    db->sql("DROP TABLE IF EXISTS `users`")->exec();
    res = db->sql("CREATE TABLE IF NOT EXISTS `users` (`id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,`name` varchar(25) NOT NULL,`bio` TEXT(255) DEFAULT NULL,`activated_at` timestamp DEFAULT NULL)")->exec();
```

## Basic Where Clauses

### Where Clauses

You may use the query builder's `where` method to add "where" clauses to the query. The most basic call to the `where` method requires three arguments. The first argument is the name of the column. The second argument is an operator, which can be any of the database's supported operators. The third argument is the value to compare against the column's value.

For example, the following query retrieves users where the value of the `exp` column is equal to `100` and the value of the `age` column is greater than `15`:

```c
res = database::table("users")
            ->where("exp", "=", 100)
            ->where("age", ">", 15)
            ->get();
```

For convenience, if you want to verify that a column is `=` to a given value, you may pass the value as the second argument to the `where` method. database will assume you would like to use the `=` operator:

```c
    res = database::table("users")->where("age", 16)->get();
```

As previously mentioned, you may use any operator that is supported by your database system:

```c
users = database::table("users")
                ->where("id", ">=", 100)
                ->get();

users = database::table("users")
                ->where("id", "<>", 100)
                ->get();

users = database::table("users")
                ->where("email", "like", "%@mud.ren")
                ->get();
```

You may also pass an array of conditions to the `where` function. Each element of the array should be an array containing the three arguments typically passed to the where method:

```c
// WHERE user_id > 7 AND category_id = 4
res = db->table("topics")->where(({ ({"user_id", ">", 7}), ({"category_id", 4}) }))->get();
```

### Or Where Clauses

When chaining together calls to the query builder's `where` method, the "where" clauses will be joined together using the `and` operator. However, you may use the `orWhere` method to join a clause to the query using the `or` operator. The `orWhere` method accepts the same arguments as the `where` method:

```c
// WHERE user_id > 7 OR category_id = 4
res = db->table("topics")->where("user_id", ">", 7)->orWhere("category_id", 4)->get();
```

### Additional Where Clauses

#### whereBetween / orWhereBetween

The `whereBetween` method verifies that a column's value is between two values:

```c
users = db->table("users")->whereBetween("age", ({14, 18}))->get();
```


#### whereNotBetween / orWhereNotBetween

The `whereNotBetween` method verifies that a column's value lies outside of two values:

```c
users = db->table("users")->whereNotBetween("age", ({14, 18}))->get();
```

#### whereIn / whereNotIn / orWhereIn / orWhereNotIn

The `whereIn` method verifies that a given column's value is contained within the given array. The `whereNotIn` method verifies that the given column's value is not contained in the given array.

```c
users = db->table("users")->whereIn("age", ({14, 18, 24, 30}))->get();
```

#### whereNull / orWhereNull / whereNotNull / orWhereNotNull

The `whereNull` method verifies that the value of the given column is `NULL`, The `whereNotNull` method verifies that the column's value is not `NULL`:

```c
users = db->table("users")->whereNull("updated_at")->get();
```

## Ordering, Grouping, Limit & Offset

### Ordering

The `orderBy` method allows you to sort the results of the query by a given column. The first argument accepted by the `orderBy` method should be the column you wish to sort by, while the second argument determines the direction of the sort and may be either `asc` or `desc`:

```c
users = db->table("users")
            ->orderBy("name", "desc")
            ->get();
```

To sort by multiple columns, you may simply invoke `orderBy` as many times as necessary:

```c
users = db->table("users")
            ->orderBy("name", "desc")
            ->orderBy("email", "asc")
            ->get();
```

### Random Ordering

The `inRandomOrder` method may be used to sort the query results randomly. For example, you may use this method to fetch a random user:

```c
randomUser = DB->table("users")
                ->inRandomOrder()
                ->get();
```

### Limit & Offset

You may use the `limit` and `offset` methods to limit the number of results returned from the query or to skip a given number of results in the query:

```c
users = db->table("users")
            ->offset(10)
            ->limit(5)
            ->get();
```

## Insert Statements

The query builder also provides an `insert` method that may be used to insert records into the database table. The `insert` method accepts an mapping of column names and values:

```c
db->table("users")->insert((["name":"test", "email":"test@mud.ren"]));
```

## Update Statements

In addition to inserting records into the database, the query builder can also update existing records using the `update` method. The `update` method, like the insert method, accepts an mapping of column and value pairs indicating the columns to be updated. You may constrain the update query using where clauses:

```c
db->table("users")->where("id", ">", 120)->limit(5)->update((["level":99]));
```

## Delete Statements 

The query builder's `delete` method may be used to delete records from the table. You may constrain delete statements by adding "where" clauses before calling the `delete` method:

```c
db->table("users")->delete();
db->table("users")->where("id",">", 5)->limit(5)->delete();
```

## Auto Close

By default, database will auto close connection and release db_handle, you can use `setAutoClose(0)` disable this and close the database connection with `close(1)`：

```c
void test()
{
    object DB = new(CORE_DB, host, db, user);
    // close auto close
    DB->setAutoClose(0);

    // ...sql

    // close
    DB->close(1);
}
```

## Debugging

You may use the `dump` methods while building a query to dump the current query bindings and SQL. 

```c
printf(db->dump());
```

    -*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*-
    db_host = 127.0.0.1
    db_db = mud
    db_user = root
    db_handle = 1
    db_error = 0
    db_table = topics
    db_table_column = ({ /* sizeof() == 2 */
    "user_id",
    "title"
    })
    db_sql = SELECT user_id,title FROM topics WHERE user_id > '7' AND category_id='4'
    -*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*-
