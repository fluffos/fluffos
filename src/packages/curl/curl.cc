#include "base/package_api.h"
#include <curl/curl.h>
#include <string>
#include <map>

struct curl_result_t {
    uint16_t status_code;
    std::string status_message;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

static CURL *curl;
static std::unordered_map<LPC_INT, curl_result_t *> *curl_results;
static LPC_INT current_result;

static LPC_INT get_next_result_id() {
  if (current_result ++ > LPC_INT_MAX) {
    current_result = 0;
  }

  if (curl_results->size() >= LPC_INT_MAX) {
    error("Maximum number of curl results reached. Please make sure to curl_free() each request.");
  }

  while (curl_results->count(current_result) > 0) {
    current_result ++;
  }

  return current_result;
}

static size_t write_callback(void *ptr, size_t size, size_t nmemb, std::string *s) {
  s->append(static_cast<char *>(ptr), size*nmemb);
  return size*nmemb;
}

static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {
  /* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */
  /* 'userdata' is set with CURLOPT_HEADERDATA */
  auto header = std::string(buffer, size * nitems);
  header = trim(header);
  auto *result = static_cast<curl_result_t *>(userdata);

  if (starts_with(header, "HTTP")) {
    size_t first_space = header.find(' ');
    size_t second_space = header.find(' ', first_space + 1);

    std::string status_code_string = header.substr(first_space + 1, second_space);
    result->status_code = std::atoi(status_code_string.c_str());

    if (second_space != std::string::npos) {
      std::string status_code_message = header.substr(second_space + 1);

      result->status_message = status_code_message;
    }
  }

  unsigned long index;
  if ((index = header.find(':')) != std::string::npos) {
    auto header_key = header.substr(0, index);
    auto header_value = header.substr(index + 1);

    result->headers.insert(std::make_pair(header_key, trim(header_value)));
  }

  return nitems * size;
}

curl_slist *headers_mapping_to_curl(mapping_t *headers) {
  auto *curl_headers = new curl_slist;

  if (headers && headers->table) {
    mapTraverse(headers, [](mapping_t *, mapping_node_t *node, void *curl_headers_ptr) -> int {
        auto *curl_headers = static_cast<curl_slist *>(curl_headers_ptr);
        std::string key = std::string(node->values[0].u.string);
        std::string value = std::string(node->values[1].u.string);
        std::string header = key + ": " + value;

        curl_slist_append(curl_headers, header.c_str());

        return 0;
    }, &curl_headers);
  }

  return curl_headers;
}

LPC_INT do_curl(LPC_STRING url, mapping_t *headersArg = nullptr, const std::function<void (CURL *)> &callback = nullptr) {
  CURLcode res;
  auto *response_body = new std::string;
  auto *result = new curl_result_t;

  if (!curl) {
    if (!(curl = curl_easy_init())) {
      error("Failed call to curl_easy_init()");
    }

    curl_results = new std::unordered_map<LPC_INT, curl_result_t *>;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_body);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, result);

  curl_slist *headers = nullptr;

  if (headersArg) {
    headers = headers_mapping_to_curl(headersArg);
    curl_easy_setopt(curl, CURLOPT_HEADER, headers);
  }

  callback(curl);

  res = curl_easy_perform(curl);

  curl_easy_cleanup(curl);
  if (headers) {
    curl_slist_free_all(headers);
  }

  /* Check for errors */
  if(res != CURLE_OK) {
    error("curl_post(%s) failed: %s\n", url, curl_easy_strerror(res));
  }

  result->body = *response_body;

  auto result_key = get_next_result_id();

  curl_results->insert(std::make_pair(result_key, result));

  return result_key;
}

#ifdef F_CURL_GET
void f_curl_get(void) {

  LPC_STRING url;
  mapping_t *headers = nullptr;

  int num_arg = st_num_arg;

  url = (sp - num_arg + 1)->u.string;

  if (num_arg > 1) {
    if ((sp - num_arg + 3)->type != T_MAPPING) {
      pop_n_elems(num_arg);
      push_number(0);
      return;
    }

    headers = (sp - num_arg + 3)->u.map;
  }

  pop_n_elems(num_arg);

  auto result_id = do_curl(url, headers);
  push_number(result_id);
}
#endif

#ifdef F_CURL_POST
void f_curl_post(void) {
  CURLcode res;

  LPC_STRING url;
  LPC_STRING data;
  mapping_t *headers = nullptr;

  int num_arg = st_num_arg;

  url = (sp - num_arg + 1)->u.string;

  if (num_arg > 1) {
    data = (sp - num_arg + 2)->u.string;
  }

  if (num_arg > 2) {
    if ((sp - num_arg + 3)->type != T_MAPPING) {
      pop_n_elems(num_arg);
      push_number(0);
      return;
    }

    headers = (sp - num_arg + 3)->u.map;
  }

  pop_n_elems(num_arg);

  auto result_id = do_curl(url, headers, [data](CURL *curl) {
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
  });

  push_number(result_id);
}
#endif

#ifdef F_CURL_GET_STATUS_CODE
void f_curl_get_status_code(void) {
  int num_arg = st_num_arg;
  LPC_INT result_id;

  result_id = (sp - num_arg + 1)->u.number;

  pop_n_elems(num_arg);

  auto results = curl_results->find(result_id);

  if (results->first == 0) {
    error("No results were found for the requested result handler.");
  }

  push_number(results->second->status_code);
}
#endif

#ifdef F_CURL_GET_STATUS_MESSAGE
void f_curl_get_status_message(void) {
  int num_arg = st_num_arg;
  LPC_INT result_id;

  result_id = (sp - num_arg + 1)->u.number;

  pop_n_elems(num_arg);

  auto results = curl_results->find(result_id);

  if (results->first == 0) {
    error("No results were found for the requested result handler.");
  }

  push_constant_string(results->second->status_message.c_str());
}
#endif

#ifdef F_CURL_GET_STATUS_MESSAGE
void f_curl_get_body(void) {
  int num_arg = st_num_arg;
  LPC_INT result_id;

  result_id = (sp - num_arg + 1)->u.number;

  pop_n_elems(num_arg);

  auto results = curl_results->find(result_id);

  if (results->first == 0) {
    error("No results were found for the requested result handler.");
  }

  push_constant_string(results->second->body.c_str());
}
#endif

#ifdef F_CURL_GET_HEADERS
void f_curl_get_headers(void) {
  int num_arg = st_num_arg;
  LPC_INT result_id;

  result_id = (sp - num_arg + 1)->u.number;

  pop_n_elems(num_arg);

  auto results = curl_results->find(result_id);

  if (results->first == 0) {
    error("No results were found for the requested result handler.");
  }

  auto headers = results->second->headers;
  auto *mapping = allocate_mapping(headers.size());

  for(auto kv : headers) {
    add_mapping_string(mapping, kv.first.c_str(), kv.second.c_str());
  }

  push_mapping(mapping);
}
#endif

#ifdef F_CURL_GET_HEADERS
void f_curl_free(void) {
  int num_arg = st_num_arg;
  LPC_INT result_id;

  result_id = (sp - num_arg + 1)->u.number;

  pop_n_elems(num_arg);

  auto results = curl_results->find(result_id);

  if (results->first == 0) {
    push_number(0);
    return;
  }

  curl_results->erase(result_id);

  push_number(1);
}
#endif
