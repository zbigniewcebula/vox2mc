#ifndef __VOX__
#define __VOX__

#include <string>
#include <fstream>
#include <cstring>
#include <cmath>

// #include <xmmintrin.h>
// #include <smmintrin.h>

using std::vector;
using std::ifstream;
using std::ofstream;

typedef unsigned char	uchar;

template<typename T>
class vec {
	public:
		union {
			struct {
				T r, g, b, a;
			};
			struct {
				T x, y, z, w;
			};

			T		raw[4];
		};

		vec(T R = 0, T G = 0, T B = 0, T A = 0)
			:	r(R), g(G), b(B), a(A)
		{}
		vec(const vec<T>& org)
			:	r(org.r), g(org.g), b(org.b), a(org.a)
		{}

		~vec()
		{}

		//Methods
		vec<T>& Set(T R, T G, T B, T A) {
			r	= R;
			g	= G;
			b	= B;
			a	= A;
			return *this;
		}
		vec<T>& Set(T R, T G, T B) {
			r	= R;
			g	= G;
			b	= B;
			return *this;
		}
		vec<T>& Set(vec<T>& other) {
			r	= other.r;
			g	= other.g;
			b	= other.b;
			a	= other.a;
			return *this;
		}

		vec<T> NormalizedCross(const vec<T>& b) {
// #ifdef __SSE__
// 			__m128 temp = _mm_load_ps(raw);
// 			__m128 _b = _mm_load_ps(b.raw);
// 			temp = _mm_sub_ps(
// 				_mm_mul_ps(
// 					_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(3, 0, 2, 1)),
// 					_mm_shuffle_ps(_b, _b, _MM_SHUFFLE(3, 1, 0, 2))
// 				), 
// 				_mm_mul_ps(
// 					_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(3, 1, 0, 2)),
// 					_mm_shuffle_ps(_b, _b, _MM_SHUFFLE(3, 0, 2, 1))
// 				)
// 			);
// 			__m128 inverse_norm = _mm_rsqrt_ps(_mm_dp_ps(temp, temp, 0x77));
			
// 			float	ret[4];
// 			_mm_store_ps(ret, _mm_mul_ps(temp, inverse_norm));
//  			return vec<T>(ret[0], ret[1], ret[2], ret[3]);
// #else
 			return Cross(b).Normalized();
// #endif
		}		

		vec<T> Cross(const vec<T>& b) {
// #ifdef __SSE__
// 			__m128 temp = _mm_load_ps(raw);
// 			__m128 _b = _mm_load_ps(b.raw);
// 			float	ret[4];
// 			_mm_store_ps(ret, _mm_sub_ps(
// 				_mm_mul_ps(
// 					_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(3, 0, 2, 1)),
// 					_mm_shuffle_ps(_b, _b, _MM_SHUFFLE(3, 1, 0, 2))
// 				), 
// 				_mm_mul_ps(
// 					_mm_shuffle_ps(temp, temp, _MM_SHUFFLE(3, 1, 0, 2)),
// 					_mm_shuffle_ps(_b, _b, _MM_SHUFFLE(3, 0, 2, 1))
// 				)
// 			));
//  			return vec<T>(ret[0], ret[1], ret[2], ret[3]);
// #else
			vec<T> result(
				this->y * b.z - this->z * b.y,
				this->z * b.x - this->x * b.z,
				this->x * b.y - this->y * b.x
			);
			return result;
// #endif
		}
		vec<T> Normalized() {
// #ifdef __SSE__
// 			__m128 temp = _mm_load_ps(raw);
// 			__m128 inverse_norm = _mm_rsqrt_ps(_mm_dp_ps(temp, temp, 0x77));
//  			float	ret[4];
//  			return _mm_store_ps(ret, _mm_mul_ps(temp, inverse_norm));
// #else
			T len	= Length();
			vec<T> result(
				this->y / len,
				this->z / len,
				this->x / len
			);
			return result;
// #endif
		}
		inline T Length() {
// #ifdef __SSE__
// 			__m128 temp = _mm_load_ps(raw);
// 			return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(temp, temp, 0x71)));
// #else
			return (T)sqrt(x * x + y * y + z * z);
// #endif
		}

