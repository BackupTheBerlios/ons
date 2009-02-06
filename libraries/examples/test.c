#include <ons/ons.h>
#include <protofcgi/protofcgi.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    cgi_env_t *env;
    printf("Content-Type: text/html\r\n\r\n");

    env = cgi_parse_env();
    if(!env) exit(1);

    unsigned int i;
    for(i = 0; env->http[i].key; ++i) printf("%s = %s\n", env->http[i].key, env->http[i].value);

    return 0;
}
