#include <iostream>
#include <vector>
#include <algorithm> // For std::swap

using namespace std;

// Bubble Sort
void bubbleSort(vector<int>& arr) {
    int n = arr.size();
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            if (arr[j] > arr[j + 1]) {
                swap(arr[j], arr[j + 1]);
            }
        }
    }
}

// Selection Sort
void selectionSort(vector<int>& arr) {
    int n = arr.size();
    for (int i = 0; i < n - 1; ++i) {
        int minIndex = i;
        for (int j = i + 1; j < n; ++j) {
            if (arr[j] < arr[minIndex]) {
                minIndex = j;
            }
        }
        swap(arr[i], arr[minIndex]);
    }
}

// Insertion Sort
void insertionSort(vector<int>& arr) {
    int n = arr.size();
    for (int i = 1; i < n; ++i) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = key;
    }
}

// Merge Sort
void merge(vector<int>& arr, int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;
    vector<int> L(n1), R(n2);
    for (int i = 0; i < n1; ++i) {
        L[i] = arr[l + i];
    }
    for (int j = 0; j < n2; ++j) {
        R[j] = arr[m + 1 + j];
    }
    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            ++i;
        } else {
            arr[k] = R[j];
            ++j;
        }
        ++k;
    }
    while (i < n1) {
        arr[k] = L[i];
        ++i;
        ++k;
    }
    while (j < n2) {
        arr[k] = R[j];
        ++j;
        ++k;
    }
}

void mergeSortHelper(vector<int>& arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSortHelper(arr, l, m);
        mergeSortHelper(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

void mergeSort(vector<int>& arr) {
    mergeSortHelper(arr, 0, arr.size() - 1);
}

// Quick Sort
int partition(vector<int>& arr, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (arr[j] <= pivot) {
            ++i;
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[high]);
    return i + 1;
}

void quickSortHelper(vector<int>& arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSortHelper(arr, low, pi - 1);
        quickSortHelper(arr, pi + 1, high);
    }
}

void quickSort(vector<int>& arr) {
    quickSortHelper(arr, 0, arr.size() - 1);
}

// Heap Sort
void heapify(vector<int>& arr, int n, int i) {
    int largest = i;
    int l = 2 * i + 1;
    int r = 2 * i + 2;
    if (l < n && arr[l] > arr[largest]) {
        largest = l;
    }
    if (r < n && arr[r] > arr[largest]) {
        largest = r;
    }
    if (largest != i) {
        swap(arr[i], arr[largest]);
        heapify(arr, n, largest);
    }
}

void heapSort(vector<int>& arr) {
    int n = arr.size();
    for (int i = n / 2 - 1; i >= 0; --i) {
        heapify(arr, n, i);
    }
    for (int i = n - 1; i >= 0; --i) {
        swap(arr[0], arr[i]);
        heapify(arr, i, 0);
    }
}

// Radix Sort
int getMax(vector<int>& arr) {
    int max = arr[0];
    for (int i = 1; i < arr.size(); ++i) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return max;
}

void countSort(vector<int>& arr, int exp) {
    vector<int> output(arr.size()), count(10, 0);
    for (int i = 0; i < arr.size(); ++i) {
        count[(arr[i] / exp) % 10]++;
    }
    for (int i = 1; i < 10; ++i) {
        count[i] += count[i - 1];
    }
    for (int i = arr.size() - 1; i >= 0; --i) {
        output[count[(arr[i] / exp) % 10] - 1] = arr[i];
        count[(arr[i] / exp) % 10]--;
    }
    for (int i = 0; i < arr.size(); ++i) {
        arr[i] = output[i];
    }
}

void radixSort(vector<int>& arr) {
    int max = getMax(arr);
    for (int exp = 1; max / exp > 0; exp *= 10) {
        countSort(arr, exp);
    }
}

// Bucket Sort
void bucketSort(vector<float>& arr) {
    int n = arr.size();
    vector<vector<float>> buckets(n);
    for (int i = 0; i < n; ++i) {
        int bucketIndex = n * arr[i];
        buckets[bucketIndex].push_back(arr[i]);
    }
    for (int i = 0; i < n; ++i) {
        sort(buckets[i].begin(), buckets[i].end());
    }
    int index = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < buckets[i].size(); ++j) {
            arr[index++] = buckets[i][j];
        }
    }
}

// Counting Sort
void countingSort(vector<int>& arr) {
    int max = *max_element(arr.begin(), arr.end());
    vector<int> count(max + 1, 0);
    vector<int> output(arr.size());
    for (int i = 0; i < arr.size(); ++i) {
        count[arr[i]]++;
    }
    for (int i = 1; i <= max; ++i) {
        count[i] += count[i - 1];
    }
    for (int i = arr.size() - 1; i >= 0; --i) {
        output[count[arr[i]] - 1] = arr[i];
        count[arr[i]]--;
    }
    for (int i = 0; i < arr.size(); ++i) {
        arr[i] = output[i];
    }
}

