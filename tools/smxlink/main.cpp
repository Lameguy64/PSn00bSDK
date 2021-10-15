/* This tool was originally developed for Scarlet Engine 
 * (formerly Project Scarlet)
 *
 * This utility is still a work in progress and the SMX and SMD specification
 * is subject to change without notice. It is included as part of the
 * PSn00bSDK project as an example utility for the n00bdemo example which
 * uses SMD format model data files.
 *
 */

#include <stdio.h>
#include <math.h>
#include <tinyxml2.h>
#include <string>
//#include <windef.h>
#include "timreader.h"

#ifdef WIN32
#define strcasecmp _stricmp
#endif

#define VERSION "0.25b"

namespace param
{
	std::string smxFileName;
	std::string smdFileName;
	std::string texDir;

	float	scaleFactor = 1.f;
}

typedef struct {
	char			id[3];		// File ID (SMD)
	unsigned char	version;	// Version number (0x01)
	unsigned short	flags;
	unsigned short	numverts;
	unsigned short	numnorms;
	unsigned short	numprims;
	unsigned long	vtxAddr;
	unsigned long	nrmAddr;
	unsigned long	priAddr;
} SMD_HEADER;

typedef struct {
	short vx,vy,vz,vp;
} SVECTOR;


#define PRIM_TYPE_LINE	0
#define PRIM_TYPE_TRI	1
#define PRIM_TYPE_QUAD	2

#define PRIM_LIGHTING_NONE		0	// No shading (no normals)
#define PRIM_LIGHTING_FLAT		1	// Flat shading (1 normal)
#define PRIM_LIGHTING_SMOOTH	2	// Smooth shading (3 normals per vertex)

typedef struct {
	
    unsigned char type:2;		// Primitive type
	unsigned char l_type:2;		// Lighting type (0 - none, 1 - flat shading, 2 - smooth shading)
	unsigned char c_type:1;		// Coloring type (0 - solid color, 1 - gouraud)
    unsigned char texture:1;	// Texture mapped
	unsigned char blend:2;		// Blend mode setting (actual blend enable is determined by primitive code)
	// byte boundary
	unsigned char zoff:4;
	unsigned char nocull:1;		// Double sided (no cull)
	unsigned char mask:1;		// Force mask bit setting    
	unsigned char texwin:2;
	// byte boundary
	unsigned char texoff:2;
	unsigned char reserved:6;
	// byte boundary
	unsigned char len;
} PRIM_ID;

typedef struct {
	unsigned short	v0,v1,v2,v3;
} PRIM_V;

typedef struct {
	unsigned char	r,g,b,c;
} PRIM_RGBC;

typedef struct {
	unsigned short tpage,clut;
} PRIM_TC;

typedef struct {
	unsigned char	u,v;
} PRIM_UV;


