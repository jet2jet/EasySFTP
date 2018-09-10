/*
 EasySFTP - Copyright (C) 2010 Kuri-Applications

 TextStrm.h - declarations of text mode flags (TEXTMODE_XXX)
 */

#pragma once

// TEXTMODE_BUFFER_XX, TEXTMODE_STREAM_YY ...
//   IStream::Read()  : Reads YY as return code, and copies XX as return code to the buffer
//   IStream::Write() : Gets XX as return code from the buffer, and writes YY as return code
//   (if XX and YY are the same, no convertion will be applied)
#define TEXTMODE_NO_CONVERT   0x00
#define TEXTMODE_BUFFER_CRLF  0x00
#define TEXTMODE_BUFFER_CR    0x01
#define TEXTMODE_BUFFER_LF    0x02
#define TEXTMODE_BUFFER_MASK  0x03
#define TEXTMODE_STREAM_CRLF  0x00
#define TEXTMODE_STREAM_CR    0x04
#define TEXTMODE_STREAM_LF    0x08
#define TEXTMODE_STREAM_MASK  0x0C
#define TEXTMODE_RETURN_MASK  (TEXTMODE_BUFFER_MASK | TEXTMODE_STREAM_MASK)
#define TEXTMODE_NONE         0x00
#define TEXTMODE_SHIFT_JIS    0x10
#define TEXTMODE_UTF8         0x20
#define TEXTMODE_EUC_JP       0x30
#define TEXTMODE_UCS4         0x40
#define TEXTMODE_ENCODE_MASK  0xF0
#define TEXTMODE_BUFFER_TO_STREAM_NO_ENCODE(f) \
	((f & TEXTMODE_BUFFER_MASK) << 2)
#define TEXTMODE_STREAM_TO_BUFFER_NO_ENCODE(f) \
	((f & TEXTMODE_STREAM_MASK) >> 2)
#define TEXTMODE_CHANGE_BUFFER_AND_STREAM_NO_ENCODE(f) \
	(TEXTMODE_BUFFER_TO_STREAM_NO_ENCODE(f) | TEXTMODE_STREAM_TO_BUFFER_NO_ENCODE(f))
#define TEXTMODE_IS_NO_CONVERTION(f) \
	(!f || (f & TEXTMODE_BUFFER_MASK) == TEXTMODE_STREAM_TO_BUFFER_NO_ENCODE(f))
