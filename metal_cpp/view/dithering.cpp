//
//  dithering.cpp
//  metal_cpp
//
//  Created by Neel on 04/03/26.
//

// not used anymore

#include "view/dithering.h"
#define ASSERT_IDX(i,j,k) \
    assert((i) >= 0 && (i) < 4 && "i out of bounds"); \
    assert((j) >= 0 && (j) < 8 && "j out of bounds"); \
    assert((k) >= 0 && (k) < 8 && "k out of bounds");

simd::float4x4 Bayer4x4(){
    simd::float4 col0 = { 0, 12,  3, 15};
    simd::float4 col1 = { 8,  4, 11,  7};
    simd::float4 col2 = { 2, 14,  1, 13};
    simd::float4 col3 = {10,  6,  9,  5};
    return simd_matrix(col0, col1, col2, col3);
}


void Bayer4x4Transitions(float (&Transitions)[4][8][8]){

    simd::float4x4 Bayer = Bayer4x4();

    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            for(int ki = 0; ki < 2; ki++){
                for(int kj = 0; kj < 2; kj++){

                    int r = 2*i + ki;
                    int c = 2*j + kj;

                    ASSERT_IDX(0,r,c);
                    Transitions[0][r][c] = Bayer.columns[j][i];
                }
            }
        }
    }

    for(int i = 1; i < 4; i++){

        for(int di = 0; di < i; di++){
            for(int dj = 0; dj < i; dj++){

                ASSERT_IDX(i,di,dj);
                ASSERT_IDX(i-1,di,dj);
                Transitions[i][di][dj] = Transitions[i-1][di][dj];

                ASSERT_IDX(i,7-di,dj);
                ASSERT_IDX(i-1,7-di,dj);
                Transitions[i][7-di][dj] = Transitions[i-1][7-di][dj];

                ASSERT_IDX(i,di,7-dj);
                ASSERT_IDX(i-1,di,7-dj);
                Transitions[i][di][7-dj] = Transitions[i-1][di][7-dj];

                ASSERT_IDX(i,7-di,7-dj);
                ASSERT_IDX(i-1,7-di,7-dj);
                Transitions[i][7-di][7-dj] = Transitions[i-1][7-di][7-dj];
            }
        }

        for(int r = 0; r < i; r++){
            for(int x = i; x <= 7-i; x += 2){

                assert(x+1 < 8);

                ASSERT_IDX(i,r,x);
                ASSERT_IDX(i-1,r,x+1);
                Transitions[i][r][x] = Transitions[i-1][r][x+1];

                ASSERT_IDX(i,r,x+1);
                ASSERT_IDX(i-1,r,x);
                Transitions[i][r][x+1] = Transitions[i-1][r][x];

                ASSERT_IDX(i,7-r,x);
                ASSERT_IDX(i-1,7-r,x+1);
                Transitions[i][7-r][x] = Transitions[i-1][7-r][x+1];

                ASSERT_IDX(i,7-r,x+1);
                ASSERT_IDX(i-1,7-r,x);
                Transitions[i][7-r][x+1] = Transitions[i-1][7-r][x];

                ASSERT_IDX(i,x,r);
                ASSERT_IDX(i-1,x+1,r);
                Transitions[i][x][r] = Transitions[i-1][x+1][r];

                ASSERT_IDX(i,x+1,r);
                ASSERT_IDX(i-1,x,r);
                Transitions[i][x+1][r] = Transitions[i-1][x][r];

                ASSERT_IDX(i,x,7-r);
                ASSERT_IDX(i-1,x+1,7-r);
                Transitions[i][x][7-r] = Transitions[i-1][x+1][7-r];

                ASSERT_IDX(i,x+1,7-r);
                ASSERT_IDX(i-1,x,7-r);
                Transitions[i][x+1][7-r] = Transitions[i-1][x][7-r];
            }
        }

        for(int ri = i; ri < 7-i; ri += 2){
            for(int rj = i; rj < 7-i; rj += 2){

                assert(ri+1 < 8);
                assert(rj+1 < 8);

                ASSERT_IDX(i,ri,rj);
                ASSERT_IDX(i-1,ri+1,rj+1);
                Transitions[i][ri][rj] = Transitions[i-1][ri+1][rj+1];

                ASSERT_IDX(i,ri+1,rj);
                ASSERT_IDX(i-1,ri,rj+1);
                Transitions[i][ri+1][rj] = Transitions[i-1][ri][rj+1];

                ASSERT_IDX(i,ri,rj+1);
                ASSERT_IDX(i-1,ri+1,rj);
                Transitions[i][ri][rj+1] = Transitions[i-1][ri+1][rj];

                ASSERT_IDX(i,ri+1,rj+1);
                ASSERT_IDX(i-1,ri,rj);
                Transitions[i][ri+1][rj+1] = Transitions[i-1][ri][rj];
            }
        }
    }
}

