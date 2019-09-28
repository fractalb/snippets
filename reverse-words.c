#include <stdio.h>

/* Words delimeted by spaces. */
 
static inline void swap(char *start, char *end)
{
	char temp = *start;
	*start = *end;
	*end = temp;

	return;
}

void reverse_chars(char *start, char *end)
{
	while (end > start) {
		swap(start, end);
		start++, end--;
	}

	return;
}

void reverse_words(char *str)
{
	char *start, *end;
	start = end = str;

	while (*end != '\0') {
		end++;
		if (*end == ' ' || *end == '\t' || *end == '\n') {
			reverse_chars(start, end-1);
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
int main()
{
	char input_string[128];

	printf("Enter a string: ");
	fgets(input_string, sizeof(input_string), stdin);
	reverse_words(input_string);
	printf("Reversed words: %s\n", input_string);
	return 0;
}
