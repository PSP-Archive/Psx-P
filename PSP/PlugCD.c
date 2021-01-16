
#include "plugins.h"
#include "PlugCD.h"
#include "zlib.h"
//ZEXTERN int ZEXPORT    gzread  OF((gzFile file, voidp buf, unsigned len));

#ifdef COMPRESSED_IO
	#define myread(file,buf,len) gzread(file,buf,len)
	#define mywrite gzwrite
	#define myseek gzseek
	#define mytell gztell
	#define myopen gzopen
	#define myclose gzclose
	#define MYFILE gzFile
#else
	#define myread(buf,size,cnt,stream) fread(buf,size,cnt,stream)
	#define mywrite fwrite
	#define myseek fseek
	#define mytell ftell
	#define myopen fopen
	#define myclose fclose
	#define MYFILE FILE

#endif

// gets track 
long getTN(unsigned char* buffer){
	int numtracks = getNumTracks();

	if (-1 == numtracks)
	{
		return -1;
	}

	buffer[0]=1;
	buffer[1]=numtracks;
	return 0;
}

 // if track==0 -> return total length of cd
 // otherwise return start in bcd time format
long getTD(int track, unsigned char* buffer)
{
      // lasttrack just keeps track of which track TD was requested last (go fig)
   if (track > CD.numtracks)
   {
//      printf("getTD bad %2d\n", track);
      return -1;
   }

   if (track == 0)
   {
      buffer[0] = CD.tl[track].end[0];
      buffer[1] = CD.tl[track].end[1];
      buffer[2] = CD.tl[track].end[2];
   }
   else
   {
      buffer[0] = CD.tl[track].start[0];
      buffer[1] = CD.tl[track].start[1];
      buffer[2] = CD.tl[track].start[2];
   }
//   printf("getTD %2d %02d:%02d:%02d\n", track, (int)buffer[0],
//         (int)buffer[1], (int)buffer[2]);

   // bcd encode it
   buffer[0] = intToBCD(buffer[0]);
   buffer[1] = intToBCD(buffer[1]);
   buffer[2] = intToBCD(buffer[2]);
   SysPrintf("end getTD()\r\n");
   return 0;
}

// opens a binary cd image and calculates its length
void openBin(const char* filename)
{
    long end, size, blocks;

    SysPrintf("start openBin() %s \r\n",filename);

    CD.cd = myopen(filename, "rb");

    printf("CD.cd = %08x\n",CD.cd);

	if (CD.cd == NULL){
		//DLog("Could not open file %s",filename);
		
	}
	
    if (CD.cd == 0)
    {
        //DLog("Error opening cd\r\n");
        exit(0);
    }

    end = myseek(CD.cd, 0, SEEK_END);
    end = mytell(CD.cd);
    size = end;
    printf("size of CD in MB = %d\r\n",size/1048576);

    rc = myseek(CD.cd, 0, SEEK_SET);
    blocks = size / 2352;
	//DLog("# of blocks %d",blocks);
    // put the track length info in the track list
    CD.tl = (Track*) malloc(sizeof(Track));

    CD.tl[0].type = Mode2;
    CD.tl[0].num = 1;
    CD.tl[0].start[0] = 0;
    CD.tl[0].start[1] = 0;
    CD.tl[0].start[2] = 0;
    CD.tl[0].end[2] = blocks % 75;
    CD.tl[0].end[1] = ((blocks - CD.tl[0].end[2]) / 75) % 60;
    CD.tl[0].end[0] = (((blocks - CD.tl[0].end[2]) / 75) - CD.tl[0].end[1]) / 60;

    CD.tl[0].start[1] += 2;
    CD.tl[0].end[1] += 2;

    normalizeTime(CD.tl[0].end);

    CD.numtracks = 1;

    SysPrintf("end openBin()\r\n");
}

void addBinTrackInfo()
{
    SysPrintf("start addBinTrackInfo()\r\n");

    //CD.tl = realloc(CD.tl, (CD.numtracks + 1) * sizeof(Track));
	CD.tl = malloc((CD.numtracks + 1) * sizeof(Track));
    CD.tl[CD.numtracks].end[0] = CD.tl[0].end[0];
    CD.tl[CD.numtracks].end[1] = CD.tl[0].end[1];
    CD.tl[CD.numtracks].end[2] = CD.tl[0].end[2];
    CD.tl[CD.numtracks].start[0] = CD.tl[0].start[0];
    CD.tl[CD.numtracks].start[1] = CD.tl[0].start[1];
    CD.tl[CD.numtracks].start[2] = CD.tl[0].start[2];

    SysPrintf("end addBinTrackInfo()\r\n");
}

// new file types should be added here and in the CDOpen function
void newCD(const char * filename)
{
    SysPrintf("start newCD()\r\n");

 	SysPrintf("Opening binary file");
	CD.type = Bin;
	openBin(filename);
	addBinTrackInfo();
	CD.bufferPos = 0x7FFFFFFF;
	seekSector(0,2,0);

    SysPrintf("end newCD\r\n");
}


// return the sector address - the buffer address + 12 bytes for offset.
unsigned char* getSector()
{
  //  SysPrintf("getSector() %d %d %d \r\n",CD.buffer,CD.sector,CD.bufferPos);
    return CD.buffer + (CD.sector - CD.bufferPos) + 12;//
//	SysPrintf("end getSector()\r\n");
}

// returns the number of tracks
char getNumTracks()
{
    SysPrintf("start getNumTracks()\r\n");
    // if there's no open cd, return -1
    if (CD.cd == 0) {
        return -1;
    }

//    printf("numtracks %d\n",CD.numtracks);
    SysPrintf("end getNumTracks()\r\n");
    return CD.numtracks;
}

