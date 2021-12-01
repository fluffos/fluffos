/**
 * @brief LPMUD数据库操作模块，使用链式调用优雅的增删改查
 * @author 雪风@mud.ren
 */
#ifdef __PACKAGE_DB__

// 数据库配置
nosave string db_host;
nosave string db_db;
nosave string db_user;
nosave int db_type = __DEFAULT_DB__;
// 数据库查询
nosave mixed db_handle;
// 错误消息
nosave string db_error;
/**
 * @brief The table which the query is targeting.
 *
 */
nosave string db_table;
// 数据表头
nosave string *db_table_column = ({});

/**
 * @brief SQL语句
 *
 * SELECT
 * [DISTINCT] <select_list>
 * FROM <left_table>
 * [<join_type> JOIN <right_table>]
 * [ON <join_condition>]
 * [WHERE <where_condition>]
 * [GROUP BY <group_by_list>]
 * [HAVING <having_condition>]
 * [ORDER BY <order_by_condition>]
 * [LIMIT <limit_number>]
 * [OFFSET <offset_number>]
 * [UNION [ALL]]
 * [SELECT ***]
 */
nosave string db_sql;
/**
 * @brief The columns that should be returned.
 *
 */
nosave string db_sql_columns = "*";
/**
 * @brief The where constraints for the query.
 *
 */
nosave string db_sql_where;
nosave string *db_sql_wheres = ({});
/**
 * @brief The groupings for the query.
 *
 */
nosave string db_sql_groups;
/**
 * @brief The having constraints for the query.
 *
 */
nosave string db_sql_havings;
/**
 * @brief The orderings for the query.
 *
 */
nosave string db_sql_orders;
/**
 * @brief The maximum number of records to return.
 *
 */
nosave int db_sql_limit;
/**
 * @brief The number of records to skip.
 *
 */
nosave int db_sql_offset;

// 状态和条件
nosave int db_distinct;
nosave int db_inRandomOrder;
nosave int db_withColumn;
nosave int db_autoClose = 1;
/**
 * @brief 数据库连接初始化
 *
 * @param host
 * @param db
 * @param user
 * @return void
 */
varargs void create(string host, string db, string user, int type)
{
    if (host)
    {
        db_host = host;
    }
    if (db)
    {
        db_db = db;
    }
    if (user)
    {
        db_user = user;
    }
    if (type)
    {
        db_type = type;
    }
}
/**
 * @brief 重置数据库链接配置
 *
 * @param db
 */
void setConnection(mapping db)
{
    db_host = db["host"];
    db_db = db["database"];
    db_user = db["user"];
    if (db["type"])
    {
        db_type = db["type"];
    }
}
/**
 * @brief 是否自动关闭数据库连接
 *
 * @param flag
 */
void setAutoClose(int flag)
{
    db_autoClose = flag;
}
// 重置查询
void resetSql()
{
    db_error = 0;
    db_withColumn = 0;
    db_distinct = 0;
    db_sql = "";
    db_sql_columns = "*";
    db_sql_where = "";
    db_sql_wheres = ({});
    db_sql_groups = "";
    db_sql_havings = "";
    db_sql_orders = "";
    db_sql_limit = 0;
    db_sql_offset = 0;
    db_inRandomOrder = 0;
}
/**
 * @brief 原生SQL调用
 *
 * @return this_object()
 */
object sql(string sql)
{
    resetSql();
    db_sql = sql;

    return this_object();
}
/**
 * @brief 构造数据表调用
 *
 * @param table
 * @return object
 */
object table(string table)
{
    resetSql();
    db_table = table;

    return this_object();
}
/**
 * @brief 使用DISTINCT过滤重复数据
 *
 * @return object
 */
object distinct()
{
    db_distinct = 1;

    return this_object();
}
/**
 * @brief 针对SQL展开数组的处理
 *
 * @param arr 需处理的数组
 * @param del 分隔符
 * @return string
 */
