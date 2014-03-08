#pragma warning( disable : 4996 )

#include <stdio.h>
#include <map>
#include <string>

using namespace std;

struct AMDCardData
{
	char Name[ 128 ];

	int ALUInstructions;
	int TEXInstructions;
	int VTXInstructions;
	int EMITInstructions;
	int InterpInstructions;
	int ControlFlowInstructions;
	int ExportInstructions;

	int TextureDepLevel;

	int TempRegs;

	float MinCycles;
	float MaxCycles;
	float AvgCycles;

	float PeakBilinar;
	float PeakTrilinear;
	float PeakAniso;
};

struct NVidiaCardData
{
	char Name[ 128 ];

	int Instructions;
	int Conditionals;

	float FillRate;
	int Cycles;
	int Regs;
};

long long ToFillRate( const char* s )
{
	const char* p = s + strlen( s );

	long long fillRate = 0;
	long long mul = 1;

	while( p >= s )
	{
		if( *p >= '0' && *p <= '9' )
		{
			fillRate += ( *p - '0' ) * mul;
			mul *= 10;
		}

		p--;
	}

	return fillRate;
}

const char* AMD				= "-amd" ;
const char* NVIDIA			= "-nvidia" ;
const char* SHORT_NVIDIA	= "-short_nvidia" ;
const char* ALL				= "-all" ;
const char* SM20			= "-sm20" ;
const char* SM40			= "-sm40" ;
const char* SM50			= "-sm50" ;

enum Args
{
	A_AMD,
	A_NVIDIA,
	A_SHORT_NVIDIA,
	A_ALL,
	A_SM20,
	A_SM40,
	A_SM50,
	A_UNKNOWN
};

Args ToArg( const char* arg )
{
	if( !strcmp( arg, AMD ) )			return A_AMD;
	if( !strcmp( arg, NVIDIA ) )		return A_NVIDIA;
	if( !strcmp( arg, SHORT_NVIDIA ) )	return A_SHORT_NVIDIA;
	if( !strcmp( arg, ALL ) )			return A_ALL;
	if( !strcmp( arg, SM20 ) )			return A_SM20;
	if( !strcmp( arg, SM40 ) )			return A_SM40;
	if( !strcmp( arg, SM50 ) )			return A_SM50;

	return A_UNKNOWN;
}