void readit(const unsigned char m, const unsigned char s, const unsigned char f)
{
//    SysPrintf("start readit()\r\n");
//    printf(" not cached %08x %08x\n",CD.sector - CD.bufferPos, BUFFER_SIZE);

    // fakie ISO support.  iso is just cd-xa data without the ecc and header.
    // read in the same number of sectors then space it out to look like cd-xa
	/*
          if (CD.type == Iso)
          {
             unsigned char temptime[3];
             long tempsector = (( (m * 60) + (s - 2)) * 75 + f) * 2048;
//             fs_seek(CD.cd, tempsector, SEEK_SET);
  //           fs_read(CD.buffer, sizeof(unsigned char), 2048*BUFFER_SECTORS, CD.cd);
			 SysPrintf("Seeking\n");
             myseek(CD.cd, tempsector, SEEK_SET);
#ifdef COMPRESSED_IO
             myread(CD.cd, CD.buffer, 2048*BUFFER_SECTORS );
#else
			 myread(CD.buffer, 2048*BUFFER_SECTORS,1, CD.cd);
#endif
             // spacing out the data here...
             for(tempsector = BUFFER_SECTORS - 1; tempsector >= 0; tempsector--)
             {
                memcpy(&CD.buffer[tempsector*2352 + 24],&CD.buffer[tempsector*2048], 2048);

                // two things - add the m/s/f flags in case anyone is looking
                // and change the xa mode to 1
                temptime[0] = m;
                temptime[1] = s;
                temptime[2] = f + (unsigned char)tempsector;
                
                normalizeTime(temptime);
                CD.buffer[tempsector*2352+12] = intToBCD(temptime[0]);
                CD.buffer[tempsector*2352+12+1] = intToBCD(temptime[1]);
                CD.buffer[tempsector*2352+12+2] = intToBCD(temptime[2]);
                CD.buffer[tempsector*2352+12+3] = 0x01;
             }
          }
          else 
		  */
    {
        rc = myseek(CD.cd, CD.sector, SEEK_SET);
        printf(" seek1 rc %d\n", rc);
#ifdef COMPRESSED_IO
        rc = myread(CD.cd, CD.buffer, sizeof(unsigned char) * BUFFER_SIZE);
#else
//        rc = myread(CD.buffer, sizeof(unsigned char) * BUFFER_SIZE, 1, CD.cd);
		  rc = myread(CD.buffer, sizeof(unsigned char) * BUFFER_SIZE, 1, CD.cd);
#endif
        printf(" seek2 rc %d\n", rc);
    }

    CD.bufferPos = CD.sector;

  //  SysPrintf("end readit()\r\n");
}


void seekSector(const unsigned char m, const unsigned char s, const unsigned char f)
{
    CD.sector = (( (m * 60) + (s - 2)) * 75 + f) * 2352;
    if ((CD.sector >= CD.bufferPos) &&
            (CD.sector < (CD.bufferPos + BUFFER_SIZE)) ) {
        return;
    }
    // not cached - read a few blocks into the cache
    else
    {
        readit(m,s,f);
    }

}
extern char romname[256];
extern int ROM_LOAD_TYPE;//psx file =0, bin =1, bin.gz =2

long CDR__open(void)
{
    SysPrintf("start CDR_open()\r\n");

    SysPrintf("end CDR_open()\r\n");
    return 0;
}

long CDR__init(void) {
 //strcpy(CDConfiguration.fn, "spiderman.bin"); // dlcm.psx");
//	strcpy(CDConfiguration.fn, "pdx-dlcm.psx");
//	strcpy(CDConfiguration.fn, "voxpsx");
    return 0;
}

long CDR__shutdown(void) {
    return 0;
}

long CDR__close(void) {
    SysPrintf("start CDR_close()\r\n");
    myclose(CD.cd);
    free(CD.tl);
    SysPrintf("end CDR_close()\r\n");
    return 0;
}

long CDR__getTN(unsigned char *buffer) {
    SysPrintf("start CDR_getTN()\r\n");
//    SysPrintf("end CDR_getTN()\r\n");
    return getTN(buffer);
//    return 0;
}

long CDR__getTD(unsigned char track, unsigned char *buffer) {
    SysPrintf("start CDR_getTD()\r\n");
//    printf("getTD from track %d\n", track);

   unsigned char temp[3];
   int result = getTD((int)track, temp);

   if (result == -1) return -1;

   buffer[1] = temp[1];
   buffer[2] = temp[0];

    SysPrintf("end CDR_getTD()\r\n");
    return 0;
}

long CDR__readTrack(unsigned char *time) {
//    SysPrintf("start CDR_readTrack()\r\n");
//    printf("readTrack at %02d:%02d:%02d\n", BCDToInt(time[0]), BCDToInt(time[1]), BCDToInt(time[2]));

    //	cdrom_reinit();

    seekSector(BCDToInt(time[0]), BCDToInt(time[1]), BCDToInt(time[2]));

    //SysPrintf("end CDR_readTrack()\r\n");
    return 0;
}

unsigned char *CDR__getBuffer(void) {
//    SysPrintf("start CDR_getBuffer()\r\n");
    return getSector();
}

/*
long (CALLBACK* CDRconfigure)(void);
long (CALLBACK* CDRtest)(void);
void (CALLBACK* CDRabout)(void);
long (CALLBACK* CDRplay)(unsigned char *);
long (CALLBACK* CDRstop)(void);
*/