string implodeX(mixed *arr, string del)
{
    string s = "";

    foreach (mixed x in arr)
    {
        // debug_message(typeof(x));
        switch (typeof(x))
        {
        case "int":
        case "float":
            s += x + del;
            break;
        default:
            s += "'" + x + "'" + del;
            break;
        }
    }

    return s[0.. < sizeof(del) + 1];
}
/**
 * @brief where条件查询处理
 *
 * @param where
 * @param boolean 条件关系" AND "、" OR "...
 */
void addArrayOfWheres(mixed *where, string boolean)
{
    foreach (mixed *x in where)
    {
        if (arrayp(x) && sizeof(x) == 2)
        {
            db_sql_wheres += ({x[0] + "='" + x[1] + "'"});
        }
        else if (arrayp(x) && sizeof(x) == 3)
        {
            db_sql_wheres += ({x[0] + " " + x[1] + " '" + x[2] + "'"});
        }
    }
    if (boolean == " OR ")
    {
        db_sql_where += " OR " + implode(db_sql_wheres, " AND ");
    }
    else
    {
        db_sql_where = implode(db_sql_wheres, " AND ");
    }
}

object where(mixed *x...)
{
    // debug_message(sprintf("%O", x));
    if (sizeof(db_sql_where))
    {
        db_sql_where += " AND ";
    }
    if (arrayp(x[0]))
    {
        addArrayOfWheres(x[0], " AND ");
    }
    else if (sizeof(x) == 2)
    {
        db_sql_where += x[0] + "='" + x[1] + "'";
    }
    else if (sizeof(x) == 3)
    {
        db_sql_where += x[0] + " " + x[1] + " '" + x[2] + "'";
    }

    return this_object();
}

object orWhere(mixed *x...)
{
    if (arrayp(x[0]))
    {
        addArrayOfWheres(x[0], " OR ");
    }
    else if (sizeof(x) == 2)
    {
        db_sql_where += " OR " + x[0] + "='" + x[1] + "'";
    }
    else if (sizeof(x) == 3)
    {
        db_sql_where += " OR " + x[0] + " " + x[1] + " '" + x[2] + "'";
    }

    return this_object();
}

varargs object whereBetween(string column, mixed *x, int not)
{
    string between = " BETWEEN ";

    if (not)
    {
        between = " NOT BETWEEN ";
    }

    if (sizeof(db_sql_where))
    {
        db_sql_where += " AND ";
    }

    if (sizeof(x) == 2)
    {
        db_sql_where += column + between + x[0] + " AND " + x[1];
    }

    return this_object();
}

object whereNotBetween(string column, mixed *x)
{
    return whereBetween(column, x, 1);
}

varargs object orWhereBetween(string column, mixed *x, int not)
{
    string between = " BETWEEN ";

    if (not)
    {
        between = " NOT BETWEEN ";
    }

    if (sizeof(x) == 2)
    {
        db_sql_where += " OR " + column + between + x[0] + " AND " + x[1];
    }

    return this_object();
}

object orWhereNotBetween(string column, mixed *x)
{
    return orWhereBetween(column, x, 1);
}

varargs object whereNull(string column, int not)
{
    string null = " IS NULL";

    if (not)
    {
        null = " IS NOT NULL";
    }

    if (sizeof(db_sql_where))
    {
        db_sql_where += " AND ";
    }

    db_sql_where += column + null;

    return this_object();
}

object whereNotNull(string column)
{
    return whereNull(column, 1);
}

varargs object orWhereNull(string column, int not)
{
    string null = " IS NULL";

    if (not)
    {
        null = " IS NOT NULL";
    }

    db_sql_where += " OR " + column + null;

    return this_object();
}

object orWhereNotNull(string column)
{
    return orWhereNull(column, 1);
}

object whereIn(string column, mixed *x, int not)
{
    string notin = " IN ";

    if (not)
    {
        notin = " NOT IN ";
    }

    if (sizeof(db_sql_where))
    {
        db_sql_where += " AND ";
    }

    db_sql_where += column + notin + "(" + implodeX(x, ",") + ")";

    return this_object();
}

object whereNotIn(string column, mixed *x)
{
    return whereIn(column, x, 1);
}

object orWhereIn(string column, mixed *x, int not )
{
    string notin = " IN ";