		T Distance(vec<T>& other) {
			return (T)(other - (*this)).Length();
		}

		//Logic
		inline bool operator==(const vec<T>& rhs) const {
			return z == rhs.z and y == rhs.y and x == rhs.x;
		}

		//Artmetic
		inline vec<T> operator+(const vec<T>& rhs) const {
			return vec<T>(x + rhs.x, y + rhs.y, z + rhs.z);
		}
		inline vec<T> operator-(const vec<T>& rhs) const {
			return vec<T>(x - rhs.x, y - rhs.y, z - rhs.z);
		}
		inline vec<T> operator*(const vec<T>& rhs) const {
			return vec<T>(x * rhs.x, y * rhs.y, z * rhs.z);
		}

		//by Scalar
		inline vec<T> operator*(const T rhs) const {
			return vec<T>(z * rhs, y * rhs, x * rhs);
		}
};
template<typename T, typename K>
inline vec<K> operator+(const vec<T>& lhs, const vec<K>& rhs) {
	return vec<K>(lhs.z + rhs.z, lhs.y + rhs.y, lhs.x + rhs.x);
}
template<typename T, typename K>
inline vec<K> operator*(const vec<T>& lhs, K rhs) {
	return vec<K>(lhs.z * rhs, lhs.y * rhs, lhs.x * rhs);
}

class VOX {
	private:
		vec<int>	size;
		vec<uchar>	palette[256];
		uchar*		voxel;

		int			version			= 0;
	public:
		VOX()
			: voxel(nullptr)
		{}
		VOX(int sizeX, int sizeY, int sizeZ)
			: voxel(nullptr)
		{
			Alloc(sizeX, sizeY, sizeZ);
		}
		VOX(vec<int> setSize)
			: voxel(nullptr)
		{
			Alloc(setSize.x, setSize.y, setSize.z);
		}
		VOX(string path)
			: voxel(nullptr)
		{
			if(not LoadFile(path))
				throw false;
		}
		~VOX() {
			if(voxel not_eq nullptr)
				delete[]	voxel;
		}
		
		inline int SizeX() {
			return size.x;
		}
		inline int SizeY() {
			return size.y;
		}
		inline int SizeZ() {
			return size.z;
		}
		inline vec<int> Size() {
			return size;
		}

		inline bool LoadFile(string path) {
			return LoadFile(path.c_str());
		}
		bool LoadFile(const char* path) {
			ifstream	hFile(path, std::ios::in bitor std::ios::binary);

			if(hFile.fail()) {
				cerr	<< "[VOX] Failed to open file!" << endl;
				return false;
			}
			bool success = ReadFile(hFile);

			hFile.close();
			return success;
		}

		inline bool SaveFile(string path) {
			return SaveFile(path.c_str());
		}
		bool SaveFile(const char* name = "") {
			//File
			ofstream	hFile(name, std::ios::trunc bitor std::ios::out bitor std::ios::binary);

			if(hFile.fail()) {
				hFile.close();
				return false;
			}
			bool success = WriteFile(hFile);

			hFile.close();
			return success;
		}

		inline void SetVoxel(vec<int>& pos, uchar colorPalleteIndex) {
			SetVoxel(pos.x, pos.y, pos.z, colorPalleteIndex);
		}
		void SetVoxel(int x, int y, int z, uchar colorPalleteIndex) {
			if(voxel not_eq nullptr
			and x > -1 and y > -1 and z > -1 
			and x < size.x and y < size.y and z < size.z 
			) {
				SetVoxelRaw(x, y, z, colorPalleteIndex);
			}
		}
		inline void SetVoxelRaw(int x, int y, int z, uchar colorPalleteIndex) {
			voxel[x + size.x * (y + size.y * z)] 	= colorPalleteIndex;
		}
		inline void SetVoxelRaw(vec<int> pos, uchar colorPalleteIndex) {
			SetVoxelRaw(pos.x, pos.y, pos.z, colorPalleteIndex);
		}

