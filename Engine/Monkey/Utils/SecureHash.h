#pragma once

#include "StringUtils.h"

#include "Common/Common.h"
#include "Common/Log.h"

#include <string>

class MD5
{
public:
	MD5();
    
	~MD5();

	void Update(const uint8* input, int32 inputLen);

	void Final(uint8* digest);

	static std::string HashAnsiString(const char* chars)
	{
		uint8 digest[16];

		MD5 md5Gen;
		md5Gen.Update((unsigned char*)(chars), int32(std::strlen(chars)));
		md5Gen.Final(digest);

		std::string md5;
		for (int32 i = 0; i < 16; i++)
		{
			md5 += StringUtils::Printf("%02x", digest[i]);
		}

		return md5;
	}

private:

	struct Context
	{
		uint32 state[4];
		uint32 count[2];
		uint8 buffer[64];
	};

	void Transform(uint32* state, const uint8* block);

	void Encode(uint8* output, const uint32* input, int32 len);

	void Decode(uint32* output, const uint8* input, int32 len);

	Context m_Context;
};

struct MD5Hash
{
	MD5Hash() 
		: m_IsValid(false) 
	{

	}

	bool IsValid() const 
	{ 
		return m_IsValid;
	}

	void Set(MD5& md5)
	{
		md5.Final(m_Bytes);
		m_IsValid = true;
	}

	friend bool operator==(const MD5Hash& lhs, const MD5Hash& rhs)
	{
		return lhs.m_IsValid == rhs.m_IsValid && (!lhs.m_IsValid || std::memcmp(lhs.m_Bytes, rhs.m_Bytes, 16) == 0);
	}

	friend bool operator!=(const MD5Hash& lhs, const MD5Hash& rhs)
	{
		return lhs.m_IsValid != rhs.m_IsValid || (lhs.m_IsValid && std::memcmp(lhs.m_Bytes, rhs.m_Bytes, 16) != 0);
	}

	const uint8* GetBytes() const
	{
		return m_Bytes;
	}

	const int32 GetSize() const
	{
		return sizeof(m_Bytes);
	}

private:
	bool  m_IsValid;
	uint8 m_Bytes[16];
};

typedef union
{
	uint8  c[64];
	uint32 l[16];
} SHA1_WORKSPACE_BLOCK;

class SHAHash
{
public:
	uint8 hash[20];

	SHAHash()
	{
		std::memset(hash, 0, sizeof(hash));
	}

	FORCE_INLINE std::string ToString() const
	{
		return StringUtils::BytesToHex((const uint8*)hash, sizeof(hash));
	}
	
	void FromString(const std::string& src)
	{
		StringUtils::HexToBytes(src, hash);
	}

	friend bool operator==(const SHAHash& lhs, const SHAHash& rhs)
	{
		return std::memcmp(&lhs.hash, &rhs.hash, sizeof(lhs.hash)) == 0;
	}

	friend bool operator!=(const SHAHash& lhs, const SHAHash& rhs)
	{
		return std::memcmp(&lhs.hash, &rhs.hash, sizeof(lhs.hash)) != 0;
	}

};
