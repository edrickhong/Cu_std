#pragma once

//Never use this please
void BubbleSort();

void RadixSort();

//does a binary search in a sorted array
s8* SortedArraySearch(s8* _restrict array,u32 elem_size,s8* _restrict key,b32 (*cmp_op)(s8* _restrict,s8* _restrict));