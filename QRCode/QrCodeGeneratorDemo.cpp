/* 
 * QR Code generator demo (C++)
 * 
 * Run this command-line program with no arguments. The program computes a bunch of demonstration
 * QR Codes and prints them to the console. Also, the SVG code for one QR Code is printed as a sample.
 * 
 * Copyright (c) Project Nayuki. (MIT License)
 * https://www.nayuki.io/page/qr-code-generator-library
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 
#endif
#pragma once
#include"QrCodeGeneratorDemo.h"
#include "Shlwapi.h"
#pragma comment (lib, "Gdiplus.lib")
using namespace Gdiplus;
using namespace IpTespit;
#define  OUT_FILE_PIXEL_PRESCALER 8
#define OUT_FILE "test.bmp"
using std::uint8_t;
using qrcodegen::QrCode;
using qrcodegen::QrSegment;


unsigned int	 x, y, l, n;
unsigned char*	pRGBData, *pSourceData, *pDestData, *tRGBData;
char text[512];

FILE*			f;


// Function prototypes
static void doBasicDemo();
static void doVarietyDemo();
static void doSegmentDemo();
static void doMaskDemo();
static void printQr(const QrCode &qr);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
void CreateImage(const QrCode &qr);

// The main application program.
int main(int argc,char *argv[]) {

	doBasicDemo();
//	doVarietyDemo();
//	doSegmentDemo();
//	doMaskDemo();
//  system("pause");
	//return EXIT_SUCCESS;
	return 0;
}



/*---- Demo suite ----*/

// Creates a single QR Code, then prints it to the console.
static void doBasicDemo() {
	
//	if (sinif != NULL) {
		//const char *text = "sinif";
		const QrCode::Ecc errCorLvl = QrCode::Ecc::LOW;  // Error correction level
		WSAData d;
		char text2[512];
		if (WSAStartup(MAKEWORD(2, 2), &d) != 0) {
		//	return -1;
		}
		Adapter *adapter = new Adapter();
		//text = adapter->adapterAdress;
		strcpy_s(text, adapter->adapterAdress);
	//	printf("%s", text);
		int i;
		for (i = 0; text[i] != NULL; i++) {
			switch ('9' - text[i])
			{
			case '0':text2[i] = 'y'; break;
			case '1':text2[i] = 'u'; break;
			case '2':text2[i] = 's'; break;
			case '3':text2[i] = 'f'; break;
			case '4':text2[i] = 'b'; break;
			case '5':text2[i] = 'a'; break;
			case '6':text2[i] = 'd'; break;
			case '7':text2[i] = 'e'; break;
			case '8':text2[i] = 'z'; break;
			case '9':text2[i] = 'k'; break;
			case '.':text2[i] = 'p'; break;
			}
		}
		text2[i] = NULL;
		WSACleanup();
		// Make and print the QR Code symbol
		const QrCode qr = QrCode::encodeText(text2, errCorLvl);
		CreateImage(qr);
	  //	printQr(qr);
		//std::cout << qr.toSvgString(4) << std::endl;
//	}
		
}


// Creates a variety of QR Codes that exercise different features of the library, and prints each one to the console.
static void doVarietyDemo() {
	// Numeric mode encoding (3.33 bits per digit)
	const QrCode qr1 = QrCode::encodeText("314159265358979323846264338327950288419716939937510", QrCode::Ecc::MEDIUM);
	printQr(qr1);
	
	// Alphanumeric mode encoding (5.5 bits per character)
	const QrCode qr2 = QrCode::encodeText("DOLLAR-AMOUNT:$39.87 PERCENTAGE:100.00% OPERATIONS:+-*/", QrCode::Ecc::HIGH);
	printQr(qr2);
	
	// Unicode text as UTF-8
	const QrCode qr3 = QrCode::encodeText("\xE3\x81\x93\xE3\x82\x93\xE3\x81\xAB\xE3\x81\xA1wa\xE3\x80\x81\xE4\xB8\x96\xE7\x95\x8C\xEF\xBC\x81\x20\xCE\xB1\xCE\xB2\xCE\xB3\xCE\xB4", QrCode::Ecc::QUARTILE);
	printQr(qr3);
	
	// Moderately large QR Code using longer text (from Lewis Carroll's Alice in Wonderland)
	const QrCode qr4 = QrCode::encodeText(
		"Alice was beginning to get very tired of sitting by her sister on the bank, "
		"and of having nothing to do: once or twice she had peeped into the book her sister was reading, "
		"but it had no pictures or conversations in it, 'and what is the use of a book,' thought Alice "
		"'without pictures or conversations?' So she was considering in her own mind (as well as she could, "
		"for the hot day made her feel very sleepy and stupid), whether the pleasure of making a "
		"daisy-chain would be worth the trouble of getting up and picking the daisies, when suddenly "
		"a White Rabbit with pink eyes ran close by her.", QrCode::Ecc::HIGH);
	printQr(qr4);
}


