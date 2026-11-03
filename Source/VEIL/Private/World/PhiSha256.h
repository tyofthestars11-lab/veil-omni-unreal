#pragma once

#include "CoreMinimal.h"

// SHA-256, because the project's world proof is a hash the Python simulation also prints.
// The engine ships SHA-1, not SHA-256, and a "proof" that only C++ can reproduce proves
// nothing - so the digest is the real FIPS 180-4 algorithm and is checked against the
// simulation's d8e824a25176a3b9 every boot.
namespace PhiSha
{
	struct FState
	{
		uint32 H[8];
		uint64 Bits;
		uint8 Block[64];
		int32 Used;
	};

	inline uint32 Ror(uint32 X, uint32 N) { return (X >> N) | (X << (32 - N)); }

	inline void Compress(FState& S, const uint8* P)
	{
		static const uint32 K[64] = {
			0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
			0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
			0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
			0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
			0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
			0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
			0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
			0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u };
		uint32 W[64];
		for (int32 i = 0; i < 16; ++i)
		{
			W[i] = ((uint32)P[i * 4] << 24) | ((uint32)P[i * 4 + 1] << 16) | ((uint32)P[i * 4 + 2] << 8) | (uint32)P[i * 4 + 3];
		}
		for (int32 i = 16; i < 64; ++i)
		{
			const uint32 S0 = Ror(W[i - 15], 7) ^ Ror(W[i - 15], 18) ^ (W[i - 15] >> 3);
			const uint32 S1 = Ror(W[i - 2], 17) ^ Ror(W[i - 2], 19) ^ (W[i - 2] >> 10);
			W[i] = W[i - 16] + S0 + W[i - 7] + S1;
		}
		uint32 a = S.H[0], b = S.H[1], c = S.H[2], d = S.H[3];
		uint32 e = S.H[4], f = S.H[5], g = S.H[6], h = S.H[7];
		for (int32 i = 0; i < 64; ++i)
		{
			const uint32 S1 = Ror(e, 6) ^ Ror(e, 11) ^ Ror(e, 25);
			const uint32 Ch = (e & f) ^ (~e & g);
			const uint32 T1 = h + S1 + Ch + K[i] + W[i];
			const uint32 S0 = Ror(a, 2) ^ Ror(a, 13) ^ Ror(a, 22);
			const uint32 Maj = (a & b) ^ (a & c) ^ (b & c);
			const uint32 T2 = S0 + Maj;
			h = g; g = f; f = e; e = d + T1; d = c; c = b; b = a; a = T1 + T2;
		}
		S.H[0] += a; S.H[1] += b; S.H[2] += c; S.H[3] += d;
		S.H[4] += e; S.H[5] += f; S.H[6] += g; S.H[7] += h;
	}

	inline FString Hex(const FString& Text)
	{
		FState S;
		S.H[0] = 0x6a09e667u; S.H[1] = 0xbb67ae85u; S.H[2] = 0x3c6ef372u; S.H[3] = 0xa54ff53au;
		S.H[4] = 0x510e527fu; S.H[5] = 0x9b05688cu; S.H[6] = 0x1f83d9abu; S.H[7] = 0x5be0cd19u;
		S.Bits = 0; S.Used = 0;

		const FTCHARToUTF8 Utf8(*Text);
		const uint8* Data = (const uint8*)Utf8.Get();
		const int32 Len = Utf8.Length();
		for (int32 i = 0; i < Len; ++i)
		{
			S.Block[S.Used++] = Data[i];
			if (S.Used == 64) { Compress(S, S.Block); S.Bits += 512; S.Used = 0; }
		}
		S.Bits += (uint64)S.Used * 8;
		S.Block[S.Used++] = 0x80;
		if (S.Used > 56)
		{
			while (S.Used < 64) { S.Block[S.Used++] = 0; }
			Compress(S, S.Block);
			S.Used = 0;
		}
		while (S.Used < 56) { S.Block[S.Used++] = 0; }
		for (int32 i = 7; i >= 0; --i) { S.Block[S.Used++] = (uint8)((S.Bits >> (i * 8)) & 0xff); }
		Compress(S, S.Block);

		FString Out;
		for (int32 i = 0; i < 8; ++i) { Out += FString::Printf(TEXT("%08x"), S.H[i]); }
		return Out;
	}
}
