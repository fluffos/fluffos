/*
 * Author: Matt Messier (Marius)
 * Description: Supporting code required in an interactive object to simulate
 *              functionality that has been removed from the driver.
 */

#ifdef OLD_ED
class ed_session
{
    function exitfn;
    int      restricted;
    string   fname;
}

private nosave ed_sessions = ({});

void push_ed_session(string fname, function exitfn, mixed writefn, int restricted)
{
    class ed_session new_session;
    string           err;

    new_session = new(class ed_session);
    new_session->restricted = restricted;
    new_session->exitfn     = exitfn;
    new_session->fname      = fname;

    ed_sessions = ({ new_session }) + ed_sessions;
    if ((err = catch {
        if (writefn)
            write(ed_start(fname, restricted, writefn));
        else
            write(ed_start(fname, restricted));
    }) != 0)
    {
        ed_sessions = ed_sessions[1..];
        error(err[1..]);
    }
}

#endif

string process_input(string input)
{
#ifdef OLD_ED
    if (query_ed_mode() != -1)
    {
        write(ed_cmd(input));
        if (query_ed_mode() == -1)
        {
            if (ed_sessions[0]->exitfn)
                catch( evaluate(ed_sessions[0]->exitfn) );
            ed_sessions = ed_sessions[1..];
        }
        return 1;   // return 1 to indicate we handled the command
    }
#endif
}

void net_dead(void)
{
    ed_sessions = ({});
}

void write_prompt(void)
{
#ifdef OLD_ED
    switch (query_ed_mode())
    {
        case -2:
        case 0:
            write(":");
            return;

        case -1:
            break;

        default:
            write("*\b");
            return;
    }
#endif
    write(">");
}
