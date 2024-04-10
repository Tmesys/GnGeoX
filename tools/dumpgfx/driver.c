/*  gngeo a neogeo emulator
 *  Copyright (C) 2001 Peponas Mathieu
 * 
 *  This program is free software; you can redistribute it and/or modify  
 *  it under the terms of the GNU General Public License as published by   
 *  the Free Software Foundation; either version 2 of the License, or    
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <zlib.h>
#include "unzip.h"
#include "type.h"
#include "driver.h"
#include "neocrypt.h"

static LIST *driver_list=NULL;
enum {
    TILE_UNCONVERTED=0,
    TILE_NORMAL,
    TILE_INVISIBLE,
    TILE_TRANSPARENT25,
    TILE_TRANSPARENT50
};

neogeoroms memory;

extern int neogeo_fix_bank_type;

#define CHECK_ALLOC(a) {if (!a) {printf("Out of Memory\n");exit(1);}}

void convert_tile(int tileno)
{
    unsigned char swap[128];
    unsigned int *gfxdata;
    int x,y;
    unsigned int pen;
    gfxdata = (unsigned int *)&memory.gfx[ tileno<<7];
  
    memcpy(swap,gfxdata,128);

    for (y = 0;y < 16;y++) {
        unsigned int dw;
    
        dw = 0;
        for (x = 0;x < 8;x++)
        {
            pen  = ((swap[64 + (y<<2) + 3] >> x) & 1) << 3;
            pen |= ((swap[64 + (y<<2) + 1] >> x) & 1) << 2;
            pen |= ((swap[64 + (y<<2) + 2] >> x) & 1) << 1;
            pen |=  (swap[64 + (y<<2)    ] >> x) & 1;
            dw |= pen << ((7-x)<<2);
            memory.pen_usage[tileno]  |= (1 << pen);
        }
		*(gfxdata++) = dw;
     
        dw = 0;
        for (x = 0;x < 8;x++)
        {
            pen  = ((swap[(y<<2) + 3] >> x) & 1) << 3;
            pen |= ((swap[(y<<2) + 1] >> x) & 1) << 2;
            pen |= ((swap[(y<<2) + 2] >> x) & 1) << 1;
            pen |=  (swap[(y<<2)    ] >> x) & 1;
            dw |= pen << ((7-x)<<2);
            memory.pen_usage[tileno]  |= (1 << pen);
        }
		*(gfxdata++) = dw;
    }
    if ((memory.pen_usage[tileno] & ~1) == 0) {
        memory.pen_usage[tileno]=TILE_INVISIBLE;
    } else {
	    memory.pen_usage[tileno]=TILE_NORMAL;
    }
  
}

#if 0
void convert_all_char(Uint8 *Ptr, int Taille, 
		      Uint8 *usage_ptr)
{
    int		i,j;
    unsigned char	usage;
    
    Uint8 *Src;
    Uint8 *sav_src;

    Src=(Uint8*)malloc(Taille);
    if (!Src) {
	printf("Not enought memory!!\n");
	return;
    }
    sav_src=Src;
    memcpy(Src,Ptr,Taille);
#ifdef WORDS_BIGENDIAN
#define CONVERT_TILE *Ptr++ = *(Src+8);\
	             usage |= *(Src+8);\
                     *Ptr++ = *(Src);\
		     usage |= *(Src);\
		     *Ptr++ = *(Src+24);\
		     usage |= *(Src+24);\
		     *Ptr++ = *(Src+16);\
		     usage |= *(Src+16);\
		     Src++;
#else
#define CONVERT_TILE *Ptr++ = *(Src+16);\
	             usage |= *(Src+16);\
                     *Ptr++ = *(Src+24);\
		     usage |= *(Src+24);\
		     *Ptr++ = *(Src);\
		     usage |= *(Src);\
		     *Ptr++ = *(Src+8);\
		     usage |= *(Src+8);\
		     Src++;
#endif
    for(i=Taille;i>0;i-=32) {
        usage = 0;
        for (j=0;j<8;j++) {
            CONVERT_TILE
                }
        Src+=24;
        *usage_ptr++ = usage;
    }
    free(sav_src);
#undef CONVERT_TILE
}
#endif

/* For MGD-2 dumps */
static int mgd2_tile_pos=0;
void convert_mgd2_tiles(unsigned char *buf,int len)
{
    int i;
    unsigned char t;

    if (len==memory.gfx_size && mgd2_tile_pos==memory.gfx_size) {
	mgd2_tile_pos=0;
    }
    if (len == 2) {
	
	
	return;
    }

    if (len == 6)
    {
        unsigned char swp[6];

        memcpy(swp,buf,6);
        buf[0] = swp[0];
        buf[1] = swp[3];
        buf[2] = swp[1];
        buf[3] = swp[4];
        buf[4] = swp[2];
        buf[5] = swp[5];

        return;
    }

    if (len % 4) exit(1);	/* must not happen */

    len /= 2;

    for (i = 0;i < len/2;i++)
    {
        t = buf[len/2 + i];
        buf[len/2 + i] = buf[len + i];
        buf[len + i] = t;
    }
    if (len==2) {
	mgd2_tile_pos+=2;
    }
    convert_mgd2_tiles(buf,len);
    convert_mgd2_tiles(buf + len,len);
}


