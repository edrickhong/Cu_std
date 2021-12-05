#include "pprint.h"
#include "stdio.h"


void PPrint(f64 a){
	printf("%f",a);
}


void PPrint(f32 a){
	printf("%f",(f64)a);
}

void PPrint(s32 a){
	printf("%d",a);
};

void PPrint(u32 a){
	printf("%d",a);
}

void PPrint(const s8* s){
	printf(s);
}