		uchar GetVoxel(int x, int y, int z) {
			if(voxel == nullptr
			or x < 0 or y < 0 or z < 0
			or x >= size.x or y >= size.y or z >= size.z 
			) {
				return 0;
			}
			return GetVoxelRaw(x, y, z);
		}
		inline uchar GetVoxel(vec<int> pos) {
			return GetVoxel(pos.x, pos.y, pos.z);
		}
		inline uchar GetVoxelRaw(int x, int y, int z) {
			return voxel[x + size.x * (y + size.y * z)];
		}
		inline uchar GetVoxelRaw(vec<int> pos) {
			return GetVoxelRaw(pos.x, pos.y, pos.z);
		}

		inline vec<uchar>& AccessPalleteColor(uchar index) {
			return palette[index];
		}

		void Flip(bool doX, bool doY, bool doZ) {
			uchar	current = 0;
			uchar	mirror	= 0;
			if(doX) {
#ifdef __unix__
				#pragma omp parallel for
#endif
				for(int z = 0; z < size.z; ++z) {
					for(int y = 0; y < size.y; ++y) {
						int	halfX	= floor(size.x * 0.5f);
						for(int x = 0; x < halfX; ++x) {
							current	= GetVoxelRaw(x, y, z);
							mirror	= GetVoxelRaw(size.x - x - 1, y, z);
							SetVoxelRaw(x, y, z, mirror);
							SetVoxelRaw(size.x - x - 1, y, z, current);
						}	
					}
				}
			}
			if(doY) {
#ifdef __unix__
				#pragma omp parallel for
#endif
				for(int z = 0; z < size.z; ++z) {
					for(int x = 0; x < size.x; ++x) {
						int	halfY	= floor(size.y * 0.5f);
						for(int y = 0; y < halfY; ++y) {
							current	= GetVoxelRaw(x, y, z);
							mirror	= GetVoxelRaw(x, size.y - y - 1, z);
							SetVoxelRaw(x, y, z, mirror);
							SetVoxelRaw(x, size.y - y - 1, z, current);
						}	
					}
				}
			}
			if(doZ) {
#ifdef __unix__
				#pragma omp parallel for
#endif
				for(int x = 0; x < size.x; ++x) {
					for(int y = 0; y < size.y; ++y) {
						int	halfZ	= floor(size.z * 0.5f);
						for(int z = 0; z < halfZ; ++z) {
							current	= GetVoxelRaw(x, y, z);
							mirror	= GetVoxelRaw(x, y, size.z - z - 1);
							SetVoxelRaw(x, y, z, mirror);
							SetVoxelRaw(x, y, size.z - z - 1, current);
						}	
					}
				}
			}
		}

	private:
		void Alloc(int x, int y, int z) {
			size.Set(x, y, z);
			unsigned int wholeSize	= x * y * z;
			if(voxel not_eq nullptr)
				delete[]	voxel;
			voxel					= new uchar[wholeSize];
			memset(voxel, 0, wholeSize);
		}

		class Chunk {
			public:
				enum Type : int {
					MAIN	= 0x4E49414D,
					SIZE	= 0x455A4953,
					XYZI	= 0x495A5958,
					RGBA	= 0x41424752,

					PACK	= 0x4B434150,
					MATT	= 0x5454414D,
					MATL	= 0x4C54414D,
					DICT	= 0x54434944,
					nTRN	= 0x4E52546E,
					nGRP	= 0x5052476E,
					nSHP	= 0x5048536E,
					LAYR	= 0x5259414C,

					rOBJ	= 0x4A424F72
				};

				Type		id;
				int			contentSize;
				int			childrenSize;
				long int	end;

				void ReadFromFile(ifstream& hFile) {
					hFile.read(reinterpret_cast<char*>(this), 12);

					end	= static_cast<int>(hFile.tellg()) + contentSize + childrenSize;
				}
		};