int main(int argc, const char* argv[]) {

	printf("SMXLINK " VERSION " - Scarlet SMX to SMD Model Converter "
		"(part of Scarlet Engine)\n");
	printf("Note: Outputs in *NEW* revision 1 format!\n");
	printf("2017-2019 Meido-Tek Productions\n\n");

	if (argc <= 1) {

		printf("Parameters:\n");
		printf("   smxlink [-o <filename>] [-s <scale>] <smxfile>\n\n");
		printf("   -o  <filename> - Specify output filename (default: first file specified)\n");
		printf("   -s  <scale>    - Scale factor to apply to model on conversion (default: 1.0)\n");
		printf("   -tp <path>     - Specify directory path to TIM texture files\n");
		printf("   <smxfile>	  - SMX file to convert to SMD\n");

		return EXIT_SUCCESS;

	}

	for(int i=1; i<argc; i++) {

		if (strcasecmp(argv[i], "-o") == 0) {

			i++;
			param::smdFileName = argv[i];

		} else if (strcasecmp(argv[i], "-s") == 0) {

			i++;
			param::scaleFactor = atof(argv[i]);

		} else if (strcasecmp(argv[i], "-tp") == 0) {

			i++;
			param::texDir = argv[i];

			if( ( param::texDir[param::texDir.size()-1] != '\\' ) &&
				( param::texDir[param::texDir.size()-1] != '/') ) {
				
				param::texDir += "/";
				
			}
			
		} else {

			param::smxFileName = argv[i];

		}

	}

	if (param::smdFileName.empty()) {

        param::smdFileName = param::smxFileName;
        param::smdFileName.erase(param::smdFileName.rfind("."));
        param::smdFileName += ".smd";

	}

	printf("Input  : %s\n", param::smxFileName.c_str());
	printf("Output : %s\n", param::smdFileName.c_str());

	if (!param::texDir.empty())
		printf("TexDir : %s\n", param::texDir.c_str());

	printf("\n");

    tinyxml2::XMLDocument smxFile;

    if (smxFile.LoadFile(param::smxFileName.c_str()) != tinyxml2::XML_SUCCESS) {
		printf("ERROR: Unable to load SMX file:\n");
		smxFile.PrintError();
		return EXIT_FAILURE;
    }

    tinyxml2::XMLElement* smxModel = smxFile.FirstChildElement("model");

	// Parse textures
	TIM_COORDS *texCoords = NULL;
	int numTextures = 0;

	if (smxModel->FirstChildElement("textures") != NULL) {

        tinyxml2::XMLElement *texFileElement = smxModel->FirstChildElement("textures");

        if (texFileElement != NULL) {

			numTextures = atoi(texFileElement->Attribute("count"));
			texFileElement = texFileElement->FirstChildElement("texture");

			texCoords = (TIM_COORDS*)malloc(sizeof(TIM_COORDS)*numTextures);

			int index = 0;

			while(texFileElement != NULL) {

				std::string timFileName = texFileElement->Attribute("file");

				if (timFileName.rfind(".") == std::string::npos)
				{
					timFileName.append(".tim");
				}

				timFileName = param::texDir + timFileName;

                if (!GetTimCoords(timFileName.c_str(), &texCoords[index])) {
                    printf("ERROR: Unable to open texture file: %s\n", timFileName.c_str());
                    free(texCoords);
                    return EXIT_FAILURE;
                }

				switch(texCoords[index].flag.pmode) {
				case 0:	// 4-bit
					texCoords[index].pixdata.pw *= 4;
					break;
				case 1: // 8-bit
					texCoords[index].pixdata.pw *= 2;
					break;
				}
				
				texFileElement = texFileElement->NextSiblingElement("texture");
				index++;

			}

        }

	}


    FILE* smdFile = fopen(param::smdFileName.c_str(), "wb");

	// Create temporary header
	SMD_HEADER smdHeader;

	memset(&smdHeader, 0x00, sizeof(SMD_HEADER));
	fwrite(&smdHeader, sizeof(SMD_HEADER), 1, smdFile);


    // Convert vertices
	if (smxModel->FirstChildElement("vertices") != NULL) {

		smdHeader.vtxAddr = ftell(smdFile);
		
		tinyxml2::XMLElement* smxVertices = smxModel->FirstChildElement("vertices");
		smxVertices = smxVertices->FirstChildElement("v");

        while(smxVertices != NULL) {

            SVECTOR vertex;

			vertex.vx = round(param::scaleFactor * atof(smxVertices->Attribute("x")));
			vertex.vy = round(param::scaleFactor * atof(smxVertices->Attribute("y")));
			vertex.vz = round(param::scaleFactor * atof(smxVertices->Attribute("z")));
			vertex.vp = 0;

			fwrite(&vertex, sizeof(SVECTOR), 1, smdFile);
			smdHeader.numverts++;
			
			smxVertices = smxVertices->NextSiblingElement("v");

        }
		
		smdHeader.flags |= 0x1;

	}


	// Convert normals
	if (smxModel->FirstChildElement("normals") != NULL) {

		smdHeader.nrmAddr = ftell(smdFile);

		tinyxml2::XMLElement* smxVertices = smxModel->FirstChildElement("normals");

		smxVertices = smxVertices->FirstChildElement("v");

        while(smxVertices != NULL) {

            SVECTOR vertex;

			vertex.vx = round(4096 * atof(smxVertices->Attribute("x")));
			vertex.vy = round(4096 * atof(smxVertices->Attribute("y")));
			vertex.vz = round(4096 * atof(smxVertices->Attribute("z")));
			vertex.vp = 0;

			fwrite(&vertex, sizeof(SVECTOR), 1, smdFile);
			smdHeader.numnorms++;
			
			smxVertices = smxVertices->NextSiblingElement("v");

        }
		
		smdHeader.flags |= 0x2;

	} else {
		
		smdHeader.numnorms = 0;
		
	}


	if (smxModel->FirstChildElement("primitives") != NULL) {

		smdHeader.priAddr = ftell(smdFile);

		tinyxml2::XMLElement* smxPrimitive = smxModel->FirstChildElement("primitives");
		smxPrimitive = smxPrimitive->FirstChildElement("poly");

		PRIM_ID *prim;
		char pribuff[40];
		char* priptr;
		while(smxPrimitive != NULL) {
			
			const char* primType = smxPrimitive->Attribute("type");
			
			if( primType == NULL ) {
				smxPrimitive = smxPrimitive->NextSiblingElement("poly");
				continue;
			}
			
			memset( pribuff, 0x0, 32 );
			priptr = pribuff;
			
			prim = (PRIM_ID*)priptr;
			
			if( smxPrimitive->IntAttribute( "double" )  )
				prim->nocull = true;
			
			if( ( strcasecmp( "F3", primType ) == 0 ) || 
				( strcasecmp( "FT3", primType ) == 0 ) ) {
				
				prim->type		= PRIM_TYPE_TRI;
				prim->len		= 4;
				
				priptr += sizeof(PRIM_ID);
				
				// Write vertex indices
				((PRIM_V*)priptr)->v0 = atoi( smxPrimitive->Attribute( "v0" ) );
				((PRIM_V*)priptr)->v1 = atoi( smxPrimitive->Attribute( "v1" ) );
				((PRIM_V*)priptr)->v2 = atoi( smxPrimitive->Attribute( "v2" ) );
				((PRIM_V*)priptr)->v3 = 0;
				
				priptr += 8;
				prim->len += 8;
				
				if( strcasecmp( "F", smxPrimitive->Attribute( "shading" ) ) == 0 ) {

					prim->l_type = PRIM_LIGHTING_FLAT;
					
					((PRIM_V*)priptr)->v0 = atoi( smxPrimitive->Attribute( "n0" ) );
					((PRIM_V*)priptr)->v1 = 0;
					priptr += 4;
					prim->len += 4;
							
				} else if( strcasecmp( "S", smxPrimitive->Attribute( "shading" ) ) == 0 ) {

					prim->l_type = PRIM_LIGHTING_SMOOTH;
					
					((PRIM_V*)priptr)->v0 = atoi( smxPrimitive->Attribute( "n0" ) );
					((PRIM_V*)priptr)->v1 = atoi( smxPrimitive->Attribute( "n1" ) );
					((PRIM_V*)priptr)->v2 = atoi( smxPrimitive->Attribute( "n2" ) );
					((PRIM_V*)priptr)->v3 = 0;
					
					priptr += 8;
					prim->len += 8;
					
				}
				
				((PRIM_RGBC*)priptr)->r = atoi( smxPrimitive->Attribute( "r0" ) );
				((PRIM_RGBC*)priptr)->g = atoi( smxPrimitive->Attribute( "g0" ) );
				((PRIM_RGBC*)priptr)->b = atoi( smxPrimitive->Attribute( "b0" ) );
				
				if( smxPrimitive->IntAttribute( "blend" ) > 0 ) {
					((PRIM_RGBC*)priptr)->c = 0x2;
					prim->blend = smxPrimitive->IntAttribute( "blend" )-1;
				}
				
				priptr += 4;
				prim->len += 4;
				
				if( strcasecmp( "FT3", primType ) == 0 ) {	// Textured
					
					TIM_COORDS *tex;
					int uoffs,voffs;
					int texNum = atoi( smxPrimitive->Attribute( "texture" ) );

					if( texNum < 0 ) {

						printf( "ERROR: Primitive with negative texture index encountered.\n" );

						fclose( smdFile );

						if( texCoords != NULL ) {
							free( texCoords );
						}

						return EXIT_FAILURE;

					} else if( texNum > numTextures-1 ) {

						printf( "ERROR: Primitive with texture index greater than specified encountered.\n" );

						fclose( smdFile );

						if( texCoords != NULL ) {
							free( texCoords );
						}

						return EXIT_FAILURE;

					}
					
					tex = &texCoords[texNum];
					
					uoffs = tex->pixdata.px;
					voffs = tex->pixdata.py&0xff;
					
					switch(tex->flag.pmode) {
					case 0:	// 4-bit
						uoffs = (uoffs*4)%256;
						break;
					case 1: // 8-bit
						uoffs = (uoffs*2)%128;
						break;
					case 2: // 16-bit
						uoffs = uoffs%64;
						break;
					}
					
					((PRIM_UV*)priptr)[0].u = smxPrimitive->IntAttribute( "tu0" )+uoffs;
					((PRIM_UV*)priptr)[0].v = smxPrimitive->IntAttribute( "tv0" )+voffs;
					((PRIM_UV*)priptr)[1].u = smxPrimitive->IntAttribute( "tu1" )+uoffs;
					((PRIM_UV*)priptr)[1].v = smxPrimitive->IntAttribute( "tv1" )+voffs;
					((PRIM_UV*)priptr)[2].u = smxPrimitive->IntAttribute( "tu2" )+uoffs;
					((PRIM_UV*)priptr)[2].v = smxPrimitive->IntAttribute( "tv2" )+voffs;
					((PRIM_UV*)priptr)[3].u = 0;
					((PRIM_UV*)priptr)[3].v = 0;
					
					priptr += 8;
					prim->len += 8;
					
					((PRIM_TC*)priptr)->tpage = GetTPage( tex->flag.pmode, 
						prim->blend, tex->pixdata.px, tex->pixdata.py );
                    ((PRIM_TC*)priptr)->clut = GetClut( tex->clutdata.px, tex->clutdata.py );
					
					priptr += 4;
					prim->len += 4;
					
					prim->texture = true;
					
				}
				
				fwrite( pribuff, 1, prim->len, smdFile );
				smdHeader.numprims++;
			
			} else if( strcasecmp( "G3", primType ) == 0 ) {
				
				prim->type		= PRIM_TYPE_TRI;
				prim->len		= 4;
				prim->c_type	= 1;	// Gouraud
				
				priptr += sizeof(PRIM_ID);
				
				// Write vertex indices
				((PRIM_V*)priptr)->v0 = atoi( smxPrimitive->Attribute( "v0" ) );
				((PRIM_V*)priptr)->v1 = atoi( smxPrimitive->Attribute( "v1" ) );
				((PRIM_V*)priptr)->v2 = atoi( smxPrimitive->Attribute( "v2" ) );
				((PRIM_V*)priptr)->v3 = 0;
				
				priptr += 8;
				prim->len += 8;
				
				if( strcasecmp( "F", smxPrimitive->Attribute( "shading" ) ) == 0 ) {

					prim->l_type = PRIM_LIGHTING_FLAT;
					
					((PRIM_V*)priptr)->v0 = atoi( smxPrimitive->Attribute( "n0" ) );
					((PRIM_V*)priptr)->v1 = 0;
					priptr += 4;
					prim->len += 4;
							
				} else if( strcasecmp( "S", smxPrimitive->Attribute( "shading" ) ) == 0 ) {

					prim->l_type = PRIM_LIGHTING_SMOOTH;
					
					((PRIM_V*)priptr)->v0 = atoi( smxPrimitive->Attribute( "n0" ) );
					((PRIM_V*)priptr)->v1 = atoi( smxPrimitive->Attribute( "n1" ) );
					((PRIM_V*)priptr)->v2 = atoi( smxPrimitive->Attribute( "n2" ) );
					((PRIM_V*)priptr)->v3 = 0;
					
					priptr += 8;
					prim->len += 8;
					
				}
				
				((PRIM_RGBC*)priptr)->r = atoi( smxPrimitive->Attribute( "r0" ) );
				((PRIM_RGBC*)priptr)->g = atoi( smxPrimitive->Attribute( "g0" ) );
				((PRIM_RGBC*)priptr)->b = atoi( smxPrimitive->Attribute( "b0" ) );
				if( smxPrimitive->IntAttribute( "blend" ) > 0 ) {
					((PRIM_RGBC*)priptr)->c = 0x2;
					prim->blend = smxPrimitive->IntAttribute( "blend" )-1;
				}
				priptr += 4;
				prim->len += 4;
				
				// Color 1
				((PRIM_RGBC*)priptr)->r = atoi( smxPrimitive->Attribute( "r1" ) );
				((PRIM_RGBC*)priptr)->g = atoi( smxPrimitive->Attribute( "g1" ) );
				((PRIM_RGBC*)priptr)->b = atoi( smxPrimitive->Attribute( "b1" ) );
				priptr += 4;
				prim->len += 4;
				
				// Color 2
				((PRIM_RGBC*)priptr)->r = atoi( smxPrimitive->Attribute( "r2" ) );
				((PRIM_RGBC*)priptr)->g = atoi( smxPrimitive->Attribute( "g2" ) );
				((PRIM_RGBC*)priptr)->b = atoi( smxPrimitive->Attribute( "b2" ) );
				priptr += 4;
				prim->len += 4;
				
				fwrite( pribuff, 1, prim->len, smdFile );
				smdHeader.numprims++;
				
			} else if( ( strcasecmp( "F4", primType ) == 0 ) || 
				( strcasecmp( "FT4", primType ) == 0 ) ) {
				
				prim->type		= PRIM_TYPE_QUAD;
				prim->len		= 4;
				
				priptr += sizeof(PRIM_ID);
				
				// Write vertex indices
				((PRIM_V*)priptr)->v0 = atoi( smxPrimitive->Attribute( "v0" ) );
				((PRIM_V*)priptr)->v1 = atoi( smxPrimitive->Attribute( "v1" ) );
				((PRIM_V*)priptr)->v2 = atoi( smxPrimitive->Attribute( "v2" ) );
				((PRIM_V*)priptr)->v3 = atoi( smxPrimitive->Attribute( "v3" ) );
				
				priptr += 8;
				prim->len += 8;
				
				if( strcasecmp( "F", smxPrimitive->Attribute( "shading" ) ) == 0 ) {

					prim->l_type = PRIM_LIGHTING_FLAT;
					
					((PRIM_V*)priptr)->v0 = atoi( smxPrimitive->Attribute( "n0" ) );
					((PRIM_V*)priptr)->v1 = 0;
					priptr += 4;
					prim->len += 4;
							
				} else if( strcasecmp( "S", smxPrimitive->Attribute( "shading" ) ) == 0 ) {

					prim->l_type = PRIM_LIGHTING_SMOOTH;
					
					((PRIM_V*)priptr)->v0 = atoi( smxPrimitive->Attribute( "n0" ) );
					((PRIM_V*)priptr)->v1 = atoi( smxPrimitive->Attribute( "n1" ) );
					((PRIM_V*)priptr)->v2 = atoi( smxPrimitive->Attribute( "n2" ) );
					((PRIM_V*)priptr)->v3 = atoi( smxPrimitive->Attribute( "n3" ) );
					
					priptr += 8;
					prim->len += 8;
					
				}
				
				((PRIM_RGBC*)priptr)->r = atoi( smxPrimitive->Attribute( "r0" ) );
				((PRIM_RGBC*)priptr)->g = atoi( smxPrimitive->Attribute( "g0" ) );
				((PRIM_RGBC*)priptr)->b = atoi( smxPrimitive->Attribute( "b0" ) );
				
				if( smxPrimitive->IntAttribute( "blend" ) > 0 ) {
					((PRIM_RGBC*)priptr)->c = 0x2;
					prim->blend = smxPrimitive->IntAttribute( "blend" )-1;
				}
				
				priptr += 4;
				prim->len += 4;
				
				if( strcasecmp( "FT4", primType ) == 0 ) {	// Textured
					
					TIM_COORDS *tex;
					int uoffs,voffs;
					int texNum = atoi( smxPrimitive->Attribute( "texture" ) );

					if( texNum < 0 ) {

						printf( "ERROR: Primitive with negative texture index encountered.\n" );

						fclose( smdFile );

						if( texCoords != NULL ) {
							free( texCoords );
						}

						return EXIT_FAILURE;

					} else if( texNum > numTextures-1 ) {

						printf( "ERROR: Primitive with texture index greater than specified encountered.\n" );

						fclose( smdFile );

						if( texCoords != NULL ) {
							free( texCoords );
						}

						return EXIT_FAILURE;

					}
					
					tex = &texCoords[texNum];
					
					uoffs = tex->pixdata.px;
					voffs = tex->pixdata.py&0xff;
					
					switch(tex->flag.pmode) {
					case 0:	// 4-bit
						uoffs = (uoffs*4)%256;
						break;
					case 1: // 8-bit
						uoffs = (uoffs*2)%128;
						break;
					case 2: // 16-bit
						uoffs = uoffs%64;
						break;
					}
					
					((PRIM_UV*)priptr)[0].u = smxPrimitive->IntAttribute( "tu0" )+uoffs;
					((PRIM_UV*)priptr)[0].v = smxPrimitive->IntAttribute( "tv0" )+voffs;
					((PRIM_UV*)priptr)[1].u = smxPrimitive->IntAttribute( "tu1" )+uoffs;
					((PRIM_UV*)priptr)[1].v = smxPrimitive->IntAttribute( "tv1" )+voffs;
					((PRIM_UV*)priptr)[2].u = smxPrimitive->IntAttribute( "tu2" )+uoffs;
					((PRIM_UV*)priptr)[2].v = smxPrimitive->IntAttribute( "tv2" )+voffs;
					((PRIM_UV*)priptr)[3].u = smxPrimitive->IntAttribute( "tu3" )+uoffs;
					((PRIM_UV*)priptr)[3].v = smxPrimitive->IntAttribute( "tv3" )+voffs;
					
					priptr += 8;
					prim->len += 8;
					
					((PRIM_TC*)priptr)->tpage = GetTPage( tex->flag.pmode, 
						prim->blend, tex->pixdata.px, tex->pixdata.py );
                    ((PRIM_TC*)priptr)->clut = GetClut( tex->clutdata.px, tex->clutdata.py );
					
					priptr += 4;
					prim->len += 4;
					
					prim->texture = true;
					
				}
				
				fwrite( pribuff, 1, prim->len, smdFile );
				smdHeader.numprims++;
			
			} else if( strcasecmp( "G4", primType ) == 0 ) {
				
				prim->type		= PRIM_TYPE_QUAD;
				prim->len		= 4;
				prim->c_type	= 1;	// Gouraud
				
				priptr += sizeof(PRIM_ID);
				
				// Write vertex indices
				((PRIM_V*)priptr)->v0 = atoi( smxPrimitive->Attribute( "v0" ) );
				((PRIM_V*)priptr)->v1 = atoi( smxPrimitive->Attribute( "v1" ) );
				((PRIM_V*)priptr)->v2 = atoi( smxPrimitive->Attribute( "v2" ) );
				((PRIM_V*)priptr)->v3 = atoi( smxPrimitive->Attribute( "v3" ) );
				
				priptr += 8;
				prim->len += 8;
				
				if( strcasecmp( "F", smxPrimitive->Attribute( "shading" ) ) == 0 ) {

					prim->l_type = PRIM_LIGHTING_FLAT;
					
					((PRIM_V*)priptr)->v0 = atoi( smxPrimitive->Attribute( "n0" ) );
					((PRIM_V*)priptr)->v1 = 0;
					priptr += 4;
					prim->len += 4;
							
				} else if( strcasecmp( "S", smxPrimitive->Attribute( "shading" ) ) == 0 ) {

					prim->l_type = PRIM_LIGHTING_SMOOTH;
					
					((PRIM_V*)priptr)->v0 = atoi( smxPrimitive->Attribute( "n0" ) );
					((PRIM_V*)priptr)->v1 = atoi( smxPrimitive->Attribute( "n1" ) );
					((PRIM_V*)priptr)->v2 = atoi( smxPrimitive->Attribute( "n2" ) );
					((PRIM_V*)priptr)->v3 = atoi( smxPrimitive->Attribute( "n3" ) );
					
					priptr += 8;
					prim->len += 8;
					
				}
				
				// Color 0
				((PRIM_RGBC*)priptr)->r = atoi( smxPrimitive->Attribute( "r0" ) );
				((PRIM_RGBC*)priptr)->g = atoi( smxPrimitive->Attribute( "g0" ) );
				((PRIM_RGBC*)priptr)->b = atoi( smxPrimitive->Attribute( "b0" ) );
				
				if( smxPrimitive->IntAttribute( "blend" ) > 0 ) {
					((PRIM_RGBC*)priptr)->c = 0x2;
					prim->blend = smxPrimitive->IntAttribute( "blend" )-1;
				}
				
				priptr += 4;
				prim->len += 4;
				
				// Color 1
				((PRIM_RGBC*)priptr)->r = atoi( smxPrimitive->Attribute( "r1" ) );
				((PRIM_RGBC*)priptr)->g = atoi( smxPrimitive->Attribute( "g1" ) );
				((PRIM_RGBC*)priptr)->b = atoi( smxPrimitive->Attribute( "b1" ) );
				((PRIM_RGBC*)priptr)->c = 0;
				priptr += 4;
				prim->len += 4;
				
				// Color 2
				((PRIM_RGBC*)priptr)->r = atoi( smxPrimitive->Attribute( "r2" ) );
				((PRIM_RGBC*)priptr)->g = atoi( smxPrimitive->Attribute( "g2" ) );
				((PRIM_RGBC*)priptr)->b = atoi( smxPrimitive->Attribute( "b2" ) );
				((PRIM_RGBC*)priptr)->c = 0;
				priptr += 4;
				prim->len += 4;
				
				// Color 3
				((PRIM_RGBC*)priptr)->r = atoi( smxPrimitive->Attribute( "r3" ) );
				((PRIM_RGBC*)priptr)->g = atoi( smxPrimitive->Attribute( "g3" ) );
				((PRIM_RGBC*)priptr)->b = atoi( smxPrimitive->Attribute( "b3" ) );
				((PRIM_RGBC*)priptr)->c = 0;
				priptr += 4;
				prim->len += 4;
				
				fwrite( pribuff, 1, prim->len, smdFile );
				smdHeader.numprims++;
				
			} else {
				
				printf( "ERROR: Unknown or unsupported primitive type: %s\n", 
					primType );
				
				fclose( smdFile );
				
				if( texCoords != NULL ) {
					free( texCoords );
				}
				
				return EXIT_FAILURE;
				
			}
			
			smxPrimitive = smxPrimitive->NextSiblingElement("poly");
			
		}
		
		{
			int term = 0;
			fwrite( &term, 1, 4, smdFile );
		}
		
		/*
		while(smxPrimitive != NULL) {

			bool polyBlended = false;
			const char* primType = smxPrimitive->Attribute("type");

			memset(&prim, 0x00, sizeof(PRIM_ID));

			if (strcasecmp("F", smxPrimitive->Attribute("shading")) == 0) {

				prim.lighting = PRIM_LIGHTING_FLAT;

			} else if (strcasecmp("S", smxPrimitive->Attribute("shading")) == 0) {

				prim.lighting = PRIM_LIGHTING_SMOOTH;

			}

			// 3-point polygons
			if ((strcasecmp("F3", primType) == 0) ||
			(strcasecmp("G3", primType) == 0) ||
			(strcasecmp("FT3", primType) == 0) ||
			(strcasecmp("GT3", primType) == 0)) {

				if (strcasecmp("F3", primType) == 0) {

					prim.type		= PRIM_TYPE_TRI;
					prim.len		= 20;

				} else if (strcasecmp("G3", primType) == 0) {

					prim.type		= PRIM_TYPE_TRI;
					prim.gouraud	= true;
					prim.len		= 28;

				} else if (strcasecmp("FT3", primType) == 0) {

					prim.type		= PRIM_TYPE_TRI;
					prim.textured	= true;
					prim.len		= 32;

				} else if (strcasecmp("GT3", primType) == 0) {

					prim.type		= PRIM_TYPE_TRI;
					prim.gouraud	= true;
					prim.textured	= true;
					prim.len		= 42;

				}

				if ( smxPrimitive->IntAttribute("blendmode") > 0 ) {
					
					polyBlended = true;
					prim.blendmode = smxPrimitive->IntAttribute("blendmode")-1;

				}

				if ( smxPrimitive->IntAttribute("doublesided")  ) {

					prim.noculling = true;

				}

				PRIM_V3 vert;

				vert.v0 = atoi(smxPrimitive->Attribute("v0"));
				vert.v1 = atoi(smxPrimitive->Attribute("v1"));
				vert.v2 = atoi(smxPrimitive->Attribute("v2"));

				if ((vert.v0 < 0) || (vert.v1 < 0) || (vert.v2 < 0)) {

					printf("ERROR: Primitive with negative vertex index encountered.\n");

					fclose(smdFile);

					if (texCoords != NULL)
						free(texCoords);

					return EXIT_FAILURE;

				}

				PRIM_V3 norm = { 0 };

				if (prim.lighting == PRIM_LIGHTING_FLAT) {

					norm.v0 = atoi(smxPrimitive->Attribute("n0"));
					prim.len += 4;

				} else if (prim.lighting == PRIM_LIGHTING_SMOOTH) {

					norm.v0 = atoi(smxPrimitive->Attribute("n0"));
					norm.v1 = atoi(smxPrimitive->Attribute("n1"));
					norm.v2 = atoi(smxPrimitive->Attribute("n2"));
					prim.len += 12;

				}

				if ((norm.v0 < 0) || (norm.v1 < 0) || (norm.v2 < 0)) {

					printf("ERROR: Primitive with negative normal index encountered.\n");

					fclose(smdFile);

					if (texCoords != NULL)
						free(texCoords);

					return EXIT_FAILURE;

				}

				fwrite(&prim, sizeof(PRIM_ID), 1, smdFile);
				fwrite(&vert, sizeof(PRIM_V3), 1, smdFile);

				if (prim.lighting == PRIM_LIGHTING_FLAT) {
					fwrite(&norm, 4, 1, smdFile);
				} else if (prim.lighting == PRIM_LIGHTING_SMOOTH) {
					fwrite(&norm, sizeof(PRIM_V3), 1, smdFile);
				}

				PRIM_RGBC col;

                col.r = atoi(smxPrimitive->Attribute("r0"));
                col.g = atoi(smxPrimitive->Attribute("g0"));
                col.b = atoi(smxPrimitive->Attribute("b0"));
                col.c = 0x00;

                if (polyBlended) {
					col.c |= 0x2;
				}

				fwrite(&col, sizeof(PRIM_RGBC), 1, smdFile);

				if ((strcasecmp("G3", primType) == 0) ||
				(strcasecmp("GT3", primType) == 0)) {

					col.r = atoi(smxPrimitive->Attribute("r1"));
					col.g = atoi(smxPrimitive->Attribute("g1"));
					col.b = atoi(smxPrimitive->Attribute("b1"));
					col.c = 0x00;

					fwrite(&col, sizeof(PRIM_RGBC), 1, smdFile);

					col.r = atoi(smxPrimitive->Attribute("r2"));
					col.g = atoi(smxPrimitive->Attribute("g2"));
					col.b = atoi(smxPrimitive->Attribute("b2"));
					col.c = 0x00;

					fwrite(&col, sizeof(PRIM_RGBC), 1, smdFile);

				}

				if ((strcasecmp("FT3", primType) == 0) || (strcasecmp("GT3", primType) == 0)) {

					PRIM_TC tc;
					PRIM_UV uvcd;

					int texNum = atoi(smxPrimitive->Attribute("texture"));

					if (texNum < 0) {

						printf("ERROR: Primitive with negative texture index encountered.\n");

						fclose(smdFile);

						if (texCoords != NULL)
							free(texCoords);

						return EXIT_FAILURE;

					} else if (texNum > numTextures-1) {

						printf("ERROR: Primitive with texture index greater than specified encountered.\n");

						fclose(smdFile);

						if (texCoords != NULL)
							free(texCoords);

						return EXIT_FAILURE;

					}

					TIM_COORDS *tex = &texCoords[texNum];

					int uoffs = tex->pixdata.px;
					int voffs = tex->pixdata.py&0xff;
					
					switch(tex->flag.pmode) {
					case 0:	// 4-bit
						uoffs = (uoffs*4)%256;
						break;
					case 1: // 8-bit
						uoffs = (uoffs*2)%128;
						break;
					case 2: // 16-bit
						uoffs = uoffs%64;
						break;
					}
					
                    // TPAGE and CLUT
                    tc.tpage = GetTPage( tex->flag.pmode, prim.blendmode, 
						tex->pixdata.px, tex->pixdata.py );					
                    tc.clut = GetClut( tex->clutdata.px, tex->clutdata.py );
					fwrite(&tc, sizeof(PRIM_TC), 1, smdFile);
					
					// Texcoords
                    uvcd.u = smxPrimitive->IntAttribute("tu0")+uoffs;
                    uvcd.v = smxPrimitive->IntAttribute("tv0")+voffs;
					fwrite(&uvcd, sizeof(PRIM_UV), 1, smdFile);

					uvcd.u = smxPrimitive->IntAttribute("tu1")+uoffs;
                    uvcd.v = smxPrimitive->IntAttribute("tv1")+voffs;
					fwrite(&uvcd, sizeof(PRIM_UV), 1, smdFile);

					uvcd.u = smxPrimitive->IntAttribute("tu2")+uoffs;
                    uvcd.v = smxPrimitive->IntAttribute("tv2")+voffs;
					fwrite(&uvcd, sizeof(PRIM_UV), 1, smdFile);

					// Padding
					uvcd.u = uvcd.v = 0;
					fwrite(&uvcd, sizeof(PRIM_UV), 1, smdFile);
					
				}

			} else {

				printf("Unsupported primitive %s, ignoring...\n", primType);

			}

			smxPrimitive = smxPrimitive->NextSiblingElement("poly");

		}

		// Write terminator
		memset(&prim, 0x00, sizeof(PRIM_ID));
		fwrite(&prim, sizeof(PRIM_ID), 1, smdFile);
		*/

	}


	strcpy(smdHeader.id, "SMD");
	smdHeader.version = 1;

	fseek(smdFile, 0, SEEK_SET);
	fwrite(&smdHeader, sizeof(SMD_HEADER), 1, smdFile);

	fclose(smdFile);

	if (texCoords != NULL)
		free(texCoords);

	printf("Converted successfully.\n");

	return EXIT_SUCCESS;

}
