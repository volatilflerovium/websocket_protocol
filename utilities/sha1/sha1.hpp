/*
    sha1.hpp - source code of

    ============
    SHA-1 in C++
    ============

    100% Public Domain.

    Original C Code
        -- Steve Reid <steve@edmweb.com>
    Small changes to fit into bglibs
        -- Bruce Guenter <bruce@untroubled.org>
    Translation to simpler C++ Code
        -- Volker Diels-Grabsch <v@njh.eu>
    Safety fixes
        -- Eugene Hopkinson <slowriot at voxelstorm dot com>
    Header-only library
        -- Zlatko Michailov <zlatko@michailov.org>
    C++ optimization
        -- Dan Machado <dan-machado@yandex.com>
*/

#ifndef SHA1_HPP
#define SHA1_HPP

#include <string>
#include <cstring>
#include <cstdio>

#if __has_include(<format>)
	#include <format>
	#define HAS_FORMAT_HEADER
#else
	#include <iomanip>
	#include <sstream>
#endif

//====================================================================

class SHA1
{
	static const size_t BLOCK_INTS = 16;  /* number of 32bit integers per SHA1 block */
	static const size_t BLOCK_BYTES = BLOCK_INTS * 4;
	static const size_t DIGEST_SIZE = 5;

	public:
		SHA1();
		~SHA1()=default;

		/**
		 * Partially calculate the sha1sum of its argument. The calculation
		 * is completed by calling final(). Consecutive calls to update()
		 * will have the effect of calculating the sha1 sum of the concatenated
		 * string of its arguments. For example:
		 * SHA1 sha1;
		 * sha1.update(str1);
		 * sha1.update(str2);
		 * sha1.update(str3);
		 * sha1.final();
		 * is the same than
		 * sha1.update(str1 + str2 + str3);
		 * sha1.final();
		 * which is the same than
		 * sha1(str1 + str2 + str3) (function operator)
		 *
		 * @param cstr null terminated string
		 *
		 * */
		void update(const char* cstr, size_t strLength=0);
		void update(const std::string& str);
		void update(const std::string&& str);

		/**
		 * Complete the calculation of the sha1sum initiated by calls to
		 * SHA1::update.
		 *
		 * @return return the sha1sum of the string (or strings)
		 *         fed by calls to SHA1::update.
		 * */
		std::string final();

	private:
		uint32_t m_dataBlock[BLOCK_INTS];
		uint8_t m_buffer[BLOCK_BYTES];
		uint32_t digest[DIGEST_SIZE];
		uint32_t m_digest[DIGEST_SIZE];
		uint64_t transforms;
		size_t m_dataSize;
		int m_lastError;
		const uint8_t* m_bufferPtr;

		void reset();		
		void buffer_to_block();
		
		template<int N, int M>
		uint32_t rol(uint32_t value);

		void blk(size_t i);

		/*
		* R0, R1, R2, R3, R4 are the different operations used in SHA1
		*/
		template<int v, int w, int x, int y, int z>
		void R0(const size_t i);
		
		template<int v, int w, int x, int y, int z>
		void R1(const size_t i);
		
		template<int v, int w, int x, int y, int z>
		void R2(const size_t i);
		
		template<int v, int w, int x, int y, int z>
		void R3(const size_t i);
		
		template<int v, int w, int x, int y, int z>
		void R4(const size_t i);

		void computeHash(const char* dataPtr, const size_t dataSize);

		std::string fileSha1Sum(const char* fileName);

		/*
		* Hash a single 512-bit block. This is the core of the algorithm.
		*/
		void transform();
};

//====================================================================

inline SHA1::SHA1()
{
	reset();
}

inline void SHA1::update(const char* cstr, size_t strLength)
{
	if(strLength==0){
		strLength=std::strlen(cstr);
	}
	computeHash(cstr, strLength);
}

inline void SHA1::update(const std::string& str)
{
	computeHash(str.c_str(), str.length());	
}

inline void SHA1::update(const std::string&& str)
{
	computeHash(str.c_str(), str.length());	
}

inline void SHA1::reset()
{
	/* SHA1 initialization constants */
	digest[0] = 0x67452301;
	digest[1] = 0xefcdab89;
	digest[2] = 0x98badcfe;
	digest[3] = 0x10325476;
	digest[4] = 0xc3d2e1f0;
	
	/* Reset counters */
	transforms = 0;
	m_dataSize=0;
	m_bufferPtr=m_buffer;
}

