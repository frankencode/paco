static unsigned char idea[] = {
0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0x00,0x00,0x00,0x0D,0x49,0x48,0x44,0x52,
0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x20,0x08,0x06,0x00,0x00,0x00,0x73,0x7A,0x7A,
0xF4,0x00,0x00,0x06,0xF1,0x49,0x44,0x41,0x54,0x58,0x85,0xAD,0x97,0x5B,0x8C,0x9D,
0x55,0x15,0xC7,0x7F,0xFB,0xF2,0x7D,0xE7,0x32,0x67,0x66,0xCE,0x99,0x96,0x61,0x5A,
0x3B,0x53,0xCA,0xB4,0x85,0x14,0x8A,0x2D,0x44,0x30,0x41,0x12,0xBC,0xBC,0xD4,0xF8,
0x60,0x7C,0x31,0x92,0xC8,0x83,0xF5,0x45,0x9F,0xD4,0x84,0x68,0x0C,0x3E,0xA8,0x0F,
0x28,0xF1,0xC1,0x40,0x62,0xA2,0x18,0x9F,0x2C,0x26,0x44,0x85,0x07,0x23,0x8A,0x89,
0x10,0x8D,0xC6,0x92,0x10,0x2E,0xA9,0x52,0x5A,0x3A,0xBD,0xC9,0x94,0x99,0xCE,0x99,
0x39,0x3D,0xE7,0x7C,0xE7,0xBB,0xED,0xBD,0x97,0x0F,0x67,0x80,0x81,0x99,0x96,0x13,
0x70,0x25,0xFF,0x7C,0xC9,0x4E,0xBE,0xBD,0x7E,0x6B,0xED,0xB5,0xD7,0xDE,0x5B,0x89,
0x08,0x1B,0x4D,0x29,0xC5,0xD5,0xEC,0xCA,0xD2,0x33,0x77,0x49,0x71,0xF9,0x93,0x52,
0xAC,0x1C,0xC2,0xF7,0x6F,0x52,0x04,0x8B,0x9D,0xCC,0x95,0x9D,0x78,0x4D,0x8F,0xED,
0xFB,0xBB,0x32,0xB5,0x17,0x1B,0xCD,0xDB,0x8F,0x5F,0xED,0xFF,0xF7,0xFA,0x02,0x50,
0xA3,0x00,0x5C,0x59,0x7C,0xF2,0xAE,0xD0,0x7D,0xF1,0x01,0x49,0xCF,0xDC,0x43,0xB9,
0x3A,0x2D,0x58,0x94,0x19,0x43,0x57,0x76,0x20,0xAE,0x8B,0x20,0xA0,0x34,0xAA,0x3A,
0xBB,0xA8,0xEB,0x37,0x3E,0x67,0xC6,0x6F,0x7D,0xB8,0xD1,0x3C,0xFC,0xF2,0x87,0x06,
0xE8,0xBC,0xF1,0xDB,0x9A,0xA4,0xE7,0x8E,0x86,0xD5,0x67,0xBF,0xA7,0xE5,0xF2,0x34,
0xAA,0x8A,0x6D,0x7D,0x0A,0xDB,0xBA,0x07,0x53,0x9D,0x05,0x65,0x00,0x41,0xCA,0x55,
0x5C,0xFF,0x24,0x45,0xE7,0x05,0xC4,0xF5,0x51,0xF5,0x3D,0x17,0x6D,0xF3,0xE3,0x3F,
0x1C,0x9F,0xFE,0xCC,0x63,0x1F,0x0A,0x60,0xED,0xF4,0xC3,0x0F,0x87,0xE5,0xA7,0xBE,
0x1E,0xD9,0x6C,0x8C,0xFA,0x7E,0xA2,0xE9,0xFB,0xD0,0x63,0xFB,0x51,0xBA,0x02,0x2A,
0x06,0xA5,0x01,0x01,0x09,0x88,0x38,0x70,0x5D,0xF2,0xCB,0x7F,0xC1,0xF5,0x4E,0x80,
0xAE,0x25,0x76,0xFA,0xB3,0x0F,0x4E,0xCC,0x7C,0xEE,0xA7,0x1F,0x08,0xA0,0xB3,0xF0,
0xC8,0x97,0xFC,0x85,0x63,0x8F,0x45,0x95,0x62,0x4C,0x4D,0x1C,0x20,0x9A,0xBE,0x0F,
0x65,0xA7,0xC0,0xD4,0x40,0x57,0x50,0x2A,0x06,0x34,0xA2,0x00,0x09,0x80,0x87,0x50,
0x22,0x92,0x52,0xB4,0xFF,0x41,0xB9,0xF6,0x2F,0x94,0x9D,0x5C,0x8D,0x3E,0xF2,0xE5,
0xA3,0xE3,0xDB,0xEE,0x7E,0xEA,0x6A,0x00,0x7A,0xD3,0x08,0xD0,0x39,0xFF,0xCB,0x79,
0xF7,0xC6,0x33,0xDF,0xB7,0x3A,0x1D,0xD3,0xF5,0x16,0xB6,0xF5,0x69,0x20,0x20,0x21,
0x85,0x90,0x82,0x4F,0x11,0x3F,0x40,0xC2,0x00,0xFC,0x00,0xC2,0x00,0x7C,0x82,0x84,
0x04,0x7C,0x4A,0x34,0xBE,0x1F,0x53,0x9B,0x43,0xB2,0xC5,0x29,0xB7,0xFC,0x87,0x87,
0x92,0xCE,0x4B,0x3B,0xB7,0xF2,0x73,0x55,0x80,0xB0,0xF6,0xCA,0x57,0x54,0x7F,0x61,
0x9F,0xAD,0xC5,0x50,0xBF,0x0D,0x08,0x10,0x12,0x08,0x09,0xE2,0x87,0xC2,0xF7,0xC1,
0x27,0x9B,0x15,0x12,0x94,0x64,0x98,0xFA,0x2E,0xD0,0x15,0x42,0xF7,0xA5,0xBD,0x2E,
0x39,0x75,0xFF,0xC8,0x00,0x6B,0x0B,0x8F,0xB6,0x7C,0xFB,0xB5,0x2F,0x98,0xD8,0x40,
0xA5,0x09,0x12,0x81,0xEF,0x21,0xBE,0x8F,0xF8,0x3E,0xF8,0x1E,0x84,0x1E,0x12,0xFA,
0x88,0xEF,0xBD,0x4B,0xF8,0x1E,0xB8,0x1E,0xE2,0x7A,0x68,0xE5,0x51,0xB6,0x01,0xE5,
0x15,0x1B,0x3A,0xC7,0xBF,0xD8,0x5F,0x7B,0xA1,0xB5,0x15,0x80,0xDD,0x34,0x52,0xAC,
0xDC,0x21,0xFD,0xA5,0xD9,0x68,0xBB,0x25,0x88,0x42,0x85,0x01,0xE2,0x7B,0x28,0x00,
0xF1,0x88,0x14,0x10,0xAA,0x28,0x65,0xD7,0x8B,0x90,0x61,0x86,0xC4,0x81,0x94,0x48,
0xC8,0x20,0x64,0xE0,0xBA,0x00,0x08,0x42,0xE8,0xFD,0x7B,0x4F,0x70,0xFD,0xFD,0xC0,
0xA6,0x1E,0xB1,0x19,0x20,0xA4,0x7B,0x94,0xCB,0xC6,0xA0,0x86,0xB8,0x8C,0x20,0x6F,
0xA2,0x6D,0x15,0x11,0x8F,0xD2,0x25,0xE8,0x1C,0x74,0x86,0xF0,0x16,0x80,0xDA,0x00,
0x50,0x40,0xC8,0x41,0x52,0xA4,0x68,0x0F,0x97,0x2A,0x38,0x24,0x7B,0x73,0x4C,0xCA,
0xF6,0xEC,0x48,0x00,0x52,0x76,0x6E,0x10,0x27,0xA0,0x41,0x49,0x4E,0xD9,0x3B,0x8F,
0x89,0x6A,0xE8,0xD8,0x21,0xA6,0x00,0xA9,0x42,0xA8,0xBC,0x93,0x01,0x51,0xA0,0x02,
0x04,0x07,0xAC,0x67,0xC0,0x27,0x14,0xBD,0xF3,0xE0,0x33,0x08,0x1E,0x29,0x0B,0x0B,
0x6A,0xC7,0x68,0x4B,0x20,0x0E,0x3C,0x10,0x14,0x52,0x04,0xA4,0x4C,0xC9,0xDB,0x27,
0x88,0x9B,0xF3,0x98,0xEA,0x76,0x82,0x8A,0x89,0x1A,0x53,0x40,0x84,0xCF,0x32,0x10,
0xD0,0x95,0x71,0x44,0x52,0x7C,0xD6,0x46,0x49,0x4E,0xD1,0xFD,0x2F,0x52,0x26,0xC3,
0x6D,0xE9,0x4B,0xF0,0x02,0x84,0xDA,0x68,0x00,0x2A,0x5E,0x11,0x40,0x9C,0xC6,0xD6,
0x63,0xF2,0x6E,0x82,0xF8,0x82,0x7C,0xE5,0x24,0xB5,0x99,0xEB,0x09,0x66,0x82,0x90,
0x64,0xE8,0x6A,0x83,0x68,0x7C,0x82,0x50,0x04,0xC4,0xAD,0xE2,0x8B,0x2E,0xCE,0xB7,
0x91,0xFE,0x9B,0xF8,0x2C,0x85,0x50,0x82,0x78,0x28,0x02,0xE2,0x05,0x71,0xDD,0x85,
0x11,0x01,0xD4,0x59,0x54,0x94,0xE0,0xCC,0x18,0x41,0x61,0x74,0x84,0x2B,0x87,0x51,
0x24,0x17,0x17,0xE9,0x76,0x4E,0xD3,0x9A,0xD9,0x86,0xB2,0xC3,0x46,0x84,0x02,0xF1,
0x25,0x45,0x3A,0x20,0x78,0x4F,0xBD,0xD9,0x80,0x32,0x47,0x82,0x60,0xB4,0xC5,0x3B,
0x20,0x9E,0xBA,0x82,0xAE,0x76,0x46,0x02,0x50,0xF1,0x75,0xCF,0x4B,0x54,0x5B,0x76,
0x19,0x7B,0xE2,0x7A,0x44,0x14,0x55,0x28,0xD3,0x62,0xB8,0x61,0xB5,0xC2,0x6A,0x4B,
0xD1,0x4B,0xB1,0x51,0x31,0xAC,0xBF,0xF5,0xCE,0xED,0xD2,0x02,0x1B,0x59,0x5C,0x37,
0x03,0x11,0x94,0xD2,0x68,0x65,0x29,0xF3,0x02,0xD3,0xDA,0x75,0x51,0xD9,0xE6,0x7F,
0xB6,0x02,0xD8,0xD4,0x07,0x5A,0x37,0xFF,0x60,0xD1,0xCE,0xEC,0x7A,0xB6,0xEC,0xE5,
0x84,0x4C,0x63,0xE2,0x1A,0x91,0x89,0x91,0x42,0x90,0x22,0xA0,0x83,0xC2,0x67,0x7E,
0x58,0x1F,0x85,0xBC,0x2D,0x97,0x7A,0x94,0x63,0xBD,0x6E,0xC0,0x9A,0x0A,0x92,0x06,
0xC4,0x05,0xF4,0xC4,0xFC,0x5F,0x27,0x67,0x8E,0x2C,0x8E,0x04,0x00,0x60,0x26,0x67,
0x7F,0xE1,0x95,0x5A,0xF4,0x99,0x46,0xA4,0x46,0x5C,0xAB,0xA3,0x95,0x45,0x4A,0x41,
0x7B,0x85,0x1B,0x94,0x48,0x29,0xEB,0x10,0x43,0xB9,0x81,0x5B,0x07,0x10,0xA2,0xA8,
0x82,0x11,0x43,0xD1,0xCD,0xD0,0xCD,0x5D,0x17,0xD5,0xE4,0xDE,0x5F,0x6D,0xE5,0xE7,
0xAA,0x00,0xAD,0x83,0x8F,0x1C,0x8F,0xE6,0xF6,0x1D,0xCB,0xDA,0x2B,0x79,0xC8,0x22,
0x54,0xDC,0x24,0xAE,0x54,0xD1,0x18,0x74,0x50,0x14,0x89,0x1B,0x46,0x5E,0xBE,0x13,
0xBD,0x38,0x01,0x07,0x95,0x7A,0x9D,0x48,0xC7,0x64,0x2B,0x09,0x62,0xE3,0xC4,0xCE,
0x7E,0xE2,0xB1,0xD6,0xEE,0xA3,0x9B,0xEE,0x06,0xD7,0x04,0x00,0xD0,0x93,0x37,0x3E,
0xA4,0x67,0xB6,0xFF,0x39,0x5D,0x5E,0x72,0x21,0x1F,0x42,0xE0,0xC0,0x60,0x28,0x93,
0xE2,0x5D,0xE9,0x2F,0x93,0x92,0xD8,0xC6,0xD4,0x27,0x26,0xB0,0x2A,0x22,0x5D,0xEE,
0x53,0x16,0xDE,0x45,0xF3,0xF7,0xFE,0x5C,0x8F,0xEF,0xFD,0xC9,0xD5,0x7C,0x5C,0x13,
0xA0,0x75,0xE0,0x47,0x6B,0x76,0xE7,0x1D,0x5F,0xD3,0x3B,0x5B,0x8F,0x67,0x97,0x97,
0x9D,0xA2,0x82,0x88,0xC5,0xE7,0x1E,0xE5,0x15,0x3E,0x77,0x68,0x65,0x88,0xE2,0x18,
0x1B,0x19,0x26,0xB7,0x4D,0x12,0x72,0x4F,0xFF,0x62,0x87,0x62,0x90,0x53,0x3D,0x78,
0xEF,0xCF,0xCC,0xF6,0xDB,0x1E,0x6C,0xCE,0xDD,0x9F,0x7E,0x20,0x80,0x21,0xC4,0x8F,
0x17,0xCD,0xF4,0x47,0x1F,0x2D,0xBA,0xAB,0x09,0x44,0x18,0x13,0xA3,0x44,0x51,0x89,
0x23,0x94,0xD2,0x44,0x71,0x8C,0xD6,0x96,0xC1,0x4A,0x42,0xB6,0xD2,0x27,0x6D,0xF7,
0x09,0x65,0x40,0x55,0x6B,0x57,0xA8,0x36,0x9F,0x6D,0xEE,0xFE,0xEA,0x35,0x9D,0xBF,
0x2F,0x00,0x80,0x04,0xB7,0xA6,0x2A,0x3A,0x01,0x83,0x8E,0x63,0x94,0x52,0x18,0x6B,
0x48,0x3B,0x09,0x65,0x5E,0x52,0x0C,0x32,0xCA,0x24,0xC7,0x18,0x8B,0x42,0x41,0x10,
0x08,0x82,0xAA,0xEC,0x38,0xFD,0x7E,0x73,0x8F,0x04,0x80,0xB8,0x14,0x53,0x5B,0x2D,
0x3B,0x5D,0xA2,0xC6,0x38,0x3A,0xB2,0x54,0x27,0xC6,0x29,0x73,0x41,0x57,0xB7,0x11,
0xC4,0x60,0xE3,0x08,0x1D,0x69,0xB4,0xD6,0x48,0x29,0xE0,0x03,0x48,0x91,0xFD,0x5F,
0x00,0xA6,0x0E,0x3E,0xBA,0xA8,0x63,0xF2,0x90,0x65,0x98,0x5A,0x0D,0x53,0xAB,0x51,
0xA4,0x1E,0x5B,0x8D,0xC8,0xBA,0x8E,0xA2,0x9F,0xA1,0x8D,0x23,0x04,0x87,0x36,0x9A,
0xE0,0x03,0x7A,0x72,0xEA,0xEC,0xF0,0x70,0x7A,0x7F,0xDB,0xDC,0x8A,0xB7,0x30,0x37,
0x77,0xE8,0x37,0xC5,0xE9,0x53,0x77,0x98,0xA5,0x55,0xE2,0xE6,0x36,0xF4,0xD8,0x41,
0xEA,0xA1,0x41,0x70,0x0E,0x25,0x1D,0x6C,0xB4,0x4A,0x18,0x5C,0xA2,0xEC,0x38,0xFC,
0xEC,0x5E,0xCA,0x9D,0x37,0xFF,0x7A,0x66,0xFF,0x77,0xCF,0x8C,0x32,0xF7,0x48,0xEF,
0x02,0x80,0x57,0x9E,0x7F,0x42,0xC2,0xA5,0xA7,0x69,0x49,0x9B,0x6A,0xD0,0x48,0x1C,
0x03,0x8E,0xDC,0x1B,0xC2,0xA0,0x4F,0x63,0xC7,0x2C,0xE6,0xFA,0xDB,0x39,0xB9,0x34,
0xCD,0xDF,0x8E,0x9F,0x39,0xF0,0x9D,0x6F,0x3F,0xF0,0xEA,0x7B,0xE7,0xD8,0xEA,0x52,
0x3A,0x52,0x06,0x00,0x7A,0x7E,0xEE,0xA5,0x13,0xCB,0x77,0x1F,0xDA,0x3F,0x3F,0xCF,
0xE1,0xDB,0x76,0xD3,0x1F,0x0C,0xC8,0xF3,0x9C,0xB3,0x0B,0xE7,0x60,0x6C,0x82,0x8F,
0xDD,0x32,0x4F,0x08,0x9E,0xE7,0x9E,0xFC,0x1D,0x02,0x97,0x46,0x9D,0xF7,0x5A,0x00,
0x1A,0x30,0x40,0x04,0x54,0x2E,0x5C,0xB8,0x78,0x6A,0x61,0xE1,0xEC,0xA1,0xE9,0xED,
0xD7,0xB1,0xB4,0xEA,0x18,0x6F,0x4C,0x61,0x71,0x94,0xB2,0x02,0x45,0xC1,0xEB,0x0B,
0x67,0x71,0xCE,0x53,0xAD,0x55,0x07,0xDF,0xFA,0xE6,0x37,0x04,0xA8,0x01,0x25,0xC3,
0xDB,0xC5,0xE6,0xD0,0x37,0x38,0xD9,0x6A,0xAC,0x0A,0xB4,0x80,0x9D,0xC0,0x8D,0xC0,
0xCD,0x7F,0x7A,0xFA,0x8F,0x27,0x1B,0x8D,0x06,0x4A,0x29,0x7A,0xDD,0x1E,0x59,0x9E,
0xD3,0xED,0x76,0xE9,0x76,0xBB,0xF8,0x20,0xCC,0xCC,0x5C,0xCF,0xDC,0xDC,0x2E,0x5E,
0x3D,0xF9,0xDA,0xCB,0xC0,0x4D,0xC0,0x0D,0xC0,0x0E,0xA0,0x09,0x54,0x78,0xFB,0xDC,
0x7C,0xB7,0x6D,0x55,0x03,0x16,0xA8,0x03,0x0D,0x60,0x02,0x98,0x7C,0x4B,0x87,0x0F,
0x1F,0xFE,0xFC,0x91,0x23,0x47,0xEE,0xBB,0xF3,0xCE,0x3B,0xA9,0x54,0xAB,0x38,0xE7,
0x58,0x6D,0xAF,0x92,0xE7,0x29,0xED,0x95,0x25,0x8E,0x3D,0xFE,0xC4,0xB1,0x13,0x27,
0x4E,0xFC,0x1E,0xB8,0xB2,0xAE,0xEE,0xBA,0xFA,0x40,0x2A,0x22,0x7E,0x14,0x00,0xD6,
0x53,0x5F,0x5D,0x4F,0xE3,0xD8,0x06,0xD5,0x80,0x6D,0xD3,0xD3,0xD3,0xF3,0x07,0x6E,
0xB9,0xF5,0x6E,0x6D,0xEC,0x0E,0xA5,0x54,0xF5,0xFC,0xB9,0xB3,0xCF,0xBC,0x7E,0xFA,
0xD4,0x3F,0x81,0x36,0x30,0x00,0x92,0x0D,0x4A,0x81,0x1C,0x08,0x1F,0xF4,0x75,0xFC,
0x56,0x2D,0xD8,0x0D,0x5F,0xCD,0x3B,0xCB,0x27,0xEB,0xF2,0x80,0xDB,0xF0,0x0D,0xEB,
0x7A,0xDB,0xB6,0x02,0xF8,0x1F,0x7B,0xB4,0xE2,0x68,0x2B,0x4F,0x25,0x2B,0x00,0x00,
0x00,0x00,0x49,0x45,0x4E,0x44,0xAE,0x42,0x60,0x82
};
static int ideaSize = 1834;
