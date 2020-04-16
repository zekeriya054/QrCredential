#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 
#endif
#ifndef QrCodeGenerator_H
#define QrCodeGenerator_H
#pragma once
#include <Shlobj.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <gdiplus.h>
#include <stdlib.h>
#include "BitBuffer.hpp"
#include "QrCode.hpp"
#include "adapter.h"
#pragma comment (lib, "Gdiplus.lib")
#define  OUT_FILE_PIXEL_PRESCALER 8
#define OUT_FILE "test.bmp"
using namespace Gdiplus;
using namespace IpTespit;

using std::uint8_t;
using qrcodegen::QrCode;
using qrcodegen::QrSegment;





// Function prototypes
static void doBasicDemo();
static void doVarietyDemo();
static void doSegmentDemo();
static void doMaskDemo();
static void printQr(const QrCode &qr);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
void CreateImage(const QrCode &qr);
#endif