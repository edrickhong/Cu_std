#include "print.h"
#include "stdio.h"


void Print(f64 a){
	printf("%f",a);
}


void Print(f32 a){
	printf("%f",(f64)a);
}

void Print(s32 a){
	printf("%d",a);
};

void Print(u32 a){
	printf("%d",a);
}

void Print(const s8* s){
	printf(s);
}
