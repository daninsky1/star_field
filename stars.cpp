#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <iostream>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


union ABGRColor {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    uint32_t c;
}; // FF FF FF FF - AA BB GG RR 

/*  Original QuickBasic types
 *  variable%: integer variable -32'768 - 32'767 
 *  variable&: integer variable -2'147'483'648 and -2'147'483'647
 *  variable!: single-precision floating-point
 *  variable: variable with no sufix is a single-precision floating-point variable
 *  variable#: double-precision floating-point variable
 *  variable$: string of characters variable
 * */
const int pal_size = 253;
float pal[pal_size][3];    // palette
float palG[pal_size][3];   // gamma correct palette?

const float gamma = 2.2f, ungamma = 1.0f / gamma;

// Define an optimal palette (generated with NeuQuant). The colors are sorted
// by luminosity (for reasons that become apparent later).
// NOTE(daniel): The monst significant byte has to be 0 to not interfere with
// the 3 color integer value
// NOTE(daniel): 0x FF FF FF FF - 0x PP RR GG BB. PP: padding
uint32_t quanti_colors[pal_size] {
    0x00000000,0x00000001,0x00010101,0x00010102,0x00020101,0x00010201,0x00010202,0x00030102,0x00020202,0x00020203,
    0x00030201,0x00040104,0x00020205,0x00030302,0x00010404,0x00020402,0x00060202,0x00030305,0x00050206,0x00040304,
    0x00030309,0x00040402,0x00020505,0x00050404,0x00040407,0x00080303,0x00050502,0x0006030A,0x00030603,0x00070405,
    0x0004040E,0x00090307,0x00040605,0x00020707,0x00060508,0x00060605,0x00060703,0x000B030B,0x000C0404,0x0004070A,
    0x00030903,0x00070707,0x00090609,0x00050614,0x00090705,0x0008060D,0x000B0411,0x0004090D,0x00060A06,0x00070909,
    0x000F0509,0x0009080A,0x00040B09,0x000B0807,0x0010040F,0x00090B03,0x00090B07,0x000D080C,0x00040E05,0x0007081C,
    0x000F0517,0x00150507,0x000F0904,0x00090A10,0x00080C0C,0x00050D0F,0x000E0B08,0x000B0C0B,0x000F0910,0x0011090B,
    0x000C0A14,0x00080F09,0x0014051E,0x00140810,0x000A0928,0x000C0F05,0x00080E16,0x000E0D0D,0x0005120B,0x00180617,
    0x0009100E,0x000C0E11,0x00130A16,0x001C070E,0x000E0C1A,0x00140D06,0x00061212,0x00100F0A,0x00120D11,0x00081505,
    0x000F100F,0x001C0B07,0x00150E0C,0x00110F14,0x000C140A,0x00110D21,0x00101305,0x00081220,0x001F0818,0x00180D11,
    0x000F0B36,0x001B0826,0x000D1315,0x000B1512,0x00081619,0x00141210,0x00190D1C,0x00260A0C,0x00161017,0x0009132D,
    0x00181306,0x0013150A,0x00101610,0x000A1A0D,0x001E100E,0x0011141C,0x00051C12,0x00200930,0x00260921,0x00131515,
    0x00170F2C,0x000F1B07,0x0019150D,0x001A1316,0x000E1724,0x00220F1A,0x000E1A18,0x002D0B16,0x00171423,0x00082108,
    0x00171813,0x00131B0F,0x000A1D1D,0x00141919,0x002C0A2B,0x0019171C,0x00251313,0x00231608,0x001F141F,0x00191C07,
    0x00231127,0x00081F26,0x000D2110,0x00131B1F,0x00380C10,0x001D1A10,0x002A0B3C,0x002E130A,0x001F1817,0x000E1A3C,
    0x00171F10,0x00121C2D,0x001D172B,0x00191D17,0x001E143B,0x00122118,0x00072817,0x00132508,0x002E131F,0x00102222,
    0x001E1C1D,0x001B1C25,0x003A0D23,0x0025191E,0x00271531,0x00380D33,0x000B2D09,0x0019221F,0x001F2211,0x002F1915,
    0x00271A26,0x001F2408,0x00291E0D,0x000C282A,0x00261F16,0x00172329,0x001F1E31,0x00321629,0x00182814,0x0014291F,
    0x00232122,0x003C1813,0x0020251A,0x00112738,0x0020222A,0x00112E15,0x00083121,0x003D113E,0x00231F3B,0x0030183E,
    0x002C1E2B,0x003D1820,0x002E201F,0x001A2930,0x001B300A,0x001F2A24,0x00242B13,0x003E192E,0x0028281C,0x00262531,
    0x0038230C,0x000B3238,0x002F2714,0x001F283B,0x0015302B,0x001C301C,0x00312135,0x00292827,0x00322428,0x002A2E09,
    0x00103C0C,0x000F3B1D,0x002D263D,0x00232F2E,0x003E2327,0x0017333B,0x000B3D2A,0x003D271A,0x00193B16,0x003E213D,
    0x00302E1F,0x00263422,0x002B3416,0x002F2D30,0x00223C0C,0x003B2832,0x003D2E0E,0x0028303C,0x001D3C22,0x001A3B30,
    0x00123E3D,0x0030332A,0x003E2A3E,0x003A331B,0x0033303D,0x00273834,0x003D3026,0x00293D1B,0x00303C0F,0x00263D29,
    0x00213D3D,0x003A3433,0x00323D24,0x003D3D0E,0x00323D31,0x002B3F3E,0x003F363F,0x003D3F1A,0x003F3E23,0x00353E3F,
    0x003E3E2D,0x003E3E36,0x003F3F3F
};