template<int N, int M>
inline uint32_t SHA1::rol(uint32_t value)
{
	return (value << N) | (value >> M);
}

inline void SHA1::blk(size_t i)
{
	 m_dataBlock[i]=rol<1, 31>(m_dataBlock[(i+13)&15] ^ m_dataBlock[(i+8)&15] ^ m_dataBlock[(i+2)&15] ^ m_dataBlock[i]);
}

template<int v, int w, int x, int y, int z>
inline void SHA1::R0(const size_t i)
{
	m_digest[z] =m_digest[z]+ ((m_digest[w]&(m_digest[x]^m_digest[y]))^m_digest[y]) + m_dataBlock[i] + 0x5a827999 + rol<5, 27>(m_digest[v]);
	m_digest[w] = rol<30, 2>(m_digest[w]);
}

template<int v, int w, int x, int y, int z>
inline void SHA1::R1(const size_t i)
{
	blk(i);
	R0<v,w,x,y,z>(i);
}

template<int v, int w, int x, int y, int z>
inline void SHA1::R2(const size_t i)
{
	blk(i);
	m_digest[z] =m_digest[z]+ (m_digest[w]^m_digest[x]^m_digest[y]) + m_dataBlock[i] + 0x6ed9eba1 + rol<5, 27>(m_digest[v]);
	m_digest[w] = rol<30, 2>(m_digest[w]);
}

template<int v, int w, int x, int y, int z>
inline void SHA1::R3(const size_t i)
{
	blk(i);
	m_digest[z] =m_digest[z]+ (((m_digest[w]|m_digest[x])&m_digest[y])|(m_digest[w]&m_digest[x])) + m_dataBlock[i] + 0x8f1bbcdc + rol<5, 27>(m_digest[v]);
	m_digest[w] = rol<30, 2>(m_digest[w]);
}

template<int v, int w, int x, int y, int z>
inline void SHA1::R4(const size_t i)
{
	blk(i);
	m_digest[z] =m_digest[z]+ (m_digest[w]^m_digest[x]^m_digest[y]) + m_dataBlock[i] + 0xca62c1d6 + rol<5, 27>(m_digest[v]);
	m_digest[w] = rol<30, 2>(m_digest[w]);
}

inline void SHA1::computeHash(const char* dataPtr, size_t dataSize)
{
	size_t idx=0;
	size_t newDataSize=dataSize;
	size_t copyB=BLOCK_BYTES-m_dataSize;

	auto update=[&idx, &copyB, &newDataSize, this](){
		idx=idx+copyB;
		newDataSize=newDataSize-copyB;
		m_dataSize=m_dataSize+copyB;
	};
	
	while(true){
		if(copyB>newDataSize){
			copyB=newDataSize;
			m_bufferPtr=m_buffer;
			std::memcpy(m_buffer+m_dataSize, dataPtr+idx, copyB);

			if(m_dataSize!=BLOCK_BYTES){
				update();
				break;
			}			
		}
		else{//copyB==BLOCK_BYTES
			m_bufferPtr=reinterpret_cast<const uint8_t*>(dataPtr+idx);
		}
		update();
		buffer_to_block();	  
		transform();

		copyB=BLOCK_BYTES;
		m_dataSize=0;
	}	
}