static SDL_bool goto_next_driver(FILE * f)
{
    char buf[512];
    char game[10], type[10];
    long pos;

    while (!feof(f)) {
	pos=ftell(f);
	fgets(buf, 510, f);
	if (sscanf(buf, "game %10s %10s", game, type) >= 2) {
	    fseek(f,pos,SEEK_SET);
	    return SDL_TRUE;
	}
    }
    return SDL_FALSE;
}

static int get_sec_by_name(char *section) {
    static char *sec[SEC_MAX]={"CPU","SFIX","SM1","SOUND1","SOUND2","GFX"};
    int i;
    for(i=0;i<SEC_MAX;i++) {
	if (strcmp(sec[i],section)==0)
	    return i;
    }
    /*printf("Unknow section %s\n",section);*/
    return -1;
}

static int get_romtype_by_name(char *type) {
    static char *rt[ROMTYPE_MAX]={"MGD2","MVS","MVS_CMC42","MVS_CMC50"};
    int i;
    for(i=0;i<ROMTYPE_MAX;i++) {
	if (strcmp(rt[i],type)==0)
	    return i;
    }
    printf("Unknown rom type %s\n",type);
    return -1;
}

static void add_driver_section(Uint32 s, SECTION *sec,FILE *f) {
    char buf[512], a[64], b[64];
    Uint32 size, offset;
    sec->size=s;
    sec->item=NULL;
    //  printf("SECTION size=%x\n",sec->size);
    while (1) {
        SECTION_ITEM *item;
        fgets(buf, 511, f);
        if (strcmp(buf, "END\n") == 0)
            break;
        sscanf(buf, "%s %x %x %s\n", a, &offset, &size, b);
        item=malloc(sizeof(SECTION_ITEM));
	CHECK_ALLOC(item);

        item->filename=strdup(a);
        item->begin=offset;
        item->size=size;
        item->type=(strcmp(b,"ALTERNATE")==0?LD_ALTERNATE:LD_NORM);
        sec->item=list_append(sec->item,item);
	//    printf("%s %x %x %d\n",item->filename,item->begin,item->size,item->type);
    }
}

/* add driver define in f */
static void add_driver(FILE *f)
{
    DRIVER *dr;
    char buf[512], a[64], b[64];
    Uint32 s, size, offset;
    int sec;
    char game[10], type[10];
    char *t;

    dr=malloc(sizeof(DRIVER));
    CHECK_ALLOC(dr);

    /* TODO: driver creation */
    fgets(buf, 510, f);
    sscanf(buf, "game %s %s", game, type);
    //printf("buf== %s | %s | %s\n",buf,game,type);
    dr->name=strdup(game);
    dr->rom_type=get_romtype_by_name(type);
    dr->special_bios=0;
    dr->banksw_type=BANKSW_NORMAL;
    dr->sfix_bank=0;
    t = strchr(buf, '"');
    if (t) {
	char *e;
	t+=1;
	e=strchr(t, '"');
	if (e) {
	    e[0]=0;
	    dr->longname=strdup(t);
	} else
	    dr->longname=NULL;
    }
    else dr->longname=NULL;

    //printf("Add %8s %s | %s \n",dr->name,type,dr->longname);

    while (1) {
	fgets(buf, 511, f);
	if (strcmp(buf, "END\n") == 0)
	    break;
	sscanf(buf, "%s %x\n", a, &s);
	//printf("SEC %s\n",a);
	sec=get_sec_by_name(a);
	if (sec==-1) {
	    int b=0;
	    if (strcmp(a,"BIOS")==0) { add_driver_section(s,&(dr->bios), f); dr->special_bios=1;}
	    else if (strcmp(a,"XOR")==0) { dr->xor=s; }
	    else if (strcmp(a,"SFIXBANK")==0) { dr->sfix_bank=s; }
	    else if (strcmp(a,"BANKSWITCH")==0) {
		dr->banksw_type=s;
		if(s==BANKSW_SCRAMBLE) {
		    /* not implemented yet */
		    fgets(buf,511,f);
		    sscanf(buf,"%x",&dr->banksw_addr);
		    fgets(buf, 511, f);
		    sscanf(buf, "%d %d %d %d %d %d\n",
			   (int *) &dr->banksw_unscramble[0],
			   (int *) &dr->banksw_unscramble[1],
			   (int *) &dr->banksw_unscramble[2],
			   (int *) &dr->banksw_unscramble[3],
			   (int *) &dr->banksw_unscramble[4],
			   (int *) &dr->banksw_unscramble[5]);
		    while (1) {
			fgets(buf, 511, f);
			if (strcmp(buf, "END\n") == 0)
			    break;
			sscanf(buf,"%x\n", &dr->banksw_off[b]);
			b++;
		    }
		}
	    } else {
		printf("Unknown section %s\n",a);
		return;
	    }
	} else add_driver_section(s,&(dr->section[sec]),f);
    }


    driver_list=list_prepend(driver_list,dr);
}