    if (not)
    {
        notin = " NOT IN ";
    }

    db_sql_where += " OR " + column + notin + "(" + implodeX(x, ",") + ")";

    return this_object();
}

object orWhereNotIn(string column, mixed *x)
{
    return orWhereIn(column, x, 1);
}
/**
 * @brief 分组
 *
 * @param column
 * @return object
 */
object groupBy(string *column...)
{
    // todo
    return this_object();
}
/**
 * @brief 分组过滤
 *
 * @param column
 * @param operator
 * @param value
 * @param boolean
 * @return object
 */
object having(string column, string operator, mixed value, string boolean)
{
    // todo
    return this_object();
}
/**
 * @brief 排序
 *
 * @param column
 * @param order asc / desc
 * @return object
 */
varargs object orderBy(string column, string order)
{
    if (!nullp(column))
    {
        if (!stringp(order) || member_array(lower_case(order), ({"asc", "desc"})) < 0)
        {
            order = "ASC";
        }
        if (sizeof(db_sql_orders))
        {
            db_sql_orders += ",";
        }

        db_sql_orders += column + " " + upper_case(order);
    }

    return this_object();
}

object inRandomOrder()
{
    db_inRandomOrder = 1;

    return this_object();
}
/**
 * @brief limit处理
 *
 * @param n
 * @return object
 */
object limit(int n)
{
    db_sql_limit = n;

    return this_object();
}
/**
 * @brief offset处理
 *
 * @param n
 * @return object
 */
object offset(int n)
{
    db_sql_offset = n;

    return this_object();
}

object with(string str)
{
    switch (str)
    {
    case "column":
        db_withColumn = 1;
        break;

    default:
        break;
    }

    return this_object();
}

// 构造条件语句
private void db_sql_bindings()
{
    if (sizeof(db_sql_where))
    {
        db_sql += " WHERE " + db_sql_where;
    }

    if (sizeof(db_sql_orders))
    {
        db_sql += " ORDER BY " + db_sql_orders;
    }

    if (db_sql_limit)
    {
        db_sql += " LIMIT " + db_sql_limit;

        if (db_sql_offset)
        {
            db_sql += " OFFSET " + db_sql_offset;
        }
    }
}
// 构造查询语句
private string db_sql()
{
    if (!sizeof(db_sql))
    {
        if (db_distinct)
        {
            db_sql_columns = "DISTINCT " + db_sql_columns;
        }

        db_sql = "SELECT " + db_sql_columns + " FROM " + db_table;
        db_sql_bindings();
    }

    return db_sql;
}
/**
 * @brief 连接数据库并返回handle
 *
 */
private mixed connect()
{
    // 连接数据库
    if (!db_handle || stringp(db_handle))
    {
        db_handle = db_connect(db_host, db_db, db_user, db_type);
        /* error */
        if (stringp(db_handle))
            return db_error = db_handle;
        else
        {
            // 默认mysql编码
            // db_exec(db_handle, "set names utf8mb4");
        }
    }
    return db_handle;
}
/**
 * @brief 关闭数据库连接
 *
 */
varargs mixed close(int flag)
{
    if ((flag || db_autoClose) && intp(db_handle) && db_handle && db_close(db_handle))
    {
        db_handle = 0;
    }

    return db_handle;
}
/**
 * @brief 执行SQL语句并返回结果行数
 *
 */
varargs mixed exec()
{
    mixed rows;
    // 连接数据库
    if (stringp(connect()))
    {
        return db_error;
    }
    // 执行SQL语句
    rows = db_exec(db_handle, db_sql());
    /* error */
    if (stringp(rows))
    {
        close();
        return db_error = rows;
    }
    // 保存数据表头
    db_table_column = db_fetch(db_handle, 0);

    return rows;
}

varargs mixed get(string *columns...)
{
    mixed rows, *res;

    if (sizeof(columns))
    {
        db_sql_columns = implode(columns, ",");
    }

    rows = exec();
    /* error */
    if (stringp(db_error))
    {
        return db_error;
    }

    res = allocate(rows);
    for (int i = 1; i <= rows; i++)
    {
        res[i - 1] = db_fetch(db_handle, i);
    }
    close();

    if (db_inRandomOrder)
    {
        res = shuffle(res);
    }
    if (db_withColumn)
    {
        res = ({ db_table_column }) + res;
    }

    return res;
}

