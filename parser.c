#include <bsd/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 1024
#define MAX_VALUE_SIZE 512

char *get_value(char *line, const char *key) {
  char *next = line;
  char *s = strsep(&next, "=");
  if (s == NULL || next == NULL) return NULL;
  s += strspn(s, "\t \v");
  if (strncmp(key, s, strcspn(s, "\t \v")) != 0) return NULL;

  /* The value is assumed to be wrapped in a pair of single quotes */
  char *s1 = strsep(&next, "'");
  if (s1 == NULL || next == NULL) return NULL;
  char *s2 = strsep(&next, "'");
  if (s2 == NULL || next == NULL) return NULL;
  return s2;
}

int parse(const char *file, const char *key, char value[MAX_VALUE_SIZE]) {
  int err = -1;
  FILE *stream;
  char *line = malloc(MAX_LINE_SIZE);
  size_t len = MAX_LINE_SIZE;
  ssize_t nread;

  stream = fopen(file, "r");
  if (stream == NULL) {
    perror("fopen");
    return err;
  }

  while ((nread = getline(&line, &len, stream)) != -1) {
    const char *_value = get_value(line, key);
    if (_value == NULL) continue;
    if (strlcpy(value, _value, MAX_VALUE_SIZE) > MAX_VALUE_SIZE)
      printf("Value truncated!\n");
    err = 0;
    break;
  }

  free(line);
  fclose(stream);
  return err;
}

int main() {
  char value[MAX_VALUE_SIZE];
  if (parse("test.properties", "ENV_CERT_CHAIN", value) != 0) {
    printf("Error parsing the file\n");
    return -1;
  }
  printf("value = %s\n", value);
  return 0;
}