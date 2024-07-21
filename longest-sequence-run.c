#include <stdio.h>
#include <stdlib.h>

/**
 * Returns the start index of the longest run of non-decresing sequence.
 * `length` parameter is updated on function return to denote the length
 * of the sequence starting from the returned index.
 *
 * On error, -1 is returned
 */
int longest_non_decreasing_sequence_run(int arr[], int n_elem, int *length)
{
	if (n_elem <= 0)
		return -1;

	int index = 0;
	int len = 1;

	int _len = 1;
	for (int i = 1; i < n_elem; i++, _len++) {
		if (arr[i] < arr[i - 1]) {
			if (_len > len) {
				len = _len;
				index = i - _len;
			}
			_len = 0;
		}
	}
	if (_len > len) {
		len = _len;
		index = n_elem - _len;
	}
	*length = len;
	return index;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
		return 0;
	int *arr = malloc(argc * sizeof(int));
	if (arr == NULL)
		return -1;
	for (int i = 1; i < argc; i++) {
		arr[i - 1] = strtol(argv[i], NULL, 10);
	}
	int len, index;
	index = longest_non_decreasing_sequence_run(arr, argc - 1, &len);
	printf("Longest run index = %d, length = %d (%d - %d)\n", index, len,
	       arr[index], arr[index + len - 1]);
	return 0;
}