		bool ReadFile(ifstream& hFile) {
			//Magic number
			int magic;
			hFile.read(reinterpret_cast<char*>(&magic), 4);
			if(magic not_eq this->ID_VOX) {
				cerr	<< "[VOX] Magic number does not match proper one!" << endl;
				return false;
			}
			
			//Version
			hFile.read(reinterpret_cast<char*>(&version), 4);
			if(version not_eq MV_VERSION) {
				cerr	<< "[VOX] Supported version does not match!" << endl;
				return false;
			}
			
			//Main chunk
			Chunk	mainChunk;
			mainChunk.ReadFromFile(hFile);
			if(mainChunk.id not_eq Chunk::Type::MAIN) {
				cerr	<< "[VOX] Main chunk does not exists! Broken file." << endl;
				return false;
			}
			
			//Skip content of main chunk
			if(mainChunk.contentSize > 0)
				hFile.seekg(static_cast<int>(hFile.tellg()) + mainChunk.contentSize);
			
			bool	customPalette	= false;
			int		numVoxels		= 0;

			//Read children chunks
			while(hFile.tellg() < mainChunk.end) {
				if(hFile.tellg() <= -1) {
					cerr << "[VOX] Unhandled error!" << endl;
					return false;
				}

				Chunk childrenChunk;
				childrenChunk.ReadFromFile(hFile);
				
				switch(childrenChunk.id) {
					case(Chunk::Type::SIZE): {
						hFile.read(reinterpret_cast<char*>(&size), 12);

						Alloc(size.x, size.y, size.z);
						break;
					}
					case(Chunk::Type::XYZI): {
						hFile.read(reinterpret_cast<char*>(&numVoxels), 4);
						if(numVoxels > 0) {
							for(int i = 0; i < numVoxels; ++i) {
								vec<uchar>	readVoxel;
								hFile.read(reinterpret_cast<char*>(&readVoxel), 4);

								int idx		= readVoxel.x + size.x * (
									readVoxel.y + size.y * readVoxel.z
								);
								voxel[idx] 	= readVoxel.w;
							}
						} else {
							cerr	<< "[VOX] Improper voxel number, file broken!" << endl;
							return false;
						}
						break;
					}
					case(Chunk::Type::RGBA): {
						//Clean old palette
						memset(reinterpret_cast<void*>(palette), 0, sizeof(vec<uchar>) * 255);

						//Last color is not used, so we only need to read 255 colors
						hFile.read(reinterpret_cast<char*>(palette), sizeof(vec<uchar>) * 255);
						hFile.seekg(static_cast<int>(hFile.tellg()) + sizeof(vec<uchar>));

						customPalette	= true;
					}
					case(Chunk::Type::PACK): {
						//cout	<< "[VOX] Multiple models are not supported, ignoring rest of model!" << endl;
						break;
					}
					case(Chunk::Type::MATT):
					case(Chunk::Type::MATL):
					case(Chunk::Type::DICT):
					case(Chunk::Type::nTRN):
					case(Chunk::Type::nGRP):
					case(Chunk::Type::nSHP):
					case(Chunk::Type::rOBJ):
					case(Chunk::Type::LAYR): {
						//Unsupported header, silent ignore
						break;
					}
					default: {
						cerr	<< "[VOX] Unknown header (at 0x" << std::hex
								<< hFile.tellg() << std::dec << "), ignoring!"
								<< endl;
						break;
					}
				}
				if(childrenChunk.end >= mainChunk.end)
					break;
				hFile.seekg(childrenChunk.end);
			}
			if(not customPalette)
				SetDefaultPalette();
			
			return true;
		}