inline void SHA1::transform()
{
	std::memcpy(m_digest, digest, DIGEST_SIZE*sizeof(uint32_t));
	
	/* 4 rounds of 20 operations each. Loop unrolled. */
	R0<0, 1, 2, 3, 4>( 0);
	R0<4, 0, 1, 2, 3>( 1);
	R0<3, 4, 0, 1, 2>( 2);
	R0<2, 3, 4, 0, 1>( 3);
	R0<1, 2, 3, 4, 0>( 4);
	R0<0, 1, 2, 3, 4>( 5);
	R0<4, 0, 1, 2, 3>( 6);
	R0<3, 4, 0, 1, 2>( 7);
	R0<2, 3, 4, 0, 1>( 8);
	R0<1, 2, 3, 4, 0>( 9);
	R0<0, 1, 2, 3, 4>(10);
	R0<4, 0, 1, 2, 3>(11);
	R0<3, 4, 0, 1, 2>(12);
	R0<2, 3, 4, 0, 1>(13);
	R0<1, 2, 3, 4, 0>(14);
	R0<0, 1, 2, 3, 4>(15);
	R1<4, 0, 1, 2, 3>( 0);
	R1<3, 4, 0, 1, 2>( 1);
	R1<2, 3, 4, 0, 1>( 2);
	R1<1, 2, 3, 4, 0>( 3);
	R2<0, 1, 2, 3, 4>( 4);
	R2<4, 0, 1, 2, 3>( 5);
	R2<3, 4, 0, 1, 2>( 6);
	R2<2, 3, 4, 0, 1>( 7);
	R2<1, 2, 3, 4, 0>( 8);
	R2<0, 1, 2, 3, 4>( 9);
	R2<4, 0, 1, 2, 3>(10);
	R2<3, 4, 0, 1, 2>(11);
	R2<2, 3, 4, 0, 1>(12);
	R2<1, 2, 3, 4, 0>(13);
	R2<0, 1, 2, 3, 4>(14);
	R2<4, 0, 1, 2, 3>(15);
	R2<3, 4, 0, 1, 2>( 0);
	R2<2, 3, 4, 0, 1>( 1);
	R2<1, 2, 3, 4, 0>( 2);
	R2<0, 1, 2, 3, 4>( 3);
	R2<4, 0, 1, 2, 3>( 4);
	R2<3, 4, 0, 1, 2>( 5);
	R2<2, 3, 4, 0, 1>( 6);
	R2<1, 2, 3, 4, 0>( 7);
	R3<0, 1, 2, 3, 4>( 8);
	R3<4, 0, 1, 2, 3>( 9);
	R3<3, 4, 0, 1, 2>(10);
	R3<2, 3, 4, 0, 1>(11);
	R3<1, 2, 3, 4, 0>(12);
	R3<0, 1, 2, 3, 4>(13);
	R3<4, 0, 1, 2, 3>(14);
	R3<3, 4, 0, 1, 2>(15);
	R3<2, 3, 4, 0, 1>( 0);
	R3<1, 2, 3, 4, 0>( 1);
	R3<0, 1, 2, 3, 4>( 2);
	R3<4, 0, 1, 2, 3>( 3);
	R3<3, 4, 0, 1, 2>( 4);
	R3<2, 3, 4, 0, 1>( 5);
	R3<1, 2, 3, 4, 0>( 6);
	R3<0, 1, 2, 3, 4>( 7);
	R3<4, 0, 1, 2, 3>( 8);
	R3<3, 4, 0, 1, 2>( 9);
	R3<2, 3, 4, 0, 1>(10);
	R3<1, 2, 3, 4, 0>(11);
	R4<0, 1, 2, 3, 4>(12);
	R4<4, 0, 1, 2, 3>(13);
	R4<3, 4, 0, 1, 2>(14);
	R4<2, 3, 4, 0, 1>(15);
	R4<1, 2, 3, 4, 0>( 0);
	R4<0, 1, 2, 3, 4>( 1);
	R4<4, 0, 1, 2, 3>( 2);
	R4<3, 4, 0, 1, 2>( 3);
	R4<2, 3, 4, 0, 1>( 4);
	R4<1, 2, 3, 4, 0>( 5);
	R4<0, 1, 2, 3, 4>( 6);
	R4<4, 0, 1, 2, 3>( 7);
	R4<3, 4, 0, 1, 2>( 8);
	R4<2, 3, 4, 0, 1>( 9);
	R4<1, 2, 3, 4, 0>(10);
	R4<0, 1, 2, 3, 4>(11);
	R4<4, 0, 1, 2, 3>(12);
	R4<3, 4, 0, 1, 2>(13);
	R4<2, 3, 4, 0, 1>(14);
	R4<1, 2, 3, 4, 0>(15);

	/* Add the working vars back into digest[] */
	digest[0] =digest[0]+ m_digest[0];
	digest[1] =digest[1]+ m_digest[1];
	digest[2] =digest[2]+ m_digest[2];
	digest[3] =digest[3]+ m_digest[3];
	digest[4] =digest[4]+ m_digest[4];
	
	/* Count the number of transformations */
	transforms++;
}