// Cocktail Shaker Sort
void cocktailShakerSort(vector<int>& arr) {
    int n = arr.size();
    bool swapped = true;
    int start = 0;
    int end = n - 1;
    while (swapped) {
        swapped = false;
        for (int i = start; i < end; ++i) {
            if (arr[i] > arr[i + 1]) {
                swap(arr[i], arr[i + 1]);
                swapped = true;
            }
        }
        if (!swapped) {
            break;
        }
        swapped = false;
        --end;
        for (int i = end - 1; i >= start; --i) {
            if (arr[i] > arr[i + 1]) {
                swap(arr[i], arr[i + 1]);
                swapped = true;
            }
        }
        ++start;
    }
}

// Shell Sort
void shellSort(vector<int>& arr) {
    int n = arr.size();
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; ++i) {
            int temp = arr[i];
            int j;
            for (j = i; j >= gap && arr[j - gap] > temp; j -= gap) {
                arr[j] = arr[j - gap];
            }
            arr[j] = temp;
        }
    }
}

// Comb Sort
void combSort(vector<int>& arr) {
    int n = arr.size();
    int gap = n;
    bool swapped = true;
    while (gap > 1 || swapped) {
        if (gap > 1) {
            gap = int(gap / 1.3);
        }
        swapped = false;
        for (int i = 0; i + gap < n; ++i) {
            if (arr[i] > arr[i + gap]) {
                swap(arr[i], arr[i + gap]);
                swapped = true;
            }
        }
    }
}

// Cycle Sort
void cycleSort(vector<int>& arr) {
    int n = arr.size();
    for (int start = 0; start < n - 1; ++start) {
        int item = arr[start];
        int pos = start;
        for (int i = start + 1; i < n; ++i) {
            if (arr[i] < item) {
                ++pos;
            }
        }
        if (pos == start) {
            continue;
        }
        while (item == arr[pos]) {
            ++pos;
        }
        if (pos != start) {
            swap(item, arr[pos]);
        }
        while (pos != start) {
            pos = start;
            for (int i = start + 1; i < n; ++i) {
                if (arr[i] < item) {
                    ++pos;
                }
            }
            while (item == arr[pos]) {
                ++pos;
            }
            if (item != arr[pos]) {
                swap(item, arr[pos]);
            }
        }
    }
}

// Gnome Sort
void gnomeSort(vector<int>& arr) {
    int n = arr.size();
    int index = 0;
    while (index < n) {
        if (index == 0) {
            ++index;
        }
        if (arr[index] >= arr[index - 1]) {
            ++index;
        } else {
            swap(arr[index], arr[index - 1]);
            --index;
        }
    }
}

// Pancake Sort
int findMaxIndex(vector<int>& arr, int n) {
    int maxIndex = 0;
    for (int i = 0; i < n; ++i) {
        if (arr[i] > arr[maxIndex]) {
            maxIndex = i;
        }
    }
    return maxIndex;
}

void flip(vector<int>& arr, int i) {
    int start = 0;
    while (start < i) {
        swap(arr[start], arr[i]);
        ++start;
        --i;
    }
}

void pancakeSort(vector<int>& arr) {
    int n = arr.size();
    for (int currSize = n; currSize > 1; --currSize) {
        int maxIndex = findMaxIndex(arr, currSize);
        if (maxIndex != currSize - 1) {
            flip(arr, maxIndex);
            flip(arr, currSize - 1);
        }
    }
}

// Stooge Sort
void stoogeSortHelper(vector<int>& arr, int l, int h) {
    if (l >= h) {
        return;
    }
    if (arr[l] > arr[h]) {
        swap(arr[l], arr[h]);
    }
    if (h - l + 1 > 2) {
        int t = (h - l + 1) / 3;
        stoogeSortHelper(arr, l, h - t);
        stoogeSortHelper(arr, l + t, h);
        stoogeSortHelper(arr, l, h - t);
    }
}

void stoogeSort(vector<int>& arr) {
    int n = arr.size();
    stoogeSortHelper(arr, 0, n - 1);
}

// Odd-Even Sort
void oddEvenSort(vector<int>& arr) {
    int n = arr.size();
    bool isSorted = false;
    while (!isSorted) {
        isSorted = true;
        for (int i = 1; i <= n - 2; i += 2) {
            if (arr[i] > arr[i + 1]) {
                swap(arr[i], arr[i + 1]);
                isSorted = false;
            }
        }
        for (int i = 0; i <= n - 2; i += 2) {
            if (arr[i] > arr[i + 1]) {
                swap(arr[i], arr[i + 1]);
                isSorted = false;
            }
        }
    }
}

// Bitonic Sort
void bitonicMerge(vector<int>& arr, int low, int cnt, bool dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        for (int i = low; i < low + k; ++i) {
            if ((arr[i] > arr[i + k]) == dir) {
                swap(arr[i], arr[i + k]);
            }