static void print_driver(void *data) {
    DRIVER *dr=data;
    int i;
    printf("game %8s | %s \n",dr->name,dr->longname);
    for (i=0;i<SEC_MAX;i++) {
	LIST *l;
	printf("Section %d\n",i);
	for(l=dr->section[i].item;l;l=l->next) {
	    SECTION_ITEM *item=l->data;
	    printf("%8s %x %x\n",item->filename,item->begin,item->size);
	}
    }
}

SDL_bool dr_load_driver_dir(char *dirname) {
    DIR *pdir;
    struct stat buf;
    struct dirent *pfile;
    if (!(pdir=opendir (dirname))) {
	    // printf("Couldn't find %s\n",dirname);
        return SDL_FALSE; 
    }
    while(pfile=readdir(pdir)) {
        char *filename=alloca(strlen(pfile->d_name)+strlen(dirname)+2);
        sprintf(filename,"%s/%s",dirname,pfile->d_name);
        stat(filename,&buf);
        if (S_ISREG(buf.st_mode)) {
            dr_load_driver(filename);
        }
    }
    closedir(pdir);
    return SDL_TRUE;
}

/* load the specified file, and create the driver struct */
SDL_bool dr_load_driver(char *filename) {
    FILE *f;
    LIST *i;


    f=fopen(filename,"rb");
    if (!f) {
	printf("Couldn't find %s\n",filename);
	return SDL_FALSE;
    }
    while(goto_next_driver(f)==SDL_TRUE) {
	add_driver(f);
    }

    //list_foreach(driver_list,print_driver);

    fclose(f);
    return SDL_TRUE;
}

void dr_list_all(void);
void dr_list_available(void);

static SDL_bool file_is_zip(char *name) {
    unzFile *gz;
    gz=unzOpen(name);
    if (gz!=NULL) {
	unzClose(gz);
	return SDL_TRUE;
    }
    return SDL_FALSE;
}

static void free_ziplist_item(void *data) {
    free(data);
}

/* check if the driver dr correspond to the zip file pointed by gz 
   (zip_list contain the zip file content)
*/
static SDL_bool check_driver_for_zip(DRIVER *dr,unzFile *gz,LIST *zip_list) {
    int i;
    LIST *l,*zl;

    for (i=0;i<SEC_MAX;i++) {
	//printf("Check section %d\n",i);
	for(l=dr->section[i].item;l;l=l->next) {
	    SECTION_ITEM *item=l->data;
	    if (strcmp(item->filename,"-")!=0) {
		for(zl=zip_list;zl;zl=zl->next) {
		    //printf("Check filename %s %s\n",(char*)zl->data,item->filename);
		    if (strcasecmp(item->filename,(char*)zl->data)==0) {
			//printf("filename %s=%s\n",(char*)zl->data,item->filename);
			break;
		    }
		}
		//printf("Zl %s = %p\n",item->filename,zl);
		if (zl==NULL)
		    return SDL_FALSE;
	    }
	   
	}
    }
    
    return SDL_TRUE;
}