		bool WriteFile(ofstream& hFile) {
			//Temporary
			int	chunkSize	= 0;
			int numVoxels	= 0;

			//Calculation of existing voxels
			int wholeSize	= size.x * size.y * size.z;
			for(int i = 0; i < wholeSize; ++i)
				if(voxel[i] > 0)
					++numVoxels;

			//Const
			const char*	zero	= "\0\0\0";

			//Headers
			hFile.write("VOX ", 4);
			hFile.write(reinterpret_cast<char*>(&version), 4);
			hFile.write("MAIN", 4);
			hFile.write(zero, 4);
			chunkSize	= numVoxels * sizeof(vec<uchar>) + 0x434;	
			hFile.write(reinterpret_cast<char*>(&chunkSize), 4);

			hFile.write("SIZE", 4);
			chunkSize	= 12;
			hFile.write(reinterpret_cast<char*>(&chunkSize), 4);
			hFile.write(zero, 4);
			hFile.write(reinterpret_cast<char*>(&size.x), 4);
			hFile.write(reinterpret_cast<char*>(&size.y), 4);
			hFile.write(reinterpret_cast<char*>(&size.z), 4);

			//Voxels
			hFile.write("XYZI", 4);
			chunkSize	= 4 + numVoxels * sizeof(vec<uchar>);
			hFile.write(reinterpret_cast<char*>(&chunkSize), 4);	
			hFile.write(zero, 4);
			hFile.write(reinterpret_cast<char*>(&numVoxels), 4);
			
			uchar 	val	= 0;
			for(int z = 0; z < size.z; ++z) {
				for(int y = 0; y < size.y; ++y) {
					for(int x = 0; x < size.x; ++x) {
						val	= GetVoxel(x, y, z);
						if(val > 0) {
							int	vox	= (
								(val << 24) & 0xFF000000
							) | (
								(z << 16) & 0x00FF0000
							) | (
								(y << 8) & 0x0000FF00
							) | x;
							hFile.write(reinterpret_cast<char*>(&vox), 4);
						}
					}
				}
			}

			//Palette
			hFile.write("RGBA", 4);
			chunkSize	= 0x400;
			hFile.write(reinterpret_cast<char*>(&chunkSize), 4);
			hFile.write(zero, 4);

			for(int i = 0; i < 256; ++i) {
				hFile.write(reinterpret_cast<char*>(&palette[i]), 4);
			}

			hFile.flush();
			return true;
		}

