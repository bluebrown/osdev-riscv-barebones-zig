#include <stdio.h>

struct Auth {
  enum { AUTH_NONE, AUTH_BASIC, AUTH_BEARER } method;
  union {
    struct {
      char *username;
      char *password;
    };
    struct {
      char *token;
    };
  };
};

void Auth_print(struct Auth *auth) {
  switch (auth->method) {
  case AUTH_NONE:
    printf("No authentication\n");
    break;
  case AUTH_BASIC:
    printf("Password: %s\n", auth->password);
    break;
  case AUTH_BEARER:
    printf("Token: %s\n", auth->token);
    break;
  }
}

int main() {
  struct Auth auth = (struct Auth){
      .method = AUTH_BEARER,
      .token = "eybeef",
  };
  Auth_print(&auth);
}