// Creates QR Codes with manually specified segments for better compactness.
static void doSegmentDemo() {
	// Illustration "silver"
	const char *silver0 = "THE SQUARE ROOT OF 2 IS 1.";
	const char *silver1 = "41421356237309504880168872420969807856967187537694807317667973799";
	const QrCode qr0 = QrCode::encodeText(
		(std::string(silver0) + silver1).c_str(),
		QrCode::Ecc::LOW);
	printQr(qr0);
	
	const QrCode qr1 = QrCode::encodeSegments(
		{QrSegment::makeAlphanumeric(silver0), QrSegment::makeNumeric(silver1)},
		QrCode::Ecc::LOW);
	printQr(qr1);
	
	// Illustration "golden"
	const char *golden0 = "Golden ratio \xCF\x86 = 1.";
	const char *golden1 = "6180339887498948482045868343656381177203091798057628621354486227052604628189024497072072041893911374";
	const char *golden2 = "......";
	const QrCode qr2 = QrCode::encodeText(
		(std::string(golden0) + golden1 + golden2).c_str(),
		QrCode::Ecc::LOW);
	printQr(qr2);
	
	std::vector<uint8_t> bytes(golden0, golden0 + std::strlen(golden0));
	const QrCode qr3 = QrCode::encodeSegments(
		{QrSegment::makeBytes(bytes), QrSegment::makeNumeric(golden1), QrSegment::makeAlphanumeric(golden2)},
		QrCode::Ecc::LOW);
	printQr(qr3);
	
	// Illustration "Madoka": kanji, kana, Greek, Cyrillic, full-width Latin characters
	const char *madoka =  // Encoded in UTF-8
		"\xE3\x80\x8C\xE9\xAD\x94\xE6\xB3\x95\xE5"
		"\xB0\x91\xE5\xA5\xB3\xE3\x81\xBE\xE3\x81"
		"\xA9\xE3\x81\x8B\xE2\x98\x86\xE3\x83\x9E"
		"\xE3\x82\xAE\xE3\x82\xAB\xE3\x80\x8D\xE3"
		"\x81\xA3\xE3\x81\xA6\xE3\x80\x81\xE3\x80"
		"\x80\xD0\x98\xD0\x90\xD0\x98\xE3\x80\x80"
		"\xEF\xBD\x84\xEF\xBD\x85\xEF\xBD\x93\xEF"
		"\xBD\x95\xE3\x80\x80\xCE\xBA\xCE\xB1\xEF"
		"\xBC\x9F";
	const QrCode qr4 = QrCode::encodeText(madoka, QrCode::Ecc::LOW);
	printQr(qr4);
	
	const std::vector<int> kanjiChars{  // Kanji mode encoding (13 bits per character)
		0x0035, 0x1002, 0x0FC0, 0x0AED, 0x0AD7,
		0x015C, 0x0147, 0x0129, 0x0059, 0x01BD,
		0x018D, 0x018A, 0x0036, 0x0141, 0x0144,
		0x0001, 0x0000, 0x0249, 0x0240, 0x0249,
		0x0000, 0x0104, 0x0105, 0x0113, 0x0115,
		0x0000, 0x0208, 0x01FF, 0x0008,
	};
	qrcodegen::BitBuffer bb;
	for (int c : kanjiChars)
		bb.appendBits(c, 13);
	const QrCode qr5 = QrCode::encodeSegments(
		{QrSegment(QrSegment::Mode::KANJI, kanjiChars.size(), bb)},
		QrCode::Ecc::LOW);
	printQr(qr5);
}


