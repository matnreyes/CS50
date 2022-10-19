#include "helpers.h"
#include <math.h>
#include <stdio.h>
#include <stdbool.h>

const int Gx[3][3] = 
{
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1},
};

const int Gy[3][3] = 
{
    {-1, -2, -1},
    {0, 0, 0},
    {1, 2, 1},
};

typedef struct
{
    long rgbtRed;
    long rgbtGreen;
    long rgbtBlue;
}
pixel;

// Convert image to grayscale
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    // Cycle through individual pixels finding grey value
    for (int row = 0; row < height; row++)
    {
        for (int column = 0; column < width; column++)
        {
            float greyvalue = (image[row][column].rgbtRed + image[row][column].rgbtGreen + image[row][column].rgbtBlue) / 3.0;
            image[row][column].rgbtRed = round(greyvalue);
            image[row][column].rgbtGreen = round(greyvalue);
            image[row][column].rgbtBlue = round(greyvalue);
        }
    }
    return;
}

// Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    // Make a copy of the original
    RGBTRIPLE imgcopy[height][width];

    for (int row = 0; row < height; row++)
    {
        for (int column = 0; column < width; column++)
        {
            imgcopy[row][column].rgbtRed = image[row][column].rgbtRed;
            imgcopy[row][column].rgbtGreen = image[row][column].rgbtGreen;
            imgcopy[row][column].rgbtBlue = image[row][column].rgbtBlue;
        }
    }

    // Reflect from temp image
    for (int row = 0; row < height; row++)
    {
        for (int column = 0; column < width; column++)
        {
            image[row][width - (column + 1)].rgbtRed = imgcopy[row][column].rgbtRed;
            image[row][width - (column + 1)].rgbtGreen = imgcopy[row][column].rgbtGreen;
            image[row][width - (column + 1)].rgbtBlue = imgcopy[row][column].rgbtBlue;
        }
    }

    return;
}

// Blur image
void blur(int height, int width, RGBTRIPLE image[height][width])
{
    // Make a cache backup of picture
    RGBTRIPLE imgtmp[height][width];
    for (int row = 0; row < height; row++)
    {
        for (int column = 0; column < width; column++)
        {
            imgtmp[row][column].rgbtRed = image[row][column].rgbtRed;
            imgtmp[row][column].rgbtGreen = image[row][column].rgbtGreen;
            imgtmp[row][column].rgbtBlue = image[row][column].rgbtBlue;
        }
    }

    // Blur each pixel across ascending rows
    for (int row = 0; row < height; row++)
    {
        for (int column = 0; column < width; column++)
        {
            float counter;
            counter = 0;
            float avgRed = 0;
            float avgBlue = 0;
            float avgGreen = 0;

            for (int vert = row - 1; vert <= row + 1; vert++)
            {
                for (int horiz = column - 1; horiz <= column + 1; horiz++)
                {
                    // Checks if pixel is within bounds of image
                    bool inBounds = true;
                    if (horiz == -1 || horiz == width)
                    {
                        inBounds = false;
                    }
                    else if (vert == -1 || vert == height)
                    {
                        inBounds = false;
                    }

                    // Adds channel values to one prior for average
                    if (inBounds == true)
                    {
                        counter++;
                        avgBlue += imgtmp[vert][horiz].rgbtBlue;
                        avgGreen += imgtmp[vert][horiz].rgbtGreen;
                        avgRed += imgtmp[vert][horiz].rgbtRed;
                    }
                }
            }


            // Finds the avaraeg and inserts in original image file
            image[row][column].rgbtRed = round(avgRed / counter);
            image[row][column].rgbtGreen = round(avgGreen / counter);
            image[row][column].rgbtBlue = round(avgBlue / counter);
        }
    }

    return;
}

// Detect edges
void edges(int height, int width, RGBTRIPLE image[height][width])
{
    // Initiliazes temporary variables
    RGBTRIPLE imgtmp[height][width];
    pixel GxVal;
    pixel GyVal;
    
    
    // Performs Sobel Operator on every pixel
    for (int row = 0; row < height; row++)
    {
        for (int column = 0; column < width; column++)
        {
            GxVal.rgbtRed = 0;
            GxVal.rgbtGreen = 0;
            GxVal.rgbtBlue = 0;
            
            GyVal.rgbtRed = 0;
            GyVal.rgbtGreen = 0;
            GyVal.rgbtBlue = 0;
            
            // Passes values though kernels to calculate the Gradient
            int arrX = 0;
            for (int x = row - 1; x <= row + 1; x++)
            {
                int arrY = 0;
                for (int y = column - 1; y <= column + 1; y++)
                {
                    bool inBounds = true;
                    if (x < 0 || x == height)
                    {
                        inBounds = false;
                    }
                    else if (y < 0 || y == width)
                    {
                        inBounds = false;
                    }
                    
                    if (inBounds == true)
                    {
                        GxVal.rgbtRed += Gx[arrX][arrY] * image[x][y].rgbtRed;
                        GxVal.rgbtGreen += Gx[arrX][arrY] * image[x][y].rgbtGreen;
                        GxVal.rgbtBlue += Gx[arrX][arrY] * image[x][y].rgbtBlue;
                            
                        GyVal.rgbtRed += Gy[arrX][arrY] * image[x][y].rgbtRed;
                        GyVal.rgbtGreen += Gy[arrX][arrY] * image[x][y].rgbtGreen;
                        GyVal.rgbtBlue += Gy[arrX][arrY] * image[x][y].rgbtBlue;
                                      
                    }
                    arrY++;
                }
                arrX++;
            }
            
            
            // Calculates the values that came out of passing the pixel through kernel
            double GRed = round(sqrt(pow(GxVal.rgbtRed, 2) + pow(GyVal.rgbtRed, 2)));
            double GGreen = round(sqrt(pow(GxVal.rgbtGreen, 2) + pow(GyVal.rgbtGreen, 2)));
            double GBlue = round(sqrt(pow(GxVal.rgbtBlue, 2) + pow(GyVal.rgbtBlue, 2)));
            
            // Caps value at 255
            if (GRed > 255)
            {
                imgtmp[row][column].rgbtRed = 255;
            }
            else
            {
                imgtmp[row][column].rgbtRed = GRed;
            }
                
            if (GGreen > 255)
            {
                imgtmp[row][column].rgbtGreen = 255;
            }
            else
            {
                imgtmp[row][column].rgbtGreen = GGreen;                
            }
    
            if (GBlue > 255)
            {
                imgtmp[row][column].rgbtBlue = 255;
            }
            else
            {
                imgtmp[row][column].rgbtBlue = GBlue;
            }
        }
    }
    
    
    // Adds filter back onto orignal image file
    for (int x = 0; x < height; x++)
    {
        for (int y = 0; y < width; y++)
        {
            image[x][y].rgbtRed = imgtmp[x][y].rgbtRed;
            image[x][y].rgbtGreen = imgtmp[x][y].rgbtGreen;
            image[x][y].rgbtBlue = imgtmp[x][y].rgbtBlue;
        }
    }
    return;
}