		void SetDefaultPalette() {
			unsigned int defaultPalette[256] = {
				//0 => Unused color
				0x00000000, 0xffffffff, 0xffccffff, 0xff99ffff, 0xff66ffff, 0xff33ffff, 0xff00ffff, 0xffffccff,
				0xffccccff, 0xff99ccff, 0xff66ccff, 0xff33ccff, 0xff00ccff, 0xffff99ff, 0xffcc99ff, 0xff9999ff,
				0xff6699ff, 0xff3399ff, 0xff0099ff, 0xffff66ff, 0xffcc66ff, 0xff9966ff, 0xff6666ff, 0xff3366ff,
				0xff0066ff, 0xffff33ff, 0xffcc33ff, 0xff9933ff, 0xff6633ff, 0xff3333ff, 0xff0033ff, 0xffff00ff,
				0xffcc00ff, 0xff9900ff, 0xff6600ff, 0xff3300ff, 0xff0000ff, 0xffffffcc, 0xffccffcc, 0xff99ffcc,
				0xff66ffcc, 0xff33ffcc, 0xff00ffcc, 0xffffcccc, 0xffcccccc, 0xff99cccc, 0xff66cccc, 0xff33cccc,
				0xff00cccc, 0xffff99cc, 0xffcc99cc, 0xff9999cc, 0xff6699cc, 0xff3399cc, 0xff0099cc, 0xffff66cc,
				0xffcc66cc, 0xff9966cc, 0xff6666cc, 0xff3366cc, 0xff0066cc, 0xffff33cc, 0xffcc33cc, 0xff9933cc,
				0xff6633cc, 0xff3333cc, 0xff0033cc, 0xffff00cc, 0xffcc00cc, 0xff9900cc, 0xff6600cc, 0xff3300cc,
				0xff0000cc, 0xffffff99, 0xffccff99, 0xff99ff99, 0xff66ff99, 0xff33ff99, 0xff00ff99, 0xffffcc99,
				0xffcccc99, 0xff99cc99, 0xff66cc99, 0xff33cc99, 0xff00cc99, 0xffff9999, 0xffcc9999, 0xff999999,
				0xff669999, 0xff339999, 0xff009999, 0xffff6699, 0xffcc6699, 0xff996699, 0xff666699, 0xff336699,
				0xff006699, 0xffff3399, 0xffcc3399, 0xff993399, 0xff663399, 0xff333399, 0xff003399, 0xffff0099,
				0xffcc0099, 0xff990099, 0xff660099, 0xff330099, 0xff000099, 0xffffff66, 0xffccff66, 0xff99ff66,
				0xff66ff66, 0xff33ff66, 0xff00ff66, 0xffffcc66, 0xffcccc66, 0xff99cc66, 0xff66cc66, 0xff33cc66,
				0xff00cc66, 0xffff9966, 0xffcc9966, 0xff999966, 0xff669966, 0xff339966, 0xff009966, 0xffff6666,
				0xffcc6666, 0xff996666, 0xff666666, 0xff336666, 0xff006666, 0xffff3366, 0xffcc3366, 0xff993366,
				0xff663366, 0xff333366, 0xff003366, 0xffff0066, 0xffcc0066, 0xff990066, 0xff660066, 0xff330066,
				0xff000066, 0xffffff33, 0xffccff33, 0xff99ff33, 0xff66ff33, 0xff33ff33, 0xff00ff33, 0xffffcc33,
				0xffcccc33, 0xff99cc33, 0xff66cc33, 0xff33cc33, 0xff00cc33, 0xffff9933, 0xffcc9933, 0xff999933,
				0xff669933, 0xff339933, 0xff009933, 0xffff6633, 0xffcc6633, 0xff996633, 0xff666633, 0xff336633,
				0xff006633, 0xffff3333, 0xffcc3333, 0xff993333, 0xff663333, 0xff333333, 0xff003333, 0xffff0033,
				0xffcc0033, 0xff990033, 0xff660033, 0xff330033, 0xff000033, 0xffffff00, 0xffccff00, 0xff99ff00,
				0xff66ff00, 0xff33ff00, 0xff00ff00, 0xffffcc00, 0xffcccc00, 0xff99cc00, 0xff66cc00, 0xff33cc00,
				0xff00cc00, 0xffff9900, 0xffcc9900, 0xff999900, 0xff669900, 0xff339900, 0xff009900, 0xffff6600,
				0xffcc6600, 0xff996600, 0xff666600, 0xff336600, 0xff006600, 0xffff3300, 0xffcc3300, 0xff993300,
				0xff663300, 0xff333300, 0xff003300, 0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000,
				0xff0000ee, 0xff0000dd, 0xff0000bb, 0xff0000aa, 0xff000088, 0xff000077, 0xff000055, 0xff000044,
				0xff000022, 0xff000011, 0xff00ee00, 0xff00dd00, 0xff00bb00, 0xff00aa00, 0xff008800, 0xff007700,
				0xff005500, 0xff004400, 0xff002200, 0xff001100, 0xffee0000, 0xffdd0000, 0xffbb0000, 0xffaa0000,
				0xff880000, 0xff770000, 0xff550000, 0xff440000, 0xff220000, 0xff110000, 0xffeeeeee, 0xffdddddd,
				0xffbbbbbb, 0xffaaaaaa, 0xff888888, 0xff777777, 0xff555555, 0xff444444, 0xff222222, 0xff111111,
			};
#ifdef __unix__
			#pragma omp parallel for
#endif
			for(int i = 0; i < 256; ++i) {
				palette[i].Set(
					uchar((defaultPalette[i] & 0xFF00) >> 8),
					uchar((defaultPalette[i] & 0xFF0000) >> 16),
					uchar((defaultPalette[i] & 0xFF000000) >> 24),
					uchar(defaultPalette[i] & 0xFF)
				);
			}
		}

		static constexpr const int MV_VERSION	= 150;
		static constexpr const int ID_VOX		= 0x20584F56;	//VOX 
};
#endif