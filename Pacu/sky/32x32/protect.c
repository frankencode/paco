static unsigned char protect[] = {
0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0x00,0x00,0x00,0x0D,0x49,0x48,0x44,0x52,
0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x20,0x08,0x06,0x00,0x00,0x00,0x73,0x7A,0x7A,
0xF4,0x00,0x00,0x05,0xAE,0x49,0x44,0x41,0x54,0x58,0x85,0xC5,0x97,0x4F,0x6C,0x5C,
0x57,0x15,0xC6,0x7F,0xF7,0x8D,0xED,0xD4,0x09,0x49,0x5C,0xA7,0x06,0xBB,0x34,0x2D,
0x91,0xA1,0x11,0x0B,0x68,0xAB,0x4A,0x45,0x80,0x8A,0xEC,0x20,0x21,0x58,0x20,0xF1,
0xA7,0x08,0x24,0x10,0x30,0x9B,0x2C,0x58,0x54,0xAD,0xD4,0x05,0x02,0xA9,0x2A,0x52,
0x25,0xB6,0x64,0x81,0x58,0x64,0xE3,0x80,0xA8,0x10,0x02,0x81,0x40,0x88,0xAD,0x85,
0x4A,0x8B,0x42,0xEB,0xBA,0xC6,0x8E,0xEB,0xD4,0x71,0x9C,0xA9,0x9B,0xB1,0xC7,0x99,
0xF1,0x9B,0x3F,0xF1,0xCC,0xBC,0xF7,0xCE,0x39,0x2C,0xDE,0xBC,0x99,0x37,0xE3,0x19,
0xC7,0x65,0xC3,0x95,0x8E,0xCE,0xBB,0x33,0xF7,0xDD,0xEF,0x7B,0xDF,0x77,0xDE,0x7D,
0xF7,0x3A,0x33,0xE3,0xFF,0xD9,0x86,0x00,0x9C,0x73,0x00,0x5C,0xBC,0x78,0xF1,0x26,
0x70,0xC5,0xCC,0xE6,0x81,0xC5,0xCB,0x97,0x2F,0xFB,0x83,0x6E,0xCC,0x66,0xB3,0x8F,
0x03,0x3F,0xA8,0x8D,0x7F,0xFE,0xB9,0xFA,0xA9,0x4F,0xCD,0x39,0xC7,0xA5,0xBF,0xBE,
0xF8,0x99,0xC5,0xC3,0xC0,0xBE,0xF1,0xF3,0x37,0x7E,0xE8,0xE0,0xA7,0x7F,0xF8,0xF1,
0x93,0x9F,0x48,0x30,0x9D,0x99,0xB5,0x09,0x64,0xB3,0x59,0x9B,0x9E,0x9E,0xA6,0xB0,
0xBB,0x4B,0xAD,0x56,0x8B,0xEF,0x32,0x9B,0xB7,0x38,0xD3,0xCA,0x63,0x06,0x8F,0x9B,
0x19,0x53,0x93,0x93,0x5C,0xF8,0xCA,0x33,0x5C,0xDB,0x3B,0xC3,0x3F,0x57,0x8B,0x34,
0x42,0xF5,0x81,0x45,0xE7,0xDC,0x26,0x70,0xCB,0x39,0x70,0x8E,0xC7,0x80,0x31,0x70,
0x33,0xE7,0x3E,0x7C,0x9C,0x42,0xB9,0xC9,0x6F,0x5F,0x78,0xC2,0x25,0x98,0x43,0x69,
0x86,0x61,0x14,0x21,0xAA,0x7C,0xE1,0xE9,0xA7,0x39,0x3D,0x36,0x46,0x2E,0x97,0x23,
0x97,0xCB,0xCD,0xF8,0xBE,0x4F,0xB1,0x58,0x64,0x74,0x74,0x94,0xC9,0xC9,0x49,0x26,
0x26,0x26,0x38,0xFF,0xE8,0xA3,0xEC,0xEC,0xEC,0xF0,0xAB,0x3B,0x9F,0xE4,0x47,0xE7,
0x8D,0x2F,0x7D,0xFA,0x01,0x56,0xDF,0xAB,0x8E,0xDD,0xDE,0x6B,0xCC,0x6C,0x15,0x1B,
0x38,0x07,0x6A,0x60,0x66,0x4C,0x4F,0x9E,0xE0,0x91,0xC9,0x93,0xBC,0x53,0x3F,0x46,
0xEE,0x8D,0x5C,0xEB,0xB9,0xEC,0x20,0x81,0x20,0x08,0xA8,0x94,0xCB,0xBC,0xB5,0xB4,
0xCC,0xC8,0xC8,0x08,0x1F,0x3F,0xF7,0x08,0x5F,0xBC,0x70,0x81,0xF1,0xF1,0x71,0x26,
0x26,0x26,0xF0,0x7D,0x9F,0x62,0xA9,0xC4,0xC6,0xC6,0x06,0x0B,0x0B,0x0B,0xEC,0xEF,
0xEF,0xC3,0x63,0xF0,0xEB,0x4D,0x8F,0x27,0x1F,0x80,0x87,0xCF,0xDC,0xCF,0x67,0x1F,
0x76,0x8C,0x64,0x1C,0xCE,0x39,0x22,0x35,0x9A,0x91,0xB1,0x52,0x52,0x7E,0xB3,0x65,
0xE4,0xCB,0xC6,0xC7,0x68,0x1E,0xAC,0x81,0xB6,0x02,0x61,0x48,0x14,0x45,0x88,0x39,
0xF6,0x25,0xC3,0xF2,0xEA,0x1A,0x2B,0x2B,0x2B,0xED,0xFF,0x9D,0x73,0xF4,0x2B,0xDA,
0xA1,0x8C,0x63,0xC9,0x87,0x25,0x3F,0x7E,0x32,0xC3,0x30,0x03,0x55,0x10,0x35,0x42,
0x81,0x1D,0x5F,0x89,0x04,0x44,0xBA,0xEF,0x3F,0x40,0x40,0x44,0x10,0x11,0xCC,0x09,
0x88,0xE0,0x4C,0xFB,0x94,0x53,0x52,0x1E,0xF1,0x64,0xA1,0x74,0xEA,0xC8,0x2C,0x06,
0x37,0x8B,0x2D,0x08,0x23,0xC8,0xED,0x0A,0xCE,0x81,0xE7,0x41,0xA4,0x47,0x20,0xA0,
0xAA,0xA8,0x2A,0xA8,0xE2,0x54,0x06,0x12,0x48,0x5A,0x10,0x81,0x73,0xAD,0x89,0x53,
0xE0,0x91,0x1A,0x5B,0xBB,0x4A,0x23,0x34,0x46,0x87,0xC1,0x00,0x91,0xEE,0x07,0x3A,
0x50,0x03,0x91,0x08,0x22,0x8A,0x79,0x1A,0x2B,0x70,0x04,0x02,0x61,0x64,0xE0,0x12,
0x59,0x12,0x20,0x78,0xBF,0x28,0x34,0x83,0x96,0x1D,0x19,0x07,0x76,0x14,0x0B,0xA2,
0x08,0x51,0xC1,0x24,0xB6,0x00,0x95,0xB6,0xBC,0xE9,0x96,0xAE,0x85,0x58,0x81,0xE4,
0xF7,0xD8,0xFB,0x7C,0x51,0x08,0xC2,0x8E,0x1D,0x22,0x60,0xEE,0x1E,0x0A,0xB4,0x2D,
0x10,0x45,0x5D,0x62,0xC1,0xE0,0x1A,0xE8,0x10,0xB0,0x36,0x01,0x55,0x28,0x94,0xB4,
0x0D,0x4E,0xEB,0x55,0x54,0x01,0x9C,0x43,0xA2,0x6E,0x45,0x07,0x58,0x20,0x1F,0xCC,
0x82,0xD6,0x10,0x53,0xB8,0xB3,0x27,0x84,0x5D,0xE0,0x2D,0x05,0x3C,0x30,0xCF,0x8E,
0xA0,0x40,0xCA,0x02,0x6B,0x11,0xB8,0xB7,0x05,0x86,0x29,0xEC,0xED,0x69,0x5C,0x0F,
0x2D,0xD0,0x0E,0x01,0x43,0x32,0x0E,0x53,0x0E,0x57,0x20,0xFA,0x1F,0x2D,0x68,0x06,
0x50,0xF1,0x85,0x28,0x04,0xA3,0x1F,0x01,0x90,0x8C,0x61,0x1E,0x88,0x1C,0x42,0xC0,
0x1B,0x3D,0xC3,0xB6,0x5F,0xE7,0x4E,0xB8,0x07,0xAE,0x82,0xD3,0x30,0x9E,0x85,0x4E,
0x91,0xF7,0xE8,0x80,0x77,0xFD,0x55,0xEE,0x34,0x9F,0xEA,0xCF,0x2C,0xFE,0x16,0xE0,
0x80,0x13,0xF5,0x1C,0xCE,0xCB,0x40,0x35,0x37,0x98,0xC0,0xF1,0xA7,0x5E,0x24,0xF8,
0xE8,0x59,0xE4,0xC1,0x87,0xF0,0x3C,0x70,0x38,0x5C,0x6A,0x92,0xC4,0x89,0x34,0x19,
0xF7,0xCA,0x33,0xCC,0x4E,0x4D,0xC5,0x74,0x52,0xB6,0x24,0xD7,0xED,0xC5,0xEA,0xE4,
0x43,0xE8,0xD0,0x28,0xD5,0xAD,0xD7,0x81,0x6C,0x7F,0x02,0x26,0x82,0xEC,0xD7,0x41,
0x5B,0x12,0x62,0x3D,0x04,0xDC,0x01,0x02,0x43,0x74,0xCB,0x3A,0x88,0x84,0xB8,0x61,
0x08,0xF6,0x0F,0x2C,0xE5,0x5D,0x04,0x54,0x74,0xDE,0xD5,0xAA,0x33,0x2A,0xC6,0x90,
0xD7,0x81,0xE9,0x10,0xE8,0xDC,0xEC,0x88,0xCD,0xB9,0x0F,0xE2,0x55,0xF3,0x10,0x70,
0xCB,0x1C,0x43,0x0C,0xBC,0xBA,0x4F,0x18,0x86,0x5D,0x7B,0x8C,0x1E,0x05,0xF4,0x8A,
0x55,0xAB,0x33,0xE6,0x97,0xB1,0x53,0xA7,0xF1,0xBC,0x18,0xA4,0x6D,0x45,0x7A,0x70,
0xC2,0xE0,0x08,0x0A,0x44,0xA3,0xA7,0xD1,0x30,0x40,0xAB,0xBB,0xD4,0x6A,0xB5,0xB9,
0xF4,0x34,0x5E,0xBA,0xF3,0xD6,0x95,0xAF,0xCE,0x69,0x24,0x9B,0xF6,0xDE,0x4D,0x22,
0xA5,0xF5,0xF5,0x82,0x48,0x8C,0x48,0x8C,0xB0,0x95,0x23,0xED,0x64,0x5A,0x0A,0xA8,
0x6A,0xD7,0x77,0x24,0xB9,0x8E,0xDC,0x30,0xD1,0xF0,0x49,0x5C,0x35,0x8F,0x5F,0x2A,
0xFA,0xAA,0x7A,0x69,0x20,0x01,0x00,0x53,0xCD,0xDA,0x5E,0x11,0x2B,0xEE,0xA2,0xDA,
0xF9,0xB0,0x88,0xC6,0x91,0x10,0x4B,0x22,0x51,0x20,0x01,0xEC,0xBD,0x0E,0x47,0x3F,
0x82,0xED,0x97,0xA8,0xFB,0x3B,0x54,0x2A,0x95,0x4B,0xFF,0xBE,0x7A,0x75,0xF3,0x50,
0x02,0x4B,0xAF,0x7C,0x7D,0xDE,0x54,0x7F,0xC6,0xBB,0x2B,0x68,0xAD,0x8A,0x6A,0x2C,
0xB5,0x69,0xE7,0x9D,0x4E,0x47,0xA2,0x40,0x1A,0xB8,0x0D,0x7E,0x6C,0x1C,0x51,0xC5,
0xCA,0xB7,0x29,0x14,0x0A,0x8B,0x0B,0x6F,0xBE,0xF9,0x52,0x2F,0xDE,0x01,0x02,0x00,
0xFF,0xF9,0xDD,0x37,0x5F,0xB2,0xA0,0x39,0xC7,0xF5,0x65,0x34,0x08,0x51,0xB1,0xF6,
0x7A,0x6E,0xDA,0x1D,0x89,0x02,0x69,0x60,0x11,0x41,0xDC,0x08,0xE1,0xC8,0x18,0x56,
0xBA,0x49,0xFE,0xF6,0x96,0x1F,0x45,0xD1,0x6C,0x3F,0xAC,0xBE,0x04,0x00,0x96,0x7F,
0xFF,0xAD,0xAC,0x56,0xFC,0x39,0x5B,0xBA,0x8A,0x04,0x51,0x0B,0xDC,0x50,0xE9,0x8E,
0x7E,0x0A,0x88,0x41,0x70,0xE2,0x41,0x28,0xDD,0x60,0x7B,0xEB,0x96,0xDF,0x6C,0x34,
0x66,0x97,0xDE,0x7E,0xBB,0xEF,0x0E,0x7B,0x20,0x01,0x80,0x6B,0x7F,0xFC,0x76,0xD6,
0xAA,0xE5,0x39,0x96,0xFE,0x85,0x34,0x43,0x54,0x41,0xA5,0x3B,0xFA,0x29,0x10,0x7C,
0xE8,0x2C,0xEA,0xDF,0x62,0x3B,0xB7,0xE1,0x37,0x1A,0x8D,0xD9,0xE5,0xE5,0xE5,0x81,
0xDB,0xF5,0x43,0x09,0x00,0xAC,0xFE,0xE9,0x3B,0x59,0xAB,0xF8,0x73,0x2C,0xBD,0x8E,
0x34,0x02,0x54,0x0D,0x91,0x4E,0xF4,0x2A,0x10,0x1C,0x9F,0x42,0xAB,0x05,0xB6,0x6F,
0xAD,0xFB,0x8D,0x46,0x63,0x76,0x75,0x75,0xF5,0xD0,0xB3,0xC2,0x3D,0x09,0x00,0xBC,
0xF3,0x97,0xEF,0x66,0xAD,0x52,0x7E,0xDE,0x2D,0xFC,0x03,0x29,0x57,0xE2,0xCD,0x66,
0xEB,0x15,0x4D,0x9A,0xAA,0x12,0x1C,0x9F,0x22,0xBC,0x5B,0x62,0x7B,0x63,0xC5,0x6F,
0xD4,0xEB,0xB3,0x6B,0x6B,0x6B,0x87,0x82,0x1F,0x99,0x00,0xC0,0xF5,0xBF,0x7D,0xEF,
0x17,0x56,0xBF,0x3B,0xEB,0x96,0x5E,0xF5,0x6D,0x27,0x8F,0x89,0xA1,0x51,0x67,0xD1,
0x09,0x87,0x4F,0x51,0x2F,0x17,0xB8,0xBD,0xBE,0xBC,0xD8,0x68,0x36,0xCF,0xAD,0xAF,
0xAF,0xDF,0x13,0xFC,0x03,0x11,0x00,0x78,0xF7,0xEF,0xDF,0x9F,0x27,0x68,0x3E,0xE1,
0x56,0x5F,0x5B,0xB4,0xCD,0x6B,0xA4,0xF7,0x2A,0xE5,0xDD,0x3C,0xF9,0x1B,0x2B,0x7F,
0x8E,0xA2,0x68,0x76,0xE3,0xC6,0x8D,0x81,0x47,0xBA,0xDE,0x96,0x5E,0x8A,0x5D,0xEB,
0xBC,0xE4,0xF5,0x89,0x4C,0x2A,0xEF,0x03,0x5F,0x3E,0xFB,0xB9,0x97,0x7F,0xE2,0x95,
0xDE,0x7F,0x16,0x60,0x23,0x97,0xA7,0xBA,0x57,0x78,0x79,0x67,0x7B,0xFB,0x97,0xC0,
0x31,0xE7,0xDC,0x04,0xA0,0xA9,0x90,0x9E,0x7E,0x12,0x96,0x9C,0x0D,0xD3,0x40,0x43,
0x7D,0xF2,0x50,0xAA,0xDF,0x8E,0xFB,0xA7,0xBF,0x76,0xFE,0xBE,0xA1,0xC6,0xC5,0xBB,
0xF9,0xD7,0x2E,0x57,0x2A,0x95,0xB5,0x1E,0xC0,0xA8,0x4F,0xEE,0xFD,0x4D,0xBA,0x0E,
0xA7,0x69,0x35,0x52,0xD9,0xF5,0xE9,0xA7,0xC7,0xF4,0x36,0x4B,0x65,0xEB,0xD3,0x4F,
0x8F,0xE1,0xBF,0x43,0x2E,0x8C,0xB3,0x8A,0xF5,0x24,0x98,0x00,0x00,0x00,0x00,0x49,
0x45,0x4E,0x44,0xAE,0x42,0x60,0x82
};
static int protectSize = 1511;