inline float rand_float()
{
    return 1.0f / (float)RAND_MAX * (float)rand();
}

inline float clamp(float v) {
    if (v < 0.0f) return 0.0f;
    else if (v > 1.0f) return 1.0f;
    else return v;
}

inline float min(float a, float b) {
    if (a < b) return a;
    return b;
}

int main()
{
    uint32_t color_table[pal_size];

    for (int p = 0; p < pal_size; ++p) {
        const char *string_data;    // TODO: read data
        //int s = quanti_colors[p];
        // NOTE(daniel): I've never seen this trick to get a value byte in any
        // integer position, this should work only with unsigned integers or
        // with some trickery with signed integers, however I've changed to
        // use a more simple approach with casting
        //int R = s / 65536;
        //int G = (s / 256) & UINT8_MAX;
        //int B = s / UINT8_MAX;
        uint8_t *c = (uint8_t*)&quanti_colors[p];
        uint8_t R = *(c+2); uint8_t G = *(c+1); uint8_t B = *(c);

        ABGRColor col;
        col.b = B;
        col.g = G;
        col.r = R;
        col.a = 0xFF;

        color_table[p] = col.c;
        //printf("%d, %x\n", p, color_table[p]);
        
        pal[p][0] = ((float)R / 63.0f); palG[p][0] = powf(pal[p][0], gamma);
        pal[p][1] = ((float)G / 63.0f); palG[p][1] = powf(pal[p][1], gamma);
        pal[p][2] = ((float)B / 63.0f); palG[p][2] = powf(pal[p][2], gamma);

        //printf("%f, %f, %f", palG[p][0], palG[p][1], palG[p][2]);
        //std::cin.ignore();

        //color_table[p] = col.c;
        //printf("red: %x\n", R);
        //printf("red: %f\n", pal[p][0]);
        //printf("green: %x\n", G);
        //printf("green: %f\n", pal[p][1]);
        //printf("blue: %x\n", B);
        //printf("blue: %f\n", pal[p][2]);
        //printf("color: %x\n", col.c);
        //std::cin.ignore();
    }
    //std::cin.ignore();

    const int cand_count = 64;     // color candidate count
    int cand_list[cand_count];
    
    // Set up a 8x8 bayer-dithering matrix
    int dither8x8[8][8];
    int q, p;
    for (int y = 0; y < 8; ++y) {
        // READY !!! ===========================================================
        for (int x = 0; x < 8; ++x) {
            q = x ^ y;
            p = (x & 4) / 4 + (x & 2) * 2 + (x & 1) * 16;
            q = (q & 4) / 2 + (q & 2) * 4 + (q & 1) * 32;
            // NOTE(daniel): This 64 maybe has connection to the cand_count
            dither8x8[y][x] = (p + q) * cand_count / 64;
        }
    }

    // Generate stars
    const int N = 25000;
    std::vector<int> starx(N), stary(N), starz(N);
    std::vector<float> starR(N), starG(N), starB(N);
    for (int c = 0; c < N; ++c) {
        // READY !!! ===========================================================
        // Random RGB color            Random 3D position
        starR[c] = rand_float() + 0.01f; starx[c] = lrintf(rand_float() * 1000.0f) - 500;
        starG[c] = rand_float() + 0.01f; stary[c] = lrintf(rand_float() * 1000.0f) - 500;
        starB[c] = rand_float() + 0.01f; starz[c] = lrintf(rand_float() * 400.0f) + 1;

        // normalize hue (maximize brightness)
        float maxhue = starR[c];
        if (starG[c] > maxhue) maxhue = starG[c];
        if (starB[c] > maxhue) maxhue = starB[c];
        starR[c] *= 1.0f / maxhue;
        starG[c] *= 1.0f / maxhue;
        starB[c] *= 1.0f / maxhue;
    }
    // READY !!! ===============================================================
    // Main loop
    std::vector<float> px(N), py(N), radius(N), radsquared(N), szfactor(N);
    // Blue buffer
    std::vector<std::vector<float>> blur(64'000, std::vector<float>(3));
    float ambient = 0.05f / sqrtf((float)N);

    const unsigned W = 320, H = 200;

    int frames = 1000;
    for (int f = 0; f < frames; ++f) {
        fprintf(stderr, "Begins frame %d\n", f);

        const unsigned channels = 4;
        size_t imsz = W*H*channels;
        uint8_t *im = (uint8_t*)malloc(imsz);
        uint32_t *pixel = (uint32_t*)im;

        // Move each star
        for (int c = 0; c < N; ++c) {
        // READY !!! ===========================================================
            int newz = starz[c] - 2;
            if (newz <= 1) {
rerandomize:
                newz = 1000 - (int)(rand_float() * 10.0f);
                starx[c] = lrintf(rand_float() * 1000.0f) - 500;
                stary[c] = lrintf(rand_float() * 1000.0f) - 500;
            }
            starz[c] = newz;
            // Do perspective transformation
            px[c] = (float)starx[c] * 200.0f / (float)starz[c];
            py[c] = (float)stary[c] * 180.0f / (float)starz[c];
            radius[c] = 900.0f / (float)(starz[c] - 1);
            if ((fabsf(px[c]) + radius[c]) > 230.0f) goto rerandomize;
            if ((fabsf(py[c]) + radius[c]) > 180.0f) goto rerandomize;
            radsquared[c] = radius[c] * radius[c];
            szfactor[c] = (1.0f - (float)starz[c] / 400.0f);
            if ((szfactor[c]) < 0.0f) szfactor[c] = 0;
            else szfactor[c] *= szfactor[c];
        }

        // Render each pixel
        int cc = 0;
        for (int y = -99; y <= 100; ++y) {
            for (int x = -159; x <= 160; ++x) {
                float R = blur[cc][0];
                float G = blur[cc][1];
                float B = blur[cc][2];
                //printf("%f, %f, %f\n", R, G, B);
                //std::cin.ignore();
//
// READY !!! ==================================================================
//
                for (int c = 0; c < N; ++c) {
                // READY !!! ===================================================
                    float distx = (float)x - px[c];
                    float disty = (float)y - py[c];
                    //printf("%f, %f", distx, disty);
                    //std::cin.ignore();
                    float distsquared = distx * distx + disty * disty;
                    if (distsquared < radsquared[c]) {
                        float distance = sqrtf(distsquared);
                        //printf("%f", distsquared);
                        //std::cin.ignore();
                        float scaleddist = distance / radius[c];
                        float sz = (1.0f - sqrtf(scaleddist)) * szfactor[c];
                        R += starR[c] * sz + ambient;
                        G += starG[c] * sz + ambient;
                        B += starB[c] * sz + ambient;
                    }
                    //printf("%f, %f, %f\n", R, G, B);
                }
                //printf("%f, %f, %f\n", R, G, B);
                //std::cin.ignore();
                // Save the color for motion blur (fade it a little)
                // READY !!! ===================================================
                blur[cc][0] = R * 0.83f;
                blur[cc][1] = G * 0.83f;
                blur[cc][2] = B * 0.83f;
                // Leak (some of) possible excess brightness to other color channels
                // NOTE: This algorithm was fixed and improved after the Youtube video
                float luma = R * 0.299f + G * 0.298f + B * 0.114f;
                float sat = 1.0f;
                if (luma >= 1.0f) {
                    R = 1.0f; G = 1.0f; B = 1.0f;
                }
                else if (luma <= 0.0f) {
                    R = 0.0f; G = 0.0f; B = 0.0f;
                }
                else {
                    if (R > 1.0f) {
                        sat = min(sat, (luma - 1.0f)/ (luma - R));
                    }
                    else if (R < 0.0f) {
                        sat = min(sat, luma / (luma - R));
                    }
                    if (G > 1.0f) {
                        sat = min(sat, (luma - 1.0f)/ (luma - G));
                    }
                    else if (G < 0.0f) {
                        sat = min(sat, luma / (luma - G));
                    }
                    if (B > 1.0f) {
                        sat = min(sat, (luma - 1.0f)/ (luma - B));
                    }
                    else if (B < 0.0f) {
                        sat = min(sat, luma / (luma - B));
                    }
                    if (sat < 1.0f) {
                        R = (R - luma) * sat + luma;
                        G = (G - luma) * sat + luma;
                        B = (B - luma) * sat + luma;
                    }
                }
                //printf("%f, %f, %f", blur[cc][0], blur[cc][1], blur[cc][2]);
                //printf("%f, %f", luma, sat);
                //printf("%f, %f, %f, %f, %f", R, G, B, luma, sat);
                //std::cin.ignore();
                // Quantize (use gamma-aware Knoll-Yliluoma positional dithering)
                float errorR = 0, gammaR = powf(R, gamma);
                float errorG = 0, gammaG = powf(G, gamma);
                float errorB = 0, gammaB = powf(B, gamma);
                // Create color candidate table
                for (int c = 0; c < cand_count; ++c) {
                    float tryR = powf(clamp(gammaR + errorR), ungamma);
                    float tryG = powf(clamp(gammaG + errorG), ungamma);
                    float tryB = powf(clamp(gammaB + errorB), ungamma);
                    //printf("%f, %f, %f\n", tryR, tryG, tryB);
                    //std::cin.ignore();

                    // Find out which palette color is the best match
                    int chosen = 0;
                    float best = 0.0f;
                    for (int p = 0; p < 252; ++p) {
                        float eR = pal[p][0] - tryR;
                        float eG = pal[p][1] - tryG;
                        float eB = pal[p][2] - tryB;
                        float test = eR * eR + eG * eG + eB * eB;
                        if ((p == 0) || (test < best)) {
                            best = test; chosen = p;
                        }
                        //printf("%f, %f, %f\n", eR, eG, eB);
                        //printf("%f", test);
                        //std::cin.ignore();
                    }
                    cand_list[c] = chosen;
                    // Find out how much it differs from the desired value
                    errorR = gammaR - palG[chosen][0];
                    errorG = gammaG - palG[chosen][1];
                    errorB = gammaB - palG[chosen][2];
                    //printf("%f, %f, %f\n", gammaR, gammaG, gammaB);
                    //printf("%f, %f, %f", errorR, errorG, errorB);
                    //printf("%d\n", chosen);
                    //printf("%f, %f, %f", palG[chosen][0], palG[chosen][1], palG[chosen][2]);
                    //std::cin.ignore();
                }
                // Sort the color candidate table by luma.
                // Since palette colors are already sorted by luma, we can
                // simply sort by palette indices.
                // Use insertion sort. (A bug was fixed here after publication.)
                for (int j = 1; j < cand_count; ++j) {
                    unsigned k = cand_list[j];
                    int i = j;
                    for (; i >= 1; --i) {
                        if (cand_list[i] <= k) break;
                        cand_list[i] = cand_list[i - 1];
                    }
                    cand_list[i] = k;
                }
                // Plot the pixel to the screen
                // NOTE: Double-buffering was removed for QB64 because it does
                // not support the assembler function.
                uint32_t color = color_table[cand_list[dither8x8[x & 7][y & 7]]];
                
                //printf("(%d, %d), cand_i:%d, cand_v:%d, c:%#010x", x, y, dither8x8[x&7][x&7], cand_list[dither8x8[x&7][x&7]], color);
                //std::cin.ignore();
                *(pixel+(x+159)+((y+99)*W)) = color;
                cc++;
            }
        }

        char buf[64]; sprintf(buf, "trace%04d.jpg", f);
        fprintf(stderr, "Writing %s...\n", buf);
        stbi_write_jpg(buf, W, H, channels, im, 100);
        free(im);

        std::cin.ignore();
    }
}
