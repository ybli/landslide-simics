/* parse_c0ffi.c
 * This file automatically generated by the command 
 * 'wrappergen parse' - editing is probably bad.
 * 
 * The call to wrappergen was likely probably by 
 * 'make libs/parse', which in turn was likely
 * invoked by 'make libs'. */

#include <stdint.h>
#include <c0runtime.h>

/* Headers */

bool * parse_bool(c0_string s);
int * parse_int(c0_string s, int base);

/* Wrappers */

void *__c0ffi_parse_bool(void **args) {
  return (void *) parse_bool((c0_string) args[0]);
}

void *__c0ffi_parse_int(void **args) {
  return (void *) parse_int((c0_string) args[0], (int) (intptr_t) args[1]);
}

