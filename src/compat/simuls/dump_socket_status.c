/*
 * Author: Tim Hollebeek (Beek)
 * Description: Returns a formatted string table of socket information.
 *              Uses the socket_status() efun which obsoleted this one.
 */

#ifdef __PACKAGE_SOCKETS__
string dump_socket_status()
{
    string ret = @END
Fd    State      Mode       Local Address          Remote Address
--  ---------  --------  ---------------------  ---------------------
END;

    foreach (mixed *item in socket_status())
    {
	ret += sprintf("%2d  %|9s  %|8s  %-21s  %-21s\n", item[0], item[1], item[2], item[3], item[4]);
    }

    return ret;
}
#endif