inline void SHA1::buffer_to_block()
{
	m_dataBlock[0] = m_bufferPtr[3] | m_bufferPtr[2]<<8 | m_bufferPtr[1]<<16 | m_bufferPtr[0]<<24;
	m_dataBlock[1] = m_bufferPtr[7] | m_bufferPtr[6]<<8 | m_bufferPtr[5]<<16 | m_bufferPtr[4]<<24;
	m_dataBlock[2] = m_bufferPtr[11] | m_bufferPtr[10]<<8 | m_bufferPtr[9]<<16 | m_bufferPtr[8]<<24;
	m_dataBlock[3] = m_bufferPtr[15] | m_bufferPtr[14]<<8 | m_bufferPtr[13]<<16 | m_bufferPtr[12]<<24;
	m_dataBlock[4] = m_bufferPtr[19] | m_bufferPtr[18]<<8 | m_bufferPtr[17]<<16 | m_bufferPtr[16]<<24;
	m_dataBlock[5] = m_bufferPtr[23] | m_bufferPtr[22]<<8 | m_bufferPtr[21]<<16 | m_bufferPtr[20]<<24;
	m_dataBlock[6] = m_bufferPtr[27] | m_bufferPtr[26]<<8 | m_bufferPtr[25]<<16 | m_bufferPtr[24]<<24;
	m_dataBlock[7] = m_bufferPtr[31] | m_bufferPtr[30]<<8 | m_bufferPtr[29]<<16 | m_bufferPtr[28]<<24;
	m_dataBlock[8] = m_bufferPtr[35] | m_bufferPtr[34]<<8 | m_bufferPtr[33]<<16 | m_bufferPtr[32]<<24;
	m_dataBlock[9] = m_bufferPtr[39] | m_bufferPtr[38]<<8 | m_bufferPtr[37]<<16 | m_bufferPtr[36]<<24;
	m_dataBlock[10] = m_bufferPtr[43] | m_bufferPtr[42]<<8 | m_bufferPtr[41]<<16 | m_bufferPtr[40]<<24;
	m_dataBlock[11] = m_bufferPtr[47] | m_bufferPtr[46]<<8 | m_bufferPtr[45]<<16 | m_bufferPtr[44]<<24;
	m_dataBlock[12] = m_bufferPtr[51] | m_bufferPtr[50]<<8 | m_bufferPtr[49]<<16 | m_bufferPtr[48]<<24;
	m_dataBlock[13] = m_bufferPtr[55] | m_bufferPtr[54]<<8 | m_bufferPtr[53]<<16 | m_bufferPtr[52]<<24;
	m_dataBlock[14] = m_bufferPtr[59] | m_bufferPtr[58]<<8 | m_bufferPtr[57]<<16 | m_bufferPtr[56]<<24;
	m_dataBlock[15] = m_bufferPtr[63] | m_bufferPtr[62]<<8 | m_bufferPtr[61]<<16 | m_bufferPtr[60]<<24;
}

/*
 * Add padding and return the message digest.
 */
inline std::string SHA1::final()
{
	/* Total number of hashed bits */
	uint64_t total_bits = (transforms*BLOCK_BYTES + m_dataSize) * 8;

	if(m_dataSize<BLOCK_BYTES){
		std::memset(m_buffer+m_dataSize, 0, BLOCK_BYTES-m_dataSize);
		m_buffer[m_dataSize]=static_cast<char>(0x80);
		buffer_to_block();
	}

	if (m_dataSize+1 > BLOCK_BYTES - 8){
		transform();
		std::memset(m_dataBlock, 0, sizeof(uint32_t)*(BLOCK_INTS - 2));
	}

	/* Append total_bits, split this uint64_t into two uint32_t */
	m_dataBlock[BLOCK_INTS - 1] = static_cast<uint32_t>(total_bits);
	m_dataBlock[BLOCK_INTS - 2] = static_cast<uint32_t>(total_bits >> 32);
	transform();

	/* Hex std::string */
	#ifdef HAS_FORMAT_HEADER
		std::string hexStr=std::format("{:8x}{:8x}{:8x}{:8x}{:8x}", digest[0], digest[1], digest[2], digest[3], digest[4]);
		reset();
		return hexStr;
	#else	
		std::ostringstream oHexSs;
		oHexSs<<std::hex<<std::setfill('0')<<std::setw(8)<<digest[0];
		oHexSs<<std::hex<<std::setfill('0')<<std::setw(8)<<digest[1];
		oHexSs<<std::hex<<std::setfill('0')<<std::setw(8)<<digest[2];
		oHexSs<<std::hex<<std::setfill('0')<<std::setw(8)<<digest[3];
		oHexSs<<std::hex<<std::setfill('0')<<std::setw(8)<<digest[4];
		reset();
		return oHexSs.str();
	#endif
}

//====================================================================

#endif /* SHA1_HPP */
