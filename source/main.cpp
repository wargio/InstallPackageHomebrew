/*
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 This program was created by Grazioli Giovanni Dante <wargio@libero.it>.
*/
#include <NoRSX.h>

#include <sys/stat.h>
#include <sys/file.h>
#include <fstream>
#include <string.h>
#include <sysmodule/sysmodule.h>

#include "ICON_IMAGE.h"
#define BLOCK_SIZE	0x1000

#define HDD		1
#define USB		0

using namespace std;

static NoRSX *GFX;

static const char Hex[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
static unsigned int f1=0,f2=2;


u32 reboot_sys(){
	lv2syscall4(379,0x200,0,0,0);
	return_to_user_prog(u32);
}


int CopyFile(const char *source, const char *dest){
	s32 src = -1;
	s32 dst = -1;
	sysFSStat stats;

	char buffer[BLOCK_SIZE];
	u64  i;
	s32  ret;

	ret = sysLv2FsOpen(source, SYS_O_RDONLY, &src, 0, NULL, 0);
	if (ret)
		goto out;

	ret = sysLv2FsOpen(dest, SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC, &dst, 0, NULL, 0);
	if (ret)
		goto out;

	sysLv2FsChmod(dest, S_IFMT | 0777);

	sysLv2FsFStat(src, &stats);

	for (i = 0; i < stats.st_size;) {
		u64 pos, read, written;

		sysLv2FsLSeek64(src, i, 0, &pos);
		sysLv2FsLSeek64(dst, i, 0, &pos);

		ret = sysLv2FsRead(src, buffer, sizeof(buffer), &read);
		if (ret || !read)
			break;

		ret = sysLv2FsWrite(dst, buffer, read, &written);
		if (ret || !written)
			break;

		i += written;
	}

out:
	if (src >= 0) sysLv2FsClose(src);
	if (dst >= 0) sysLv2FsClose(dst);

	return ret;
}

static string Appid = "INSTALLPK";

void SetAppID(const char *path){
	char APPid[10];
	fstream file;
	file.open(path,ios::in|ios::binary);
	if(!file.fail()){
		file.seekg(55, ios::beg);
		file.read(APPid,9);
		file.close();
	}
	Appid = APPid;
}

int Get_size(const char *path){
	int length;
	fstream file;
	file.open(path,ios::in|ios::binary);
	if(!file.fail()){
		file.seekg(0, ios::end);
		length = file.tellg();
		file.close();
		return length;
	}
	return 0;
}

void Create_PDB(const char *path, const char *name, int size, const char *package){
	char appid[10];
	char contentid[37];
	fstream file;
	file.open(package,ios::in|ios::binary);
	if(!file.fail()){
		file.seekg(55, ios::beg);
		file.read(appid,9);
		file.seekg(0x30, ios::beg);
		file.read(contentid,37);
		file.close();
	}


	const char a[] = {0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x64,0x00, 0x00,0x00,0x04,0x00,0x00,0x00,0x04,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x65,0x00, 0x00,0x00,0x04,0x00,0x00,0x00,0x04,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x66,0x00, 0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,
		0x00,0x00,0x00,0x6b,0x00,0x00,0x00,0x04, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x03,
		0x00,0x00,0x00,0x68,0x00,0x00,0x00,0x04, 0x00,0x00,0x00,0x04,0x80,0x02,0x3e,0x13,
		0x00,0x00,0x00,0x6c,0x00,0x00,0x00,0x04, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0xd0,0x00,0x00,0x00,0x08, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00};
	const char a2[] = {0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x64,0x00, 0x00,0x00,0x04,0x00,0x00,0x00,0x04,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x65,0x00, 0x00,0x00,0x04,0x00,0x00,0x00,0x04,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x66,0x00, 0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,
		0x00,0x00,0x00,0x6b,0x00,0x00,0x00,0x04, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x03,
		0x00,0x00,0x00,0x68,0x00,0x00,0x00,0x04, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x6c,0x00,0x00,0x00,0x04, 0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0xd0,0x00,0x00,0x00,0x08, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00};
	const char b[] = {
		0x00,0x00,0x00,0xce,0x00,0x00,0x00,0x08, 0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00};
	const char c[] = {
		0x00,0x00,0x00,0xcc,0x00,0x00,0x00,0x1e, 
		0x00,0x00,0x00,0x1e,0x54,0x68,0x75,0x2c, 0x20,0x30,0x32,0x20,0x53,0x65,0x70,0x20,
		0x32,0x30,0x31,0x30,0x20,0x31,0x37,0x3a, 0x32,0x38,0x3a,0x31,0x30,0x20,0x47,0x4d,
		0x54,0x00,0x00,0x00,0x00,0x6a,0x00,0x00, 0x00,0x26,0x00,0x00,0x00,0x26,0x2f,0x64,
		0x65,0x76,0x5f,0x68,0x64,0x64,0x30,0x2f, 0x76,0x73,0x68,0x2f,0x74,0x61,0x73,0x6b,
		0x2f,0x30,0x30,0x30,0x30,0x30,0x30,Hex[f1], Hex[f2],0x2f,0x49,0x43,0x4f,0x4e,0x5f,0x46,
		0x49,0x4c,0x45,0x00,0x00,0x00,0x00,0x69, 0x00,0x00,0x00,0x21,0x00,0x00,0x00,0x21};
	const char d[] = {
		0x00,0x00,0x00,0x00,0xca,0x00,0x00,0x00, 0x5d,0x00,0x00,0x00,0x5d,0x68,0x74,0x74,
		0x70,0x3a,0x2f,0x2f,0x7a,0x65,0x75,0x73, 0x2e,0x64,0x6c,0x2e,0x70,0x6c,0x61,0x79,
		0x73,0x74,0x61,0x74,0x69,0x6f,0x6e,0x2e, 0x6e,0x65,0x74,0x2f,0x63,0x64,0x6e,0x2f,
		0x55,0x50,0x30,0x30,0x30,0x31,0x2f};
	const char e[] = {0x5f,0x30,0x30,0x2f};
	const char f[] = {
		0x2e,0x70,0x6b,
		0x67,0x3f,0x70,0x72,0x6f,0x64,0x75,0x63, 0x74,0x3d,0x30,0x30,0x38,0x34,0x26,0x63,
		0x6f,0x75,0x6e,0x74,0x72,0x79,0x3d,0x75, 0x73,0x00,0x00,0x00,0x00,0xcb,0x00,0x00,
		0x00,0x0e,0x00,0x00,0x00,0x0e};
	const char g[] = {
		0x2e,
		0x70,0x6b,0x67,0x00,0x00,0x00,0x00,0xd9, 0x00,0x00,0x00,0x25,0x00,0x00,0x00,0x25};
	const char h[] = {0x00,0x00,0x00, 0xda,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
		0x01,0x01,0x00,0x00,0x00,0xcd,0x00,0x00, 0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x00,
		0x00,0x00,0xeb,0x00,0x00,0x00,0x12,0x00, 0x00,0x00,0x12,0x68,0x74,0x74,0x70,0x3a,
		0x2f,0x2f,0x67,0x6f,0x6f,0x67,0x6c,0x65, 0x2e,0x63,0x6f,0x6d,0x00,0x00,0x00,0x00,
		0xec,0x00,0x00,0x00,0x01,0x00,0x00,0x00, 0x01,0x00};

	char fill_space[] = {
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20, 0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20, 0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};

	char path_file[80];
	sprintf(path_file,"%s/ICON_FILE",path);
	file.open(path_file,ios::out|ios::binary);
	if(!file.fail()){
		file.seekg(0, ios::beg);
		file.write(icon_image,sizeof(icon_image));
		file.close();
	}
	sysLv2FsChmod(path_file, S_IFMT | 0777);

	sprintf(path_file,"%s/f0.pdb",path);
	file.open(path_file,ios::out|ios::binary);
	if(!file.fail())
		file.close();
	sysLv2FsChmod(path_file, S_IFMT | 0777);



	sprintf(path_file,"%s/d0.pdb",path);
	file.open(path_file,ios::out|ios::binary);
	if(!file.fail()){
		file.seekg(0, ios::beg);
		file.write(a,sizeof(a));
		file.write(reinterpret_cast<char*>(&size),4); //File size
		file.write(b,sizeof(b));
		file.write(reinterpret_cast<char*>(&size),4); //File size
		file.write(c,sizeof(c));
		if(strlen(name)<32){
			file.write(name,strlen(name)); //APP name
			file.write(fill_space,(32-strlen(name))); //fill with ' '
		}else
			file.write(name,32);
		file.write(d,sizeof(d));
		file.write(appid,9); //APPID
		file.write(e,sizeof(e));
		file.write(appid,9); //APPID
		file.write(f,sizeof(f));
		file.write(appid,9); //APPID
		file.write(g,sizeof(g));
		file.write(contentid,37); //contentid
		file.write(h,sizeof(h));

		file.close();
	}
	sysLv2FsChmod(path_file, S_IFMT | 0777);
	
	sprintf(path_file,"%s/d1.pdb",path);
	file.open(path_file,ios::out|ios::binary);
	if(!file.fail()){
		file.seekg(0, ios::beg);
		file.write(a2,sizeof(a2));
		file.write(reinterpret_cast<char*>(&size),4); //File size
		file.write(b,sizeof(b));
		file.write(reinterpret_cast<char*>(&size),4); //File size
		file.write(c,sizeof(c));
		if(strlen(name)<32){
			file.write(name,strlen(name)); //APP name
			file.write(fill_space,(32-strlen(name))); //fill with ' '
		}else
			file.write(name,32);
		file.write(d,sizeof(d));
		file.write(appid,9); //APPID
		file.write(e,sizeof(e));
		file.write(appid,9); //APPID
		file.write(f,sizeof(f));
		file.write(appid,9); //APPID
		file.write(g,sizeof(g));
		file.write(contentid,37); //contentid
		file.write(h,sizeof(h));
//0x25
		file.close();
	}
	sysLv2FsChmod(path_file, S_IFMT | 0777);

}

s32 main(s32 argc, const char* argv[]){
	GFX = new NoRSX();
	Font F(JPN,GFX);

	F.Printf(100,80 ,COLOR_YELLOW,15,"Written By Deroad (PSXBrew.net)"); GFX->Flip();
	F.Printf(100,80 ,COLOR_YELLOW,15,"Written By Deroad (PSXBrew.net)"); GFX->Flip();

	F.Printf(100,100,COLOR_WHITE,15,"Looking for USB.."); GFX->Flip();
	F.Printf(100,100,COLOR_WHITE,15,"Looking for USB.."); GFX->Flip();
	int Y = 100;

	int set_dir = USB;

	char files[256], file[90];
	sysFSDirent entry;
	string package_names[100];
	int check=-1;
	int fd, num_pkg=0;
	u64 read;
	int usb_num=0;
	char path [256];
	for(usb_num=0;usb_num<11;usb_num++){
		sprintf(path,"/dev_usb00%d/",usb_num);
		if(sysLv2FsOpenDir(path,&fd) == 0){
			set_dir = USB;
			goto read_dir;
		}
	}

	Y+=50;
	F.Printf(100,Y,COLOR_WHITE,15,"No USB Found.. Looking for /dev_hdd0/packages"); GFX->Flip();
	F.Printf(100,Y,COLOR_WHITE,15,"No USB Found.. Looking for /dev_hdd0/packages"); GFX->Flip();
	sprintf(path,"/dev_hdd0/packages");
	if(sysLv2FsOpenDir(path,&fd) == 0){
		set_dir = HDD;
		goto read_dir;
	}else
		goto end;
read_dir:
	Y+=50;
	F.Printf(100,Y,COLOR_WHITE,15,"Dir: %s",path); GFX->Flip();
	F.Printf(100,Y,COLOR_WHITE,15,"Dir: %s",path); GFX->Flip();
	Y+=20;
	num_pkg=0;

	if(set_dir != HDD)
		sprintf(path,"/dev_usb00%d/",usb_num);
	else
		sprintf(path,"/dev_hdd0/packages");


	for(int i=0;i<100;i++){
		if(sysLv2FsReadDir(fd,&entry,&read)!=0)
			break;
		if (!strcmp(entry.d_name, ".") || !strcmp(entry.d_name, "..") || entry.d_type != 2)
			continue;
		if(entry.d_name[strlen(entry.d_name)-3] == 'p' &&
		   entry.d_name[strlen(entry.d_name)-2] == 'k' &&
		   entry.d_name[strlen(entry.d_name)-1] == 'g')
		{
			package_names[num_pkg] = entry.d_name;
			num_pkg++;
			check++;
		}
		GFX->Flip();
	}
	if(check<0)
		goto end;
	sysLv2FsCloseDir(fd);

	Y+=20;
	F.Printf(100,Y,COLOR_WHITE,15,"Installing PKG.."); GFX->Flip();
	F.Printf(100,Y,COLOR_WHITE,15,"Installing PKG.."); GFX->Flip();
	Y+=20;
	if(set_dir != HDD){
		F.Printf(100,Y,COLOR_RED,15,"DO NOT PLUG OFF THE USB.."); GFX->Flip();
		F.Printf(100,Y,COLOR_RED,15,"DO NOT PLUG OFF THE USB.."); GFX->Flip();
	}else{
		F.Printf(100,Y,COLOR_RED,15,"DO NOT TURN OFF THE PS3.."); GFX->Flip();
		F.Printf(100,Y,COLOR_RED,15,"DO NOT TURN OFF THE PS3.."); GFX->Flip();
	}

	for(int i=0;i<num_pkg;i++){
		if(set_dir != HDD)
			sprintf(path,"/dev_usb00%d/%s",usb_num,package_names[i].c_str());
		else
			sprintf(path,"/dev_hdd0/packages/%s",package_names[i].c_str());

		Y+=20;
		SetAppID(path);
		for(;f1<sizeof(Hex);f1++)
			for(;f2<sizeof(Hex);f2++){
				sprintf(file,"/dev_hdd0/vsh/task/000000%c%c",Hex[f1],Hex[f2]);
				if(sysLv2FsMkdir(file, S_IRWXO | S_IRWXU | S_IRWXG | S_IFDIR) == 0)
					goto copy;
			}
	copy:	
		sprintf(files,"%s/%s.pkg",file,Appid.c_str());
		if(CopyFile(path,files))
			goto end;
		Create_PDB(file,package_names[i].c_str(),Get_size(files),Appid.c_str());
	}

end:
	Y+=30;
	if(usb_num>9 && set_dir!=HDD){
		F.Printf(100,Y,COLOR_WHITE,15,"No USB/HDD package folder found"); GFX->Flip();
		F.Printf(100,Y,COLOR_WHITE,15,"No USB/HDD package folder found"); GFX->Flip();
		sleep(10);
	}else if(check<0){
		F.Printf(100,Y,COLOR_WHITE,15,"No package found"); GFX->Flip();
		F.Printf(100,Y,COLOR_WHITE,15,"No package found"); GFX->Flip();
		sleep(10);	
	}
	else{
		F.Printf(100,Y,COLOR_WHITE,15,"Done! Now your PS3 will Reboot.."); GFX->Flip();
		F.Printf(100,Y,COLOR_WHITE,15,"Done! Now your PS3 will Reboot.."); GFX->Flip();
		sleep(10);
		reboot_sys();
	}
	return 0;
}

