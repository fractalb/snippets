/* Reverse words delimeted by spaces or tabs */
#include <stdio.h>

static inline void swap(char *start, char *end) {
  char temp = *start;
  *start = *end;
  *end = temp;
  return;
}

void reverse_chars(char *start, char *end) {
  while (end > start) {
    swap(start, end);
    start++, end--;
  }
  return;
}

/*
 * Only spaces or tabs are treated as word delimeters.
 * Everything else is treated as part of a word.
 * The str argument is a NUL terminated C-string.
 */
void reverse_words(char *str) {
  char *start, *end;
  start = str;

  /* make sure that 'start' points to a
   * non-space character */
  while (*start == ' ' || *start == '\t')
    start++;

  end = start;
  while (*end != '\0') {
    end++;
    if (*end == ' ' || *end == '\t') {
      reverse_chars(start, end - 1);
      end++;
      /* Skip to next non-space character */
      while (*end == ' ' || *end == '\t')
        end++;
      start = end;
    }
  }

  end--;
  reverse_chars(start, end);
  reverse_chars(str, end);
  return;
}

/* Test reverse words function */
int main() {
  char input_string[128];

  fgets(input_string, sizeof(input_string), stdin);
  reverse_words(input_string);
  printf("%s\n", input_string);
  return 0;
}
