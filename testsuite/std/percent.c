int percent(int num, int den) {
    if( floatp(num)  || floatp( den ) ) return num * 100.0 / den;
    else return num * 100 / den;
}

int percent_of(int percent, int base) {
    if( floatp(percent) || floatp( base ) ) return percent * base / 100.0;
    else return percent * base / 100;
}
