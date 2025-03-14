#include <stdio.h>
#include <stdlib.h>

/**
 * Returns the start index of the longest subarray of non-decreasing numbers.
 * `length` parameter is updated on function return to denote the length
 * of the subarray starting from the returned index.
 *
 * On error, -1 is returned
 */
int longest_non_decreasing_subarray(int arr[], int n_elem, int *length)
{
	if (n_elem <= 0)
		return -1;

	int start_index = 0;
	int len = 1;

	int curr_len = 1;
	for (int i = 1; i < n_elem; i++, curr_len++) {
		if (arr[i] < arr[i - 1]) {
			if (curr_len > len) {
				len = curr_len;
				start_index = i - curr_len;
			}
			curr_len = 0;
		}
	}
	if (curr_len > len) {
		len = curr_len;
		start_index = n_elem - curr_len;
	}
	*length = len;
	return start_index;
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
	index = longest_non_decreasing_subarray(arr, argc - 1, &len);
	printf("Longest subarray index = %d, length = %d (%d - %d)\n", index, len,
	       arr[index], arr[index + len - 1]);
	return 0;
}
