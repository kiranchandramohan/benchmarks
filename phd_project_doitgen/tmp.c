

void doitgen(int* restrict A, int* restrict sum, int* restrict C4, int start_indx, int end_indx)
{
        int r, q, p, s ; 

        for (r = 0; r < NP; r++)
                for (q = 0; q < NP; q++)
                        C4[r*NP+q] = ((int) r*q) / NP ;

        for (r = start_indx ; r < end_indx ; r++)
                for (q = 0 ; q < NQ ; q++)
                        for (p = 0; p < NP; p++)
                                A[r*NQ*NP+q*NP+p] = ((int) r*q + p) / NP ;   

        for (r = start_indx ; r < end_indx ; r++) {
                for (q = 0; q < NQ; q++) {
                        for (p = 0; p < NP; p++) {    
                                sum[r*NQ*NP+q*NP+p] = 0;    
                                for (s = 0; s < NP; s++)  {
                                        sum[r*NQ*NP+q*NP+p] = sum[r*NQ*NP+q*NP+p] + 
                                                        A[r*NQ*NP+q*NP+s] * C4[s*NP+p];    
                                }   
                        }   

                        for (p = 0; p < NR; p++) {
                                A[r*NQ*NP+q*NP+p] = sum[r*NQ*NP+q*NP+p] ;
                        }   
                }   
        }   
} 

