#pragma once

#include "Common/Common.h"
#include "Math/Math.h"

#include <string>
#include <cstring>
#include <vector>
#include <stdarg.h>
#include <memory>

#define STARTING_BUFFER_SIZE 512

static FORCE_INLINE int32 GetVarArgs(char* dest, SIZE_T destSize, int32 count, const char*& fmt, va_list argPtr)
{
	int32 Result = vsnprintf(dest, count, fmt, argPtr);
	va_end(argPtr);
	return Result;
}

#define GET_VARARGS(msg, msgsize, len, lastarg, fmt) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		GetVarArgs(msg, msgsize, len, fmt, ap); \
	}
#define GET_VARARGS_WIDE(msg, msgsize, len, lastarg, fmt) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		GetVarArgs(msg, msgsize, len, fmt, ap); \
	}
#define GET_VARARGS_ANSI(msg, msgsize, len, lastarg, fmt) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		GetVarArgs(msg, msgsize, len, fmt, ap); \
	}
#define GET_VARARGS_RESULT(msg, msgsize, len, lastarg, fmt, result) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		result = GetVarArgs(msg, msgsize, len, fmt, ap); \
		if (result >= msgsize) \
		{ \
			result = -1; \
		} \
	}
#define GET_VARARGS_RESULT_WIDE(msg, msgsize, len, lastarg, fmt, result) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		result = GetVarArgs(msg, msgsize, len, fmt, ap); \
		if (result >= msgsize) \
		{ \
			result = -1; \
		} \
	}
#define GET_VARARGS_RESULT_ANSI(msg, msgsize, len, lastarg, fmt, result) \
	{ \
		va_list ap; \
		va_start(ap, lastarg); \
		result = GetVarArgs(msg, msgsize, len, fmt, ap); \
		if (result >= msgsize) \
		{ \
			result = -1; \
		} \
	}

struct StringUtils
{
	static std::string Printf(const char* fmt, ...)
	{
		int32 bufferSize = STARTING_BUFFER_SIZE;
		char  startingBuffer[STARTING_BUFFER_SIZE];
		char* buffer = startingBuffer;
		int32 result = -1;

		GET_VARARGS_RESULT(buffer, bufferSize, bufferSize - 1, fmt, fmt, result);

		if (result == -1)
		{
			buffer = nullptr;
			while (result == -1)
			{
				bufferSize *= 2;
				buffer = (char*)realloc(buffer, bufferSize * sizeof(char));
				GET_VARARGS_RESULT(buffer, bufferSize, bufferSize - 1, fmt, fmt, result);
			};
		}

		buffer[result] = 0;
		std::string resultString(buffer);

		if (bufferSize != STARTING_BUFFER_SIZE)
		{
			free(buffer);
		}

		return resultString;
	}

	static void AddUnique(std::vector<std::string>& arr, const std::string& val)
	{
		bool found = false;
		for (int32 i = 0; i < arr.size(); ++i) {
			if (arr[i] == val) {
				found = true;
				break;
			}
		}
		if (!found) {
			arr.push_back(val);
		}
	}

	static void AddUnique(std::vector<const char*>& arr, const char* val)
	{
		bool found = false;
		for (int32 i = 0; i < arr.size(); ++i) {
			if (strcmp(arr[i], val) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			arr.push_back(val);
		}
	}

	static char NibbleToTChar(uint8 num)
	{
		if (num > 9)
		{
			return 'A' + char(num - 10);
		}
		return '0' + char(num);
	}

	static void ByteToHex(uint8 val, std::string& result)
	{
		result += NibbleToTChar(val >> 4);
		result += NibbleToTChar(val & 15);
	}
    
	static std::string BytesToHex(const uint8* bytes, int32 count)
	{
		std::string result;
		result.resize(count * 2);

		while (count)
		{
			ByteToHex(*bytes++, result);
			count--;
		}
		return result;
	}

	static const uint8 CharToNibble(const char c)
	{
		if (c >= '0' && c <= '9')
		{
			return c - '0';
		}
		else if (c >= 'A' && c <= 'F')
		{
			return (c - 'A') + 10;
		}
		return (c - 'a') + 10;
	}

	static int32 HexToBytes(const std::string& hexString, uint8* outBytes)
	{
		int32 numBytes = 0;
		const bool padNibble = (hexString.size() % 2) == 1;
		const char* charPos  = hexString.c_str();
		if (padNibble)
		{
			outBytes[numBytes++] = CharToNibble(*charPos++);
		}
		while (*charPos)
		{
			outBytes[numBytes] = CharToNibble(*charPos++) << 4;
			outBytes[numBytes] += CharToNibble(*charPos++);
			++numBytes;
		}
		return numBytes;
	}
    
};