// Creates QR Codes with the same size and contents but different mask patterns.
static void doMaskDemo() {
	// Project Nayuki URL
	std::vector<QrSegment> segs0 = QrSegment::makeSegments("https://www.nayuki.io/");
	printQr(QrCode::encodeSegments(segs0, QrCode::Ecc::HIGH, QrCode::MIN_VERSION, QrCode::MAX_VERSION, -1, true));  // Automatic mask
	printQr(QrCode::encodeSegments(segs0, QrCode::Ecc::HIGH, QrCode::MIN_VERSION, QrCode::MAX_VERSION, 3, true));  // Force mask 3
	
	// Chinese text as UTF-8
	std::vector<QrSegment> segs1 = QrSegment::makeSegments(
		"\xE7\xB6\xAD\xE5\x9F\xBA\xE7\x99\xBE\xE7\xA7\x91\xEF\xBC\x88\x57\x69\x6B\x69\x70"
		"\x65\x64\x69\x61\xEF\xBC\x8C\xE8\x81\x86\xE8\x81\xBD\x69\x2F\xCB\x8C\x77\xC9\xAA"
		"\x6B\xE1\xB5\xBB\xCB\x88\x70\x69\xCB\x90\x64\x69\x2E\xC9\x99\x2F\xEF\xBC\x89\xE6"
		"\x98\xAF\xE4\xB8\x80\xE5\x80\x8B\xE8\x87\xAA\xE7\x94\xB1\xE5\x85\xA7\xE5\xAE\xB9"
		"\xE3\x80\x81\xE5\x85\xAC\xE9\x96\x8B\xE7\xB7\xA8\xE8\xBC\xAF\xE4\xB8\x94\xE5\xA4"
		"\x9A\xE8\xAA\x9E\xE8\xA8\x80\xE7\x9A\x84\xE7\xB6\xB2\xE8\xB7\xAF\xE7\x99\xBE\xE7"
		"\xA7\x91\xE5\x85\xA8\xE6\x9B\xB8\xE5\x8D\x94\xE4\xBD\x9C\xE8\xA8\x88\xE7\x95\xAB");
	printQr(QrCode::encodeSegments(segs1, QrCode::Ecc::MEDIUM, QrCode::MIN_VERSION, QrCode::MAX_VERSION, 0, true));  // Force mask 0
	printQr(QrCode::encodeSegments(segs1, QrCode::Ecc::MEDIUM, QrCode::MIN_VERSION, QrCode::MAX_VERSION, 1, true));  // Force mask 1
	printQr(QrCode::encodeSegments(segs1, QrCode::Ecc::MEDIUM, QrCode::MIN_VERSION, QrCode::MAX_VERSION, 5, true));  // Force mask 5
	printQr(QrCode::encodeSegments(segs1, QrCode::Ecc::MEDIUM, QrCode::MIN_VERSION, QrCode::MAX_VERSION, 7, true));  // Force mask 7
}



/*---- Utilities ----*/

