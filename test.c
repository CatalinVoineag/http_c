#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include "request_line.c"

Test(request_from_reader, basic) {
  char input[1000] = "GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n";

  request_line_t request_line = request_from_reader(input);
  cr_assert(eq(str, request_line.method, "GET"));
  cr_assert(eq(str, request_line.target, "/"));
  cr_assert(eq(str, request_line.version, "1.1"));
};

Test(request_from_reader, post) {
  char input[1000] = "POST /coffee HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n";

  request_line_t request_line = request_from_reader(input);
  cr_assert(eq(str, request_line.method, "POST"));
  cr_assert(eq(str, request_line.target, "/coffee"));
  cr_assert(eq(str, request_line.version, "1.1"));
};

Test(request_from_reader, request_not_http_1_1) {
  char input[1000] = "GET / HTTP/1.2\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n";

  request_line_t request_line = request_from_reader(input);
  cr_assert(eq(str, request_line.error, "Request version needs to be HTTP 1.1"));
};

Test(request_from_reader, too_many_items) {
  char input[1000] = "GET POST / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n";

  request_line_t request_line = request_from_reader(input);
  cr_assert(eq(str, request_line.error, "Too many items in the Request Line"));
};

Test(request_from_reader, not_all_items) {
  char input[1000] = "/ HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n";

  request_line_t request_line = request_from_reader(input);
  cr_assert(eq(str, request_line.error, "Incomplete attributes"));
};

Test(request_from_reader, out_of_order) {
  char input[1000] = "/coffee GET HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n";

  request_line_t request_line = request_from_reader(input);
  cr_assert(eq(str, request_line.error, "Method invalid"));
};

Test(request_from_reader, invalid_request) {
  char input[1000] = "OPTIONS /prime/rib TCP/1.1\r\nHost: localhost:42069\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n";

  request_line_t request_line = request_from_reader(input);
  cr_assert(eq(str, request_line.error, "Request needs to be HTTP"));
};