#if 0
char *get_zip_name(char *name) {
    char *zip;
    char *path = CF_STR(cf_get_item_by_name("rompath"));
    if (file_is_zip(name)) {
	    zip=malloc(strlen(name)+1); CHECK_ALLOC(zip)
        strcpy(zip,name);
    } else {
        int len = strlen(path) + strlen(name) + 6;
        zip = malloc(len); CHECK_ALLOC(zip)
        sprintf(zip,"%s/%s.zip",path,name);
    }
    return zip;
}
#endif

/* return the correct driver for the zip file zip*/

DRIVER *get_driver_for_zip(char *zip) {
    unzFile *gz;
    int i;
    char zfilename[256];
    char *t;
    LIST *zip_list=NULL,*l,*zl;
    int res;

    /* first, we check if it a zip */
    gz = unzOpen(zip);
    if (gz==NULL) {
	return NULL;
    }
    //printf("Get driver for %s\n",zip);
    /* now, we create a list containing the content of the zip */
    i=0;
    unzGoToFirstFile(gz);
    do {
	unzGetCurrentFileInfo(gz,NULL,zfilename,256,NULL,0,NULL,0);
	//printf("List zip %s\n",zfilename);
	t=strrchr(zfilename,'.');
	if (! ( (strncasecmp(zfilename,"n",1)==0 && strlen(zfilename)<=12 )|| 
		(t && (strcasecmp(t,".rom")==0 || strcasecmp(t,".bin")==0) ) )
	    )
	    
	    i++;
	if (i>10) {
	    //printf("More than 10 file are not rom....\n");
	    /* more than 10 files are not rom.... must not be a valid romset 
	       10 files should be enough */
	    list_erase_all(zip_list,free_ziplist_item);
	    return NULL;
	}
	zip_list=list_prepend(zip_list,strdup(zfilename));
    } while (unzGoToNextFile(gz)!=UNZ_END_OF_LIST_OF_FILE);
    
    /* now we check every driver to see if it match the zip content */
    for (l=driver_list;l;l=l->next) {
	DRIVER *dr=l->data;
	if (check_driver_for_zip(dr,gz,zip_list)==SDL_TRUE) {
	    unzClose(gz);
	    list_erase_all(zip_list,free_ziplist_item);
	    return dr;
	}
    }
		
    list_erase_all(zip_list,free_ziplist_item);
    unzClose(gz);
    /* not match found */
    return NULL;
}

DRIVER *dr_get_by_name(char *name) {
    //char *zip=get_zip_name(name);
    char *zip=name;
    DRIVER *dr=get_driver_for_zip(zip);
    free(zip);
    return dr;
}

static int zfread(unzFile * f, void *buffer, int length)
{
    Uint8 *buf = (Uint8*)buffer;
    Uint8 *tempbuf;
    Uint32 totread, r, i;
    int totlength=length;
    totread = 0;
    tempbuf=alloca(4097);

    while (length) {

	r = length;
	if (r > 4096)
	    r = 4096;

	r = unzReadCurrentFile(f, tempbuf, r);
	if (r == 0) {
	    return totread;
	}
	memcpy(buf, tempbuf, r);

	buf += r;
	totread += r;
	length -= r;
    }

    return totread;
}

static int zfread_alternate(unzFile * f, void *buffer, int length, int inc)
{
    Uint8 *buf = buffer;
    Uint8 tempbuf[4096];
    Uint32 totread, r, i;
    int totlength=length;
    totread = 0;
    

    while (length) {

	r = length;
	if (r > 4096)
	    r = 4096;

	r = unzReadCurrentFile(f, tempbuf, r);
	if (r == 0) {
	    return totread;
	}
	for (i = 0; i < r; i++) {
	    *buf = tempbuf[i];
	    buf += inc;
	}
	totread += r;
	length -= r;
    }

    return totread;
}

SDL_bool dr_load_section(unzFile *gz, SECTION s, Uint8 *current_buf) {
    LIST *l;
    for(l=s.item;l;l=l->next) {
	SECTION_ITEM *item=l->data;
    
	if (strcmp(item->filename,"-")!=0) {
	    /* nouveau fichier, on l'ouvre */
	    if (unzLocateFile(gz, item->filename, 2) == UNZ_END_OF_LIST_OF_FILE) {
		unzClose(gz);
		return SDL_FALSE;
	    }
	    if (unzOpenCurrentFile(gz) != UNZ_OK) {
		unzClose(gz);
		return SDL_FALSE;
	    }
	}
	if (item->type==LD_ALTERNATE)
	    zfread_alternate(gz, current_buf + item->begin, item->size, 2);
	else
	    zfread(gz, current_buf + item->begin, item->size);
    }
    return SDL_TRUE;
}

