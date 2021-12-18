int curl_post(string, string | void, mapping | void);
int curl_get(string, mapping | void);
int curl_get_status_code(int);
string curl_get_status_message(int);
string curl_get_body(int);
mapping curl_get_headers(int);
void curl_free(int);