// Prints the given QR Code to the console.
static void printQr(const QrCode &qr) {
	int border = 4;
	int tWidth = (qr.getSize() + 2 * border);
	int tWidthAdjusted = tWidth*OUT_FILE_PIXEL_PRESCALER * 3;
	if (tWidthAdjusted % 4)
		tWidthAdjusted = (tWidthAdjusted / 4 + 1) * 4;

	int tDataBytes = tWidthAdjusted*tWidth*OUT_FILE_PIXEL_PRESCALER * 3;


	if (!(tRGBData = (unsigned char *)malloc(tDataBytes))) {
		printf("Out of memory");
		exit(-1);
	}
	// Preset to white

	memset(tRGBData, 0xFF, tDataBytes);

	// Prepare bmp headers

	BITMAPFILEHEADER kFileHeader;
	kFileHeader.bfType = 0x4d42;  // "BM"
	kFileHeader.bfSize = sizeof(BITMAPFILEHEADER) +
		sizeof(BITMAPINFOHEADER) + 
		tDataBytes;
	kFileHeader.bfReserved1 = 0;
	kFileHeader.bfReserved2 = 0;
	kFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) +
		sizeof(BITMAPINFOHEADER);

	BITMAPINFOHEADER kInfoHeader;
	kInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	kInfoHeader.biWidth = tWidth * OUT_FILE_PIXEL_PRESCALER;
	kInfoHeader.biHeight = -((int)tWidth * OUT_FILE_PIXEL_PRESCALER);
	kInfoHeader.biPlanes = 1;
	kInfoHeader.biBitCount = 24;
	kInfoHeader.biCompression = BI_RGB;
	kInfoHeader.biSizeImage = 0;
	kInfoHeader.biXPelsPerMeter = 0;
	kInfoHeader.biYPelsPerMeter = 0;
	kInfoHeader.biClrUsed = 0;
	kInfoHeader.biClrImportant = 0;


	// Convert QrCode bits to bmp pixels


	for (int y = 0; y < qr.getSize() ; y++) {
		//pDestData = pRGBData + unWidthAdjusted * y * OUT_FILE_PIXEL_PRESCALER;
		pDestData = tRGBData + tWidthAdjusted * (y+border) * OUT_FILE_PIXEL_PRESCALER+border*OUT_FILE_PIXEL_PRESCALER*3;
		for (int x = 0; x < qr.getSize() ; x++) {
			std::cout << (qr.getModule(x, y) ? "##" : "  ");
			if (qr.getModule(x, y)) {
				for (l = 0; l < OUT_FILE_PIXEL_PRESCALER; l++)
				{
					for (n = 0; n < OUT_FILE_PIXEL_PRESCALER; n++)
					{
						*(pDestData + n * 3 + tWidthAdjusted * l) = 0;//PIXEL_COLOR_B;
						*(pDestData + 1 + n * 3 + tWidthAdjusted * l) = 0;//	PIXEL_COLOR_G;
						*(pDestData + 2 + n * 3 + tWidthAdjusted * l) = 0;//	PIXEL_COLOR_R;
					}
				}

			}

			pDestData += 3 * OUT_FILE_PIXEL_PRESCALER;
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;

	// Output the bmp file

	if (!(fopen_s(&f, OUT_FILE, "wb")))
	{
		fwrite(&kFileHeader, sizeof(BITMAPFILEHEADER), 1, f);
		fwrite(&kInfoHeader, sizeof(BITMAPINFOHEADER), 1, f);
		fwrite(tRGBData, sizeof(unsigned char), tDataBytes, f);

		fclose(f);
	}
	else
	{
		printf("Unable to open file");
		exit(-1);
	}


}

void CreateImage(const QrCode &qr)
{
	int border = 2;
	int tWidth = (qr.getSize() + 2 * border);
	int tWidthAdjusted = tWidth*OUT_FILE_PIXEL_PRESCALER * 3;
	if (tWidthAdjusted % 4)
		tWidthAdjusted = (tWidthAdjusted / 4 + 1) * 4;

	int tDataBytes = tWidthAdjusted*tWidth*OUT_FILE_PIXEL_PRESCALER * 3;


	if (!(tRGBData = (unsigned char *)malloc(tDataBytes))) {
		printf("Out of memory");
		exit(-1);
	}
	// Preset to white

	memset(tRGBData, 0xFF, tDataBytes);

	for (int y = 0; y < qr.getSize(); y++) {
		//pDestData = pRGBData + unWidthAdjusted * y * OUT_FILE_PIXEL_PRESCALER;
		pDestData = tRGBData + tWidthAdjusted * (y + border) * OUT_FILE_PIXEL_PRESCALER + border*OUT_FILE_PIXEL_PRESCALER * 3;
		for (int x = 0; x < qr.getSize(); x++) {
			//std::cout << (qr.getModule(x, y) ? "##" : "  ");
			if (qr.getModule(x, y)) {
				for (l = 0; l < OUT_FILE_PIXEL_PRESCALER; l++)
				{
					for (n = 0; n < OUT_FILE_PIXEL_PRESCALER; n++)
					{
						*(pDestData + n * 3 + tWidthAdjusted * l) = 0;//PIXEL_COLOR_B;
						*(pDestData + 1 + n * 3 + tWidthAdjusted * l) = 0;//	PIXEL_COLOR_G;
						*(pDestData + 2 + n * 3 + tWidthAdjusted * l) = 0;//	PIXEL_COLOR_R;
					}
				}

			}

			pDestData += 3 * OUT_FILE_PIXEL_PRESCALER;
		}
		//std::cout << std::endl;
	}
	int nImageWidth = tWidthAdjusted / 3;
	int nImageHeight = nImageWidth;
	//BYTE* pImageData = new BYTE[nImageWidth * 3 * nImageHeight];
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	// gdiplus
	Bitmap bmp(nImageWidth, nImageHeight, 3 * nImageWidth, PixelFormat24bppRGB, tRGBData);
	Gdiplus::Graphics *g = Gdiplus::Graphics::FromImage(&bmp);
	//g->Clear(Color(255, 255, 255, 255));

	// draw 
	// ...
	//draw on bitmap

	/*
	Pen blackpen(Color(255, 0, 0, 0), 3);

	int x1 = 1;
	int x2 = 200;
	int y1 = 1;
	int y2 = 200;
	g->DrawLine(&blackpen, x1, y1, x2, y2);
	*/

	HRESULT hr;
	CLSID pngClsid;
	wchar_t pdPath[MAX_PATH];
	wchar_t path2[MAX_PATH]=L"\\EtCredential";
	hr=SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, pdPath);
	wcscat(pdPath, path2);
	int ret=PathFileExistsW(pdPath);
	if (ret != 1) {
		CreateDirectoryW(pdPath, NULL);
	}
	wcscat(pdPath,L"\\QrCode.bmp");
	GetEncoderClsid(L"image/bmp", &pngClsid);
	bmp.Save(pdPath, &pngClsid);
	//delete[] pImageData;
	delete g;

}
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}