SDL_bool dump_gfx(char *name) {
    FILE *f;
    char filename[512];
    sprintf(filename,"%s.gfx",name);
    f=fopen(filename,"wb");
    if (!f) return SDL_FALSE;
    fwrite(memory.gfx,memory.gfx_size,1,f);
    fwrite(memory.pen_usage,(memory.gfx_size >> 7)*sizeof(Uint32),1,f);
    fclose(f);
}    

SDL_bool dr_load_game(DRIVER *dr,char *name) {
    unzFile *gz;
    int i;
    LIST *l;
    //    char *zip=get_zip_name(name);
    char *zip=name;
    gz = unzOpen(zip);
    free(zip);
    if (gz==NULL) {
	return SDL_FALSE;
    }

    for (i=0;i<SEC_MAX;i++) {
	int s=dr->section[i].size;
	Uint8 *current_buf=NULL;
	//if (dr->section[i].item==NULL) continue;
	if (s==0) continue;
	//      printf("%p %d \n",dr->section[i].item,i);
	switch (i) {
	case SEC_CPU:

	    break;
	case SEC_SFIX:
        //printf("alloc sfix memory(%d)...\n", s);
		memory.sfix_game = malloc(s); CHECK_ALLOC(memory.sfix_game);
		memory.sfix_size = s;
		memory.fix_game_usage = malloc(s >> 5);
		CHECK_ALLOC(memory.fix_game_usage);
	    break;
	case SEC_SM1:
	    break;
	case SEC_SOUND1:
		//memory.sound1 = malloc(s); CHECK_ALLOC(memory.sound1);
		memory.sound1_size = s;
		break;
	case SEC_SOUND2:
		//memory.sound2 = malloc(s); CHECK_ALLOC(memory.sound2);
		memory.sound2_size = s;
		break;
	case SEC_GFX:
		memory.gfx = malloc(s); 
		CHECK_ALLOC(memory.gfx);
		//printf("Alloc %x for GFX: %p\n",s,memory.gfx);
		memory.gfx_size = s;
		current_buf = memory.gfx;
		memory.pen_usage = malloc((s >> 7) * sizeof(int));
		CHECK_ALLOC(memory.pen_usage);
		memset(memory.pen_usage, 0, (s >> 7) * sizeof(int));
		memory.nb_of_tiles = s >> 7;
	    break;
	    /* TODO: Crypted rom */
	default:
	    break;
	}
	if (current_buf) {
		if (!dr_load_section(gz,dr->section[i],current_buf)) return SDL_FALSE;
	}
    }
    unzClose(gz);
    
    neogeo_fix_bank_type = dr->sfix_bank;
    
    if (dr->rom_type == MGD2) {
        printf("convert MGD2 tiles\n");
	    convert_mgd2_tiles(memory.gfx, memory.gfx_size);
	    convert_mgd2_tiles(memory.gfx, memory.gfx_size);
    }
/* TODO */
#if 1
    if (dr->rom_type == MVS_CMC42) {
	    printf("Decrypt GFX\n");
	    kof99_neogeo_gfx_decrypt(dr->xor);
    }
    if (dr->rom_type == MVS_CMC50) {
	    printf("Decrypt GFX\n");
	    kof2000_neogeo_gfx_decrypt(dr->xor);
    }
#endif

   /* convert_all_char(memory.sfix_game, memory.sfix_size,
		     memory.fix_game_usage);
		     */


   	for (i = 0; i < memory.nb_of_tiles; i++) {
	    convert_tile(i);
	}

	dump_gfx(dr->name);
 
    return SDL_TRUE;
}

static int cmp_driver(void *a,void *b) {
    DRIVER *da=a;
    DRIVER *db=b;
    return strcmp(da->name,db->name);
}

void dr_list_all(void) {
    LIST *l;
    LIST *t=NULL;
    for(l=driver_list;l;l=l->next) {
	t=list_insert_sort_unique(t,l->data,cmp_driver);
    }
    for(l=t;l;l=l->next) {
	DRIVER *dr=l->data;
	printf("%-8s : %s\n",dr->name,dr->longname);
    }
}

LIST *driver_get_all(void) {
    return driver_list;
}
