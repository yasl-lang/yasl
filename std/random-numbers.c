/*                                    
   This example first calls &srand. with a value other than 1 to                
   initiate the random value sequence.                                          
   Then the program computes 5 random values for the array of                   
   integers called ranvals.                                                     
   If you repeat this code exactly, then the same sequence of                   
   random values will be generated.                                             
                                                                                
 */                                                                             
#include <stdlib.h>                                                             
#include <stdio.h>                                                              
                                                                                
int main(void)                                                                  
{                                                                               
   int i, ranvals[5];                                                           
                                                                                
   srand(17);                                                                   
   for (i = 0; i < 5; i++)                                                      
   {                                                                            
      ranvals[i] = rand();                                                      
      printf("Iteration %d ranvals [%d] = %d\n", i+1, i, ranvals[i]);           
   }                                                                            
}                                                                               