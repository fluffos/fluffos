/*
 * Author: Matt Messier (Marius)
 * Description: Sends a formatted string (via sprintf()) to this_player()
 */
varargs
void printf(string msg, mixed *args...)
{
    if (this_player())
        tell_object(this_player(), sprintf(msg, args...));
}