int main( int argc, char** argv )
{
	struct OnExit
	{
		~OnExit()
		{
			fcloseall();
		}
	} onExit; (void)onExit;

	bool bad_args = false;

	if( argc < 3  || argc > 6 )
	{
		bad_args = true;
	}

	bool do_nvidia		= false;
	bool do_amd			= false;
	bool short_nvidia	= false;
	bool do_sm20		= false;
	bool do_sm40		= false;
	bool do_sm50		= false;

	if( argc == 3 )
	{
		do_nvidia = true;
		do_amd = true;
	}
	else
	{
		for( int i = 3, e = argc; i < e && !bad_args; i ++ )
		{
			Args arg = ToArg( argv[ i ] );

			if( arg == A_UNKNOWN )
			{
				bad_args = true;
				break;
			}

			if( arg == A_NVIDIA )		{ do_nvidia = true;	}
			if( arg == A_SHORT_NVIDIA )	{ do_nvidia = true; short_nvidia = true; }
			if( arg == A_AMD )			{ do_amd = true; }
			if( arg == A_ALL )			{ do_amd = true; do_nvidia = true; }
			if( arg == A_SM20 )			{ do_sm20 = true ; }
			if( arg == A_SM40 )			{ do_sm40 = true ; }
			if( arg == A_SM50 )			{ do_sm50 = true ; }
		}
	}

	if( bad_args )
	{
		printf( "Usage: GPUSAParser <shader_input> <output> [-amd|-nvidia|-short_nvidia|-all] [-sm20|-sm50]\n" );
		return 1;
	}

	string gpuAnalizerPath = "C:\\Program Files (x86)\\AMD\\GPU ShaderAnalyzer 1.55\\";
	string nvPerfPath = "C:\\Program Files (x86)\\NVIDIA Corporation\\NVIDIA ShaderPerf\\";

	char CfgFilePath[ 2048 ];
	{
		char drive[16], path[2048], name[128], ext[128];

		_splitpath( argv[ 0 ], drive, path, name, ext );

		sprintf( CfgFilePath, "%s%sGPUSAParser.txt", drive, path );
	}

	FILE* cfg = fopen( CfgFilePath, "rb" );

	if( cfg )
	{
		char path[ 2048 ];
		path[ sizeof path - 1 ] = 0;

		fgets( path, sizeof path - 1, cfg );

		if( *( path + strlen( path ) - 2 ) == 0xD )
			*( path + strlen( path ) - 2 ) = 0;

		gpuAnalizerPath = path;

		fgets( path, sizeof path - 1, cfg );

		if( *( path + strlen( path ) - 2 ) == 0xD )
			*( path + strlen( path ) - 2 ) = 0;

		nvPerfPath = path;

		fclose( cfg );
	}

	bool vertexShader = false;

	if( strstr( argv[1], "_vs") || 
		strstr( argv[1], "_Vs") ||
		strstr( argv[1], "_VS") || 
		strstr( argv[1], "_vS") )
	{
		vertexShader = true;
	}

	FILE* fout = fopen( argv[ 2 ], "wb" );

	if( !fout )
		return 12;

	// do nVIDIA Commands
	if( do_nvidia )
	{
		char *GPUNames[][2] = 
		{
			{ "GeForce 6800 Ultra", "NV40"		},
			{ "GeForce 6800 GT",	"NV40-GT"	},
			{ "GeForce 6800",		"NV40-12"	},
			{ "GeForce 6600 GT",	"NV43-GT"	},
			{ "GeForce 6200",		"NV44"		},
			{ "GeForce 7800 GTX",	"G70-GT"	},
			{ "GeForce 7800 GT",	"G70"		},
			{ "GeForce 8800 GTX",	"G80"		},
			{ "GeForce 8800 GTS",	"G80-GTS"	},
			{ "GeForce 8600 GTS",	"G84"		},
			{ "GeForce 8600 GT",	"G84-GT"	},
			{ "GeForce 8500 GT",	"G86"		},
			{ "GeForce 8400 GS",	"G86-GS"	},
			{ "GeForce 8800 Ultra",	"G80-Ultra"	}
		};

		bool GPUEnable[ sizeof GPUNames / sizeof GPUNames[ 0 ] ] = 
		{
			false,
			false,
			true,
			false,
			false,
			false,
			true,
			true,
			false,
			false,
			false,
			false
		};

		if( !short_nvidia )
		{
			for( size_t i = 0, e = sizeof GPUEnable / sizeof GPUEnable[ 0 ]; i < e; i ++ )
			{
				GPUEnable[ i ] = true ;
			}
		}

		typedef multimap< float, NVidiaCardData > Datas;

		Datas datas;

		string command_args = "–type hlsl_ps –profile ";

		if( vertexShader )
		{
			command_args = "–type hlsl_vs –profile ";

			if( do_sm20 )
			{
				command_args += "vs_2_0" ;
			}
			else
			{
				command_args += "vs_3_0" ;
			}
		}
		else
		{
			if( do_sm20 )
			{
				command_args += "ps_2_0" ;
			}
			else
			{
				command_args += "ps_3_0" ;
			}			
		}

		string result_file_path = "NVPerf_diag.txt";

		char drive[16], dir[512], name[512], ext[ 512 ];

		_splitpath( argv[ 1 ], drive, dir, name, ext );

		string include = string( " -include " ) + drive + dir ;

		bool hasBranching = false;

		for( size_t i = 0, e = sizeof GPUNames / sizeof GPUNames[ 0 ]; i < e; i ++ )
		{

			if( !GPUEnable[ i ] )
				continue;

			string cmd = "\"" + nvPerfPath + "NVShaderPerf.exe\" " + command_args + include + " –function main -g " + GPUNames[ i ][ 1 ] + " \"" + argv[ 1 ] + "\"";
			cmd = "cmd /c \"" + cmd + " > " + "\"" + result_file_path + "\"\"";

			remove( result_file_path.c_str() );

			system( cmd.c_str() );

			FILE* results = fopen( result_file_path.c_str(), "rb" );

			bool have_output = false;

			if( results )
			{
				fseek( results, 0, SEEK_END );
				if( ftell( results ) )
				{
					have_output = true;
					fseek( results, 0, SEEK_SET );
				}
			}

			if( !have_output )
			{
				fprintf( fout, "nVShaderPerf command produced no results! Check your path in GPUSAParser.txt!\n" );
				return 13;
			}

			char str[ 2048 ];

			str[ sizeof str - 1 ] = 0;

			NVidiaCardData data;

			memset( &data, 0, sizeof data );

			strcpy( data.Name, GPUNames[ i ][ 0 ] );

			while( !feof( results ) )
			{
				fgets( str, sizeof str - 1, results );

				if( vertexShader )
				{
					if( strstr( str, "Vertex program has" ) )
					{
						sscanf( str, "Vertex program has %d ucode instructions and %d conditionals", &data.Instructions, &data.Conditionals );
					}

					if( strstr( str, "Results " ) )
					{
						char verts[ 64 ];
						sscanf( str, "Results %d cycles, %d r regs, %s", &data.Cycles, &data.Regs, verts );

						data.FillRate = float( ToFillRate( verts ) / 1000000. );
					}
				}
				else
				{
					if( strstr( str, "Results " ) )
					{
						char verts[ 64 ];

						sscanf( str, "Results %d cycles, %d r regs, %s", &data.Cycles, &data.Regs, verts );

						data.FillRate = float( ToFillRate( verts ) / 1000000. );
						data.Instructions = 0;
						data.Conditionals = 0;
					}
				}
			}

			datas.insert( Datas::value_type( data.FillRate, data ) );

			if( data.Conditionals )
				hasBranching = true;

			fclose( results );

			printf( "%s\n", GPUNames[ i ][ 0 ] );
		}

		if( hasBranching )
		{
			fprintf( fout, "WARNING: Shader has dynamic branching!\n" );
		}

		fprintf( fout, "%20s|%8s|%4s|%6s|%8s|\n", "Card", "Instr.", "Regs", "Cycles", "Fill" );
		fprintf( fout, "%20s|%8s|%4s|%6s|%8s|\n", "", "", "", "", "" );

		float FillRateSum = 0.f;
		float FillRateCount = 0.f;

		for( Datas::const_iterator i = datas.begin(), e = datas.end(); i != e; ++ i )
		{
			const NVidiaCardData& d = i->second;

			fprintf( fout, "%-20s|%8d|%4d|%6d|%8.2f|\n", 
							d.Name, 
							d.Instructions + d.Conditionals,
							d.Regs,
							d.Cycles,
							d.FillRate );

			FillRateSum += d.FillRate;
			FillRateCount += 1.f;
		}

		if( FillRateCount < 1.f )
			FillRateCount = 1.f;

		fprintf( fout, "\n" );

		fprintf( fout, "AVERAGE FILL RATE: %.2f %s\n\n", FillRateSum / FillRateCount, vertexShader ? "MVertices/sec" : "MPixels/sec" );

	}

	{
		if( do_amd )
		{
			string result_file_path = "GPUSAnOutput_diag.txt";

			remove( result_file_path.c_str() );

			// do AMD command
			{
				string profile_args = "ps_3_0";
				if( vertexShader )
				{
					profile_args = "vs_3_0";
				}

				if( do_sm20 )
				{
					if( vertexShader )
					{
						profile_args = "vs_2_0";
					}
					else
					{
						profile_args = "ps_2_0";
					}
				}

				if( do_sm40 )
				{
					if( vertexShader )
					{
						profile_args = "vs_4_0";
					}
					else
					{
						profile_args = "ps_4_0";
					}
				}

				if( do_sm50 )
				{
					if( vertexShader )
					{
						profile_args = "vs_5_0";
					}
					else
					{
						profile_args = "ps_5_0";
					}
				}

				string cmd = "\"" + gpuAnalizerPath + "GPUShaderAnalyzer.exe\" \"" + argv[ 1 ] + "\" -A " + result_file_path + " -P " + profile_args + " -ASIC ALL -Z";

				cmd = "cmd /c \"" + cmd + "\"";

				system( cmd.c_str() );
			}

			FILE* analizeIn = fopen( result_file_path.c_str(), "rb" );

			if( !analizeIn )
			{
				fprintf( fout, "AMD GPU Shader Analizer command produced no results! Check your path in GPUSAParser.txt!\n" );
				return 13;
			}
		
			typedef multimap< float, AMDCardData > Datas;
			Datas datas;

			while( !feof( analizeIn ) )
			{
				AMDCardData data;

				char str[ 1024 ];
				str[ sizeof str - 1 ] = 0;

				fgets( str, sizeof str - 1, analizeIn );

				char HardwareStatsStr[] = "Hardware stats for";

				if( char* stats_start = strstr( str, HardwareStatsStr ) )
				{
					strncpy( data.Name, stats_start + sizeof HardwareStatsStr, sizeof data.Name - 1 );

					if( data.Name[ strlen( data.Name ) - 2 ] == 0x0D )
						data.Name[ strlen( data.Name ) - 2 ] = 0;

					data.Name[ sizeof data.Name - 1 ] = 0;

					fgets( str, sizeof str - 1, analizeIn );

					if( !strstr( str, "Compile Failed" ) )
					{				
						sscanf( str, "Number of instructions: %d ALU, %d TEX, %d VTX, %d EMIT", &data.ALUInstructions, &data.TEXInstructions, &data.VTXInstructions, &data.EMITInstructions );

						fgets( str, sizeof str - 1, analizeIn );
						sscanf( str, "Number of instructions: %d Interp, %d ControlFlow, %d Export", &data.InterpInstructions, &data.ControlFlowInstructions, &data.ExportInstructions );

						fgets( str, sizeof str - 1, analizeIn );
						sscanf( str, "Texture Dependancy level: %d", &data.TextureDepLevel );

						fgets( str, sizeof str - 1, analizeIn );
						sscanf( str, "Temp Registers Used: %d", &data.TempRegs );			

						fgets( str, sizeof str - 1, analizeIn );
						sscanf( str, "Expected cycles = min %f, max %f, avg %f", &data.MinCycles, &data.MaxCycles, &data.AvgCycles );

						fgets( str, sizeof str - 1, analizeIn );
						fgets( str, sizeof str - 1, analizeIn );
						fgets( str, sizeof str - 1, analizeIn );

						fgets( str, sizeof str - 1, analizeIn );
						sscanf( str, "Avg Peak Throughput: Bilinear:%f M \\ Trilinear:%f M \\ Aniso:%f M", &data.PeakBilinar, &data.PeakTrilinear, &data.PeakAniso );

						datas.insert( Datas::value_type( data.PeakBilinar, data ) );
					}
				}

		/*
				================================================================
				Hardware stats for Radeon HD 2900
				Number of instructions: 103 ALU, 0 TEX, 1 VTX, 0 EMIT
				Number of instructions: 0 Interp, 13 ControlFlow, 7 Export
				Texture Dependancy level: 0
				Temp Registers Used: 17
				Expected cycles = min 25.75, max 25.75, avg 25.75
				Estimated Cycles: Bilinear:25.75 \ Trilinear:25.75 \ Aniso:25.75
				Bottleneck: Bilinear:ALU Ops \ Trilinear: ALU Ops \ Aniso: ALU Ops
				ALU:Tex Ratio: Bilinear:25.75 \ Trilinear:25.75 \ Aniso:25.75
				Avg Peak Throughput: Bilinear:461.05 M \ Trilinear:461.05 M \ Aniso:461.05 M
				Avg Item Per Clock: Bilinear:0.62 \ Trilinear:0.62 \ Aniso:0.62
				Max scratch registers allocated: 0*/

			}

			fprintf( fout, "%15s|%8s|%4s|%7s|%8s|%9s|%9s|\n", "Card", "Instr.", "Regs", "Cycles", "Fill(Bi)", "Fill(Tri)", "Fill(Ani)" );
			fprintf( fout, "%15s|%8s|%4s|%7s|%8s|%9s|%9s|\n", "", "", "", "", "", "", "" );

			float FillRateSum = 0.f;
			float FillRateCount = 0.f;

			for( Datas::const_iterator i = datas.begin(), e = datas.end(); i != e; ++i )
			{
				const AMDCardData& d = i->second;
				fprintf( fout, "%-15s|%8d|%4d|%7.2f|%8.2f|%9.2f|%9.2f|\n", 
							d.Name, 
							d.ALUInstructions + d.ControlFlowInstructions + d.InterpInstructions + d.VTXInstructions,
							d.TempRegs,
							d.AvgCycles,
							d.PeakBilinar,
							d.PeakTrilinear,
							d.PeakAniso	);

				FillRateSum += d.PeakBilinar ;
				FillRateSum += d.PeakBilinar ;
				FillRateSum += d.PeakAniso ;
				FillRateCount += 3.f ;

			}

			if( FillRateCount < 1)
				FillRateCount = 1;

			fprintf( fout, "\n" );

			fprintf( fout, "AVERAGE FILL RATE: %.2f %s\n\n", FillRateSum / FillRateCount, vertexShader ? "MVertices/sec" : "MPixels/sec" );
		}

		// disassemble using AMD tool
		{
			string profile_args = "ps_3_0";
			if( vertexShader )
			{
				profile_args = "vs_3_0";
			}

			if( do_sm20 )
			{
				if( vertexShader )
				{
					profile_args = "vs_2_0";
				}
				else
				{
					profile_args = "ps_2_0";
				}
			}

			if( do_sm40 )
			{
				if( vertexShader )
				{
					profile_args = "vs_4_0";
				}
				else
				{
					profile_args = "ps_4_0";
				}
			}

			if( do_sm50 )
			{
				if( vertexShader )
				{
					profile_args = "vs_5_0";
				}
				else
				{
					profile_args = "ps_5_0";
				}
			}

			string result_file_path = "GPUSAnOutput_dasm.txt";

			string cmd = "\"" + gpuAnalizerPath + "GPUShaderAnalyzer.exe\" \"" + argv[ 1 ] + "\" -I " + result_file_path + " -P " + profile_args + " -ASIC D3D";

			cmd = "cmd /c \"" + cmd + "\"";
			system( cmd.c_str() );

			FILE* dasm = fopen( result_file_path.c_str(), "rb" );

			if( dasm )
			{
				fseek( dasm, 0, SEEK_END );

				size_t sz = ftell( dasm );

				char* buff = new char[ sz ];

				fseek( dasm, 0, SEEK_SET );
				
				fread( buff, sz, 1, dasm );

				fwrite( buff, sz, 1, fout );
			}
		}
	}

}