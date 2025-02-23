#include <iostream>
#include <opencv2/opencv.hpp>
#include <bitset>
#include <fstream>

using namespace std;
using namespace cv;

// Function to encode a single pixel value using Golomb coding
string golombEncode(int num, int M) {
    int quotient = num / M;   // q = num / M
    int remainder = num % M;  // r = num % M

    // Unary encoding for quotient (q)
    string unaryPart = string(quotient, '1') + "0";

    // Binary encoding for remainder (r)
    int b = ceil(log2(M));
    string binaryPart = bitset<8>(remainder).to_string().substr(8 - b);

    return unaryPart + binaryPart;
}


int golombDecode(const string& code, int M, int& index) {
    int quotient = 0;

    // Read unary part (count consecutive 1s before first 0)
    while (index < code.length() && code[index] == '1') {
        quotient++;
        index++;
    }

    index++;

    int b = ceil(log2(M));
    string binaryPart = code.substr(index, b);
    int remainder = stoi(binaryPart, nullptr, 2);
    index += b;

    return quotient * M + remainder;
}

// Function to compress an image using Golomb coding
void compressImage(const string& imagePath, int M) {
    Mat img = imread(imagePath, IMREAD_GRAYSCALE);
    if (img.empty()) {
        cout << "Error loading image!" << endl;
        return;
    }

    ofstream outFile("compressed_golomb.bin", ios::binary);
    if (!outFile) {
        cout << "Error opening output file!" << endl;
        return;
    }

    int rows = img.rows, cols = img.cols;
    outFile.write(reinterpret_cast<char*>(&rows), sizeof(rows));
    outFile.write(reinterpret_cast<char*>(&cols), sizeof(cols));

    string encodedData;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            encodedData += golombEncode(img.at<uchar>(i, j), M);
        }
    }

    outFile.write(encodedData.c_str(), encodedData.size());
    outFile.close();
    cout << "Image successfully compressed!" << endl;
}

// Function to decompress an image
void decompressImage(int M) {
    ifstream inFile("compressed_golomb.bin", ios::binary);
    if (!inFile) {
        cout << "Error opening compressed file!" << endl;
        return;
    }

    int rows, cols;
    inFile.read(reinterpret_cast<char*>(&rows), sizeof(rows));
    inFile.read(reinterpret_cast<char*>(&cols), sizeof(cols));

    string encodedData((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    Mat decompressed(rows, cols, CV_8UC1);
    int index = 0;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            decompressed.at<uchar>(i, j) = golombDecode(encodedData, M, index);
        }
    }

    imwrite("decompressed_image.png", decompressed);
    cout << "Image successfully decompressed and saved as decompressed_image.png" << endl;
}

// Main function
int main() {
    int M;
    string imagePath;

    cout << "Enter image path: ";
    cin >> imagePath;

    cout << "Enter divisor (M) for Golomb coding: ";
    cin >> M;

    compressImage(imagePath, M);
    decompressImage(M);

    return 0;
}