mixed pluck(string column)
{
    mixed *res, *arr = ({});
    int index;

    db_sql_columns = column;
    res = get();
    /* error */
    if (stringp(db_error))
    {
        return db_error;
    }
    index = member_array(column, db_table_column);
    for (int i = 0; i < sizeof(res); i++)
    {
        arr += ({res[i][index]});
    }

    return arr;
}

varargs mixed first(string *columns...)
{
    mixed rows, *res;
    int i = 1;

    if (sizeof(columns))
    {
        db_sql_columns = implode(columns, ",");
    }

    rows = exec();
    /* error */
    if (stringp(db_error))
    {
        return db_error;
    }

    if (db_inRandomOrder)
    {
        i = random(rows) + 1;
    }
    res = db_fetch(db_handle, i);
    close();

    return res;
}

mixed find(int id)
{
    return where("id", "=", id)->first();
}

mixed value(string column)
{
    mixed *res = first(column);
    int index = member_array(column, db_table_column);

    if (sizeof(res))
    {
        return res[index];
    }

    return "";
}

/**
 * @brief 数据库聚合函数
 *
 * @param func
 * @param column
 * @return private
 */
private mixed aggregate(string func, mixed column)
{
    mixed rows, *res;

    if (column)
    {
        db_sql = "SELECT " + func + "(" + column + ") FROM " + db_table;
        db_sql_bindings();
    }
    else
    {
        return "";
    }

    rows = exec();
    /* error */
    if (stringp(db_error))
    {
        return db_error;
    }

    res = db_fetch(db_handle, 1);
    close();

    return res[0];
}

varargs mixed count(mixed column)
{
    if (nullp(column))
    {
        column = 1;
    }

    return aggregate("COUNT", column);
}

mixed max(string column)
{
    return aggregate("MAX", column);
}

mixed min(string column)
{
    return aggregate("MIN", column);
}

mixed avg(string column)
{
    return aggregate("AVG", column);
}

mixed sum(string column)
{
    return aggregate("SUM", column);
}

/**
 * @brief 插入
 *
 */
mixed insert(mapping m)
{
    // 构造插入语句
    db_sql = "INSERT INTO " + db_table + " (" + implode(keys(m), ",") + ") VALUES (" + implodeX(values(m), ",") + ")";
    //执行SQL
    exec();
    /* error */
    if (stringp(db_error))
    {
        return db_error;
    }
    close();

    return 1;
}

/**
 * @brief 更新
 *
 */
mixed update(mapping m)
{
    mixed key, value;
    string sql = "";
    foreach (key, value in m)
    {
        value = typeof(value) == "string" ? "'" + value + "'" : value;
        sql += key + "=" + value + ",";
    }
    sql = sql[0.. < 2];
    // 构造更新语句
    db_sql = "UPDATE " + db_table + " SET " + sql;
    db_sql_bindings();
    //执行SQL
    exec();
    /* error */
    if (stringp(db_error))
    {
        return db_error;
    }
    close();

    return 1;
}

/**
 * @brief 删除
 *
 */
mixed delete()
{
    // 构造删除语句
    db_sql = "DELETE FROM " + db_table;
    db_sql_bindings();
    //执行SQL
    exec();
    /* error */
    if (stringp(db_error))
    {
        return db_error;
    }
    close();

    return 1;
}

/**
 * @brief 调试
 *
 */
string dump()
{
    string line = repeat_string("-*-", 20);
    return sprintf("\n%s\ndb_host = %s\ndb_db = %s\ndb_user = %s\ndb_handle = %d\ndb_error = %O\ndb_table = %s\ndb_table_column = %O\ndb_sql = %s\ndb_status = %s\n%s\n", line, db_host, db_db, db_user, db_handle, db_error, db_table, db_table_column, db_sql, db_status(), line);
}

#endif
