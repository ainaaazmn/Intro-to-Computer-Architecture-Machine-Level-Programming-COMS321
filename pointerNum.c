#include <stdio.h>


void increment(int* num){

(*num)++;

    printf("%d\r\n", *num);

}
int main(){

    int num = 0;

    for(int i = 0; i < 11; i++){

        
    increment(&num);

    }
    

    return 0; 
}