// void xsort_inplace(const int *input, int count) 
// {
//     int arr[count];
//     for (int i = 0; i < count; ++i)
//         arr[i] = input[i];
//     for (int i = 1; i < count; ++i) {
//         int key = arr[i];
//         int j = i - 1;
//         while (j >= 0 && arr[j] > key) {
//             arr[j + 1] = arr[j];
//             j = j - 1;
//         }
//         arr[j + 1] = key;
//     }
//     for (int i = 0; i < count; ++i)
//         printf("%d ", arr[i]);
//     putchar('\n');
// }

// int partition(uint16_t arr[], int low, int high) 
// {
//     int pivot = arr[high]; 
//     int i = (low - 1); 
//     for (int j = low; j <= high - 1; j++) {
//         if (arr[j] < pivot) {
//             i++;
//             std::swap(arr[i], arr[j]);
//         }
//     }
//     std::swap(arr[i + 1], arr[high]);
//     return (i + 1);
// }

// void quicksort(uint16_t arr[], int low, int high) 
// {
//     if (low < high) {
//         uint16_t pi = partition(arr, low, high);
//         quicksort(arr, low, pi - 1);
//         quicksort(arr, pi + 1, high);
//     }
// }

// void xsort_quick(const int *input, int count) 
// {
//     uint16_t arr[count];
//     for (int i = 0; i < count; ++i)
//         arr[i] = (uint16_t) input[i];
//     quicksort(arr, 0, count - 1);
//     for (int i = 0; i < count; ++i)
//         printf("%d ", arr[i]);
//     putchar('\n');
// }

// void counting_sort_16bit(uint16_t* arr, int count, int shift) 
// {
//     uint32_t count_array[256] = {0};
//     uint16_t output[count];

//     for (int i = 0; i < count; i++) {
//         uint16_t digit = (arr[i] >> shift) & 0xFF;
//         count_array[digit]++;
//     }
//     for (int i = 1; i < 256; ++i)
//         count_array[i] += count_array[i - 1];
//     for (int i = count - 1; i >= 0; i--) {
//         uint16_t digit = (arr[i] >> shift) & 0xFF;
//         output[count_array[digit] - 1] = arr[i];
//         count_array[digit]--;
//     }
//     for (int i = 0; i < count; i++)
//         arr[i] = output[i];
// }

// void xsort_radix(const int *input, int count) 
// {
//     uint16_t arr[count];
//     for (int i = 0; i < count; i++)
//         arr[i] = (uint16_t) input[i];
//     counting_sort_16bit(arr, count, 0);
//     counting_sort_16bit(arr, count, 8);
//     for (int i = 0; i < count; i++)
//         printf("%u ", arr[i]);
//     putchar('\n');
// }